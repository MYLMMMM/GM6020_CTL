#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <vector>
#include <cstdint>

/**
 * @brief A simple C++ wrapper for Linux SocketCAN interface
 *
 * Example usage:
 * @code
 *   CanSocket can("can0");
 *   can.sendFrame(0x123, {0x11, 0x22, 0x33});
 *
 *   struct can_frame frame;
 *   if (can.recvFrame(frame, 1000)) {
 *       printf("Got frame: ID=0x%X DLC=%d\n", frame.can_id, frame.can_dlc);
 *   }
 * @endcode
 */
class CanSocket {
public:
    /**
     * @brief Construct and open a CAN socket on given interface
     * @param ifname e.g., "can0"
     */
    explicit CanSocket(const std::string &ifname)
        : ifname_(ifname), sock_(-1)
    {
        open();
    }

    /**
     * @brief Destructor: close socket automatically
     */
    ~CanSocket() {
        if (sock_ >= 0) {
            ::close(sock_);
        }
    }

    /**
     * @brief Send a CAN frame
     * @param can_id  CAN ID (11-bit or 29-bit)
     * @param data    Payload (0~8 bytes)
     */
    void sendFrame(uint32_t can_id, const std::vector<uint8_t> &data) {
        if (data.size() > 8) {
            throw std::runtime_error("CAN data > 8 bytes");
        }

        struct can_frame frame{};
        frame.can_id = can_id;
        frame.can_dlc = data.size();
        std::memcpy(frame.data, data.data(), data.size());

        ssize_t nbytes = ::write(sock_, &frame, sizeof(frame));
        if (nbytes != sizeof(frame)) {
            throw std::runtime_error("Failed to send CAN frame");
        }
    }

    /**
     * @brief Receive a CAN frame (blocking or with timeout)
     * @param frame_out  Output frame
     * @param timeout_ms Timeout in milliseconds (-1 = blocking forever)
     * @return true if frame received; false if timeout
     */
    bool recvFrame(struct can_frame &frame_out, int timeout_ms = -1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_, &readfds);

        struct timeval tv;
        if (timeout_ms >= 0) {
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
        }

        int ret = ::select(sock_ + 1, &readfds, nullptr, nullptr,
                           (timeout_ms >= 0 ? &tv : nullptr));

        if (ret < 0) {
            throw std::runtime_error("select() failed");
        }
        if (ret == 0) {
            return false;  // timeout
        }

        ssize_t nbytes = ::read(sock_, &frame_out, sizeof(frame_out));
        if (nbytes < 0) {
            throw std::runtime_error("read() failed");
        }

        return true;
    }

private:
    /**
     * @brief Internal function: open and bind CAN socket
     */
    void open() {
        sock_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock_ < 0) {
            throw std::runtime_error("socket(PF_CAN) failed");
        }

        struct ifreq ifr{};
        std::strncpy(ifr.ifr_name, ifname_.c_str(), IFNAMSIZ - 1);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';  // 保证以'\0'结尾

        if (::ioctl(sock_, SIOCGIFINDEX, &ifr) < 0) {
            throw std::runtime_error("ioctl(SIOCGIFINDEX) failed: " + ifname_);
        }

        struct sockaddr_can addr{};
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (::bind(sock_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            throw std::runtime_error("bind() failed");
        }

        std::cout << "[CAN] Connected to interface: " << ifname_ << std::endl;
    }

private:
    std::string ifname_;  ///< Interface name, e.g. "can0"
    int sock_;            ///< Socket file descriptor
};
