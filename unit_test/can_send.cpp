#include "lubancat_can.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>

void printHelp() {
    std::cout << "\n可用命令：" << std::endl;
    std::cout << "  send <can_id> <data_bytes...>   发送帧 (例如: send 123 11 22 33 44)" << std::endl;
    std::cout << "  recv [timeout_ms]               接收一帧 (默认1000ms)" << std::endl;
    std::cout << "  loop                            持续监听接收" << std::endl;
    std::cout << "  help                            查看命令帮助" << std::endl;
    std::cout << "  exit / quit                     退出程序" << std::endl;
}

int main(int argc, char** argv) {
    std::string ifname = "vcan0"; // 默认接口
    if (argc > 1) {
        ifname = argv[1]; // 支持从命令行指定接口，如 ./can_cli can0
    }

    try {
        CanSocket can(ifname);
        std::cout << "\n已连接到接口: " << ifname << std::endl;
        printHelp();

        std::string line;
        while (true) {
            std::cout << "\n> ";
            std::getline(std::cin, line);
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "send") {
                uint32_t can_id;
                iss >> std::hex >> can_id;  // 允许十六进制输入
                std::vector<uint8_t> data;
                std::string byte_str;

                while (iss >> byte_str) {
                    int value = std::stoi(byte_str, nullptr, 16);
                    data.push_back(static_cast<uint8_t>(value));
                }

                can.sendFrame(can_id, data);
                std::cout << "[TX] 发送 CAN 帧: ID=0x" << std::hex << can_id << " DLC=" << std::dec << data.size() << " Data=";
                for (auto b : data) printf(" %02X", b);
                std::cout << std::endl;
            }

            else if (cmd == "recv") {
                int timeout_ms = 1000;
                if (iss >> timeout_ms) {}
                struct can_frame frame;
                if (can.recvFrame(frame, timeout_ms)) {
                    std::cout << "[RX] 接收到 CAN 帧: ID=0x" << std::hex << frame.can_id << std::dec
                              << " DLC=" << int(frame.can_dlc) << " Data:";
                    for (int i = 0; i < frame.can_dlc; ++i)
                        printf(" %02X", frame.data[i]);
                    std::cout << std::endl;
                } else {
                    std::cout << "[RX] 超时（无新帧）" << std::endl;
                }
            }

            else if (cmd == "loop") {
                std::cout << "进入持续监听模式（按 Ctrl+C 退出）..." << std::endl;
                struct can_frame frame;
                while (true) {
                    if (can.recvFrame(frame, 1000)) {
                        std::cout << "[RX] ID=0x" << std::hex << frame.can_id << std::dec
                                  << " DLC=" << int(frame.can_dlc) << " Data:";
                        for (int i = 0; i < frame.can_dlc; ++i)
                            printf(" %02X", frame.data[i]);
                        std::cout << std::endl;
                    }
                }
            }

            else if (cmd == "help") {
                printHelp();
            }

            else if (cmd == "exit" || cmd == "quit") {
                std::cout << "退出程序。" << std::endl;
                break;
            }

            else {
                std::cout << "未知命令：" << cmd << "（输入 help 查看帮助）" << std::endl;
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "❌ 异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
