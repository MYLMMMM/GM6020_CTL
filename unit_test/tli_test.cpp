#include <ncurses.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>
#include <string>

class NcursesCLI {
private:
    std::atomic<bool> running_;
    
public:
    NcursesCLI() : running_(true) {}
    
    void run() {
        // 初始化ncurses
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        
        int input;
        int counter = 0;
        std::string input_history = "No input";
        
        while (running_) {
            clear();
            
            // 使用英文显示
            mvprintw(0, 0, "=== Real-time Monitor (Press 'q' to quit) ===");
            mvprintw(2, 0, "Counter: %d", counter++);
            mvprintw(3, 0, "Timestamp: %ld", time(nullptr));
            
            // 显示当前时间
            time_t now = time(nullptr);
            struct tm* local_time = localtime(&now);
            mvprintw(4, 0, "Local Time: %02d:%02d:%02d", 
                    local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
            
            // 输入区域
            mvprintw(7, 0, "Input Area (keys will show here):");
            mvprintw(8, 0, "Last Input: %s", input_history.c_str());
            mvprintw(10, 0, "Hint: Press arrow keys to test, 'q' to quit");
            
            // 检查输入
            input = getch();
            if (input != ERR) {
                if (input == 'q' || input == 'Q') {
                    running_ = false;
                    input_history = "Quit command";
                } else {
                    switch(input) {
                        case KEY_UP:
                            input_history = "UP arrow";
                            break;
                        case KEY_DOWN:
                            input_history = "DOWN arrow";
                            break;
                        case KEY_LEFT:
                            input_history = "LEFT arrow";
                            break;
                        case KEY_RIGHT:
                            input_history = "RIGHT arrow";
                            break;
                        case '\n':
                        case '\r':
                            input_history = "ENTER key";
                            break;
                        case 27:
                            input_history = "ESC key";
                            break;
                        case '\t':
                            input_history = "TAB key";
                            break;
                        case KEY_BACKSPACE:
                        case 127:
                            input_history = "BACKSPACE";
                            break;
                        default:
                            if (input >= 32 && input <= 126) {
                                input_history = std::string("Char: '") + static_cast<char>(input) + "'";
                            } else {
                                input_history = "Special key (code: " + std::to_string(input) + ")";
                            }
                    }
                }
            }
            
            refresh();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        endwin();
        printf("Program exited normally. Thank you!\n");
    }
};

int main() {
    NcursesCLI cli;
    cli.run();
    return 0;
}