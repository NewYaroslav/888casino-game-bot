#include <iostream>
#include <server_ws.hpp>
#include <nlohmann/json.hpp>
#include <xtime.hpp>
#include <future>
#include <thread>
#include <atomic>
#include <chrono>
#include <limits>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <fstream>
#include <windows.h>
#include <conio.h>

#include "../../include/casio-strategy.hpp"

using json = nlohmann::json;
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

int main() {
    std::cout << "start!" << std::endl;
    casino::Strategy strategy;

    while(true) {
        bool is_exit = false;
        if(kbhit()) {
            int c = getch();
            switch(c) {
                case '0':
                    {
                        std::cout << "Zero" << std::endl;
                        casino::Strategy::Bet bet;
                        strategy.update(casino::Strategy::TypesSignal::ZERO, bet);
                        if(bet.step == 0) std::cout << "skip" << std::endl;
                    }
                    break;
                case 'r':
                case 'R':
                case '1':
                    {
                        std::cout << "Red" << std::endl;
                        casino::Strategy::Bet bet;
                        strategy.update(casino::Strategy::TypesSignal::RED, bet);
                        if(bet.step == 0) std::cout << "skip" << std::endl;
                        else std::cout << "step " << bet.step  << " color " << bet.get_color() << std::endl;
                    }
                    break;
                case 'b':
                case 'B':
                case '2':
                    {
                        std::cout << "Black" << std::endl;
                        casino::Strategy::Bet bet;
                        strategy.update(casino::Strategy::TypesSignal::BLACK, bet);
                        if(bet.step == 0) std::cout << "skip" << std::endl;
                        else std::cout << "step " << bet.step  << " color " << bet.get_color() << std::endl;
                    }
                    break;
                case VK_SPACE:
                    is_exit = true;
                    std::cout << "exit" << std::endl;
                    break;
                default:
                    std::cout << c << std::endl;
                    break;
            }
        } // if
        if(is_exit) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::system("pause");
    return 0;
}


