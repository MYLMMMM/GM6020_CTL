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
        memset(&send_buffer,0,sizeof(send_buffer));
        memset(recv_buffer,0,2*sizeof(recv_buffer[0]));
    }

    /**
     * @brief Destructor: close socket automatically
     */
    ~CanSocket() {
        if (sock_ >= 0) {
            ::close(sock_);
        }
    }

    void sendFrame()
    {
        
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

public:
    struct can_frame send_buffer,recv_buffer[2];
private:
    std::string ifname_;  ///< Interface name, e.g. "can0"
    int sock_;            ///< Socket file descriptor
};

