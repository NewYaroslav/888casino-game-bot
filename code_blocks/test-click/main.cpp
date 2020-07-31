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
    int x = 48;
    int y = 410;
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    for(int i = 0; i < 10; ++i) {
        mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x * 65535 / w, y * 65535 / h, 0, 0);
       // mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0); //*нажимаем* левую клавишу мыши
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0); //*Отпускаем* левую клавишу мыши
        //mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::system("pause");
    return 0;
}


