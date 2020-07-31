#include "ico.h"
#include "../../include/casio-ws-server.hpp"
#include "../../include/casio-strategy.hpp"
#include "../../lib/xtechnical_analysis/include/xtechnical_circular_buffer.hpp"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <mutex>
#include <atomic>
#include <future>

#include "nlohmann/json.hpp"


#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"
//#include "IconsFontaudio.h"
//#include "IconsForkAwesome.h"
//#include "inlcude/va-editor-256x256.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#define NDEBUG
#define PROGRAM_VERSION "1.0"
#define PROGRAM_DATE "20.07.2020"

static void HelpMarker(const char* desc);

/** \brief Класс параметров программы
 */
class ParamSpec {
private:
    std::string ini_file;
    xtechnical::circular_buffer<std::string> text_buffer;
    std::mutex text_buffer_mutex;
public:
    enum StringId {
        STR_MAIN_MENU_HELP_RED = 0,
        STR_MAIN_MENU_HELP_BLACK = 1,
        STR_MAIN_MENU_START = 2,
        STR_MAIN_MENU_STOP = 3,
        STR_MAIN_MENU_HELP_START = 4,
    };

    const char text_data[5][512] = {
        u8"Задайте координаты кнопки для ставок на 'красное'. "
        "Для этого нажмите на кнопку 'Set Red', "
        "переместите курсок мыши на кнопку ставки на 'красное', "
        "и нажмите букву 'S', после чего программа запомнит координаты.",
        u8"Задайте координаты кнопки для ставок на 'черное'"
        "Для этого нажмите на кнопку 'Set Black', "
        "переместите курсок мыши на кнопку ставки на 'черное', "
        "и нажмите букву 'S', после чего программа запомнит координаты.",
        u8"START##Start",
        u8"STOP##Start",

        u8"Нажмите на кнопку 'START' для запуска робота. Повторное нажатие на кнопку 'STOP' остановит робота. "
        "Если кнопка неактивная, значит отсутствует подключение к 888casino.",
    };

    const uint32_t window_width = 640;
    const uint32_t window_height = 480;
    const uint32_t window_indent = 32;

    int red_x = 0;
    int red_y = 0;
    int black_x = 0;
    int black_y = 0;
    int click_delay = 500;              /**< Задержка между кликами */
    int step = 0;                       /**< Шаг мартингейла */
    bool is_set_red_pos = false;        /**< Флаг установки координат для красного */
    bool is_set_black_pos = false;      /**< Флаг установки координат для черного */
    std::atomic<bool> is_start;         /**< Флаг работы робота */
    std::atomic<bool> is_connect;       /**< Флаг соединения робота */

    std::string note;

    ParamSpec() : ini_file("settings.json"), text_buffer(100) {
        is_start = false;
        is_connect = false;
        using json = nlohmann::json;
        std::ifstream file_json(ini_file);
        if(file_json) {
            try {
                json j;
                file_json >> j;
                red_x = j["data"]["red"]["x"];
                red_y = j["data"]["red"]["y"];
                black_x = j["data"]["black"]["x"];
                black_y = j["data"]["black"]["y"];
                click_delay = j["data"]["click_delay"];
            }
            catch(...) {}
            file_json.close();
        }
    }

    /** \brief Сохранить настройки
     */
    void save() {
        using json = nlohmann::json;
        try {
            json j;
            j["data"]["red"]["x"] = red_x;
            j["data"]["red"]["y"] = red_y;
            j["data"]["black"]["x"] = black_x;
            j["data"]["black"]["y"] = black_y;
            j["data"]["click_delay"] = click_delay;
            std::ofstream file_json(ini_file);
            file_json << std::setw(4) << j << std::endl;
            file_json.close();
        }
        catch(...) {}
    }

    void add_message(const std::string &msg) {
        std::lock_guard<std::mutex> lock(text_buffer_mutex);
        std::cout << "msg " << msg << std::endl;
        text_buffer.push_back(msg);
    }

    std::string get_note() {
        std::string text;
        std::lock_guard<std::mutex> lock(text_buffer_mutex);
        for(int i = 99; i >= (100 - text_buffer.size()); --i) {
            text += text_buffer[i];
            text += "\n";
        }
        return text;
    }

    void click_red(const int step) {
        int x = red_x;
        int y = red_y;
        int d = click_delay;
        std::thread thread_click([&, step, x, y, d](){
            int w = GetSystemMetrics(SM_CXSCREEN);
            int h = GetSystemMetrics(SM_CYSCREEN);
            for(int i = 0; i < step; ++i) {
                mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (x * 65535 / w), (y * 65535 / h), 0, 0);
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, (x * 65535 / w), (y * 65535 / h), 0, 0); //*нажимаем* левую клавишу мыши
                mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, (x * 65535 / w), (y * 65535 / h), 0, 0); //*Отпускаем* левую клавишу мыши
                //mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(d));
            }
        });
        thread_click.detach();
    }

    void click_black(const int step) {
        int x = black_x;
        int y = black_y;
        int d = click_delay;
        std::thread thread_click([&, step, x, y, d](){
            int w = GetSystemMetrics(SM_CXSCREEN);
            int h = GetSystemMetrics(SM_CYSCREEN);
            for(int i = 0; i < step; ++i) {
                mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (x * 65535 / w), (y * 65535 / h), 0, 0);
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, (x * 65535 / w), (y * 65535 / h), 0, 0); //*нажимаем* левую клавишу мыши
                mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, (x * 65535 / w), (y * 65535 / h), 0, 0); //*Отпускаем* левую клавишу мыши
                //mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(d));
            }
        });
        thread_click.detach();
    }

    ~ParamSpec() {};
};

int main() {
    HWND hwnd = GetConsoleWindow();
    #ifdef NDEBUG
    ShowWindow (hwnd,SW_HIDE);
    #endif

    ParamSpec param;

    /* запускаем отдельный поток */
    std::atomic<bool> is_shutdown;
    is_shutdown = false;
    std::future<void> casino_future= std::async(std::launch::async,[&]() {
        casino::Server ws_server;
        casino::Strategy strategy;
        casino::Strategy::Bet bet;
        ws_server.on_event = [&](casino::Server::EventTypes event, casino::Server::WinBetTypes bet_type, const std::string &msg) {
            switch(event) {
            case casino::Server::EventTypes::WIN_SPOTS: {
                    if(!param.is_start) {
                        strategy.clear();
                    }
                    switch(bet_type) {
                    case casino::Server::WinBetTypes::RED: {
                            param.add_message("RED");
                            if(param.is_start && param.is_connect) strategy.update(casino::Strategy::TypesSignal::RED, bet);
                        }
                        break;
                    case casino::Server::WinBetTypes::BLACK: {
                            param.add_message("BLACK");
                            if(param.is_start && param.is_connect) strategy.update(casino::Strategy::TypesSignal::BLACK, bet);
                        }
                        break;
                    case casino::Server::WinBetTypes::ZERO: {
                            param.add_message("ZERO");
                            if(param.is_start && param.is_connect) strategy.update(casino::Strategy::TypesSignal::ZERO, bet);
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            case casino::Server::EventTypes::BETS_OPEN: {
                    param.add_message("BETS OPEN");
                    /* делаем ставку, если нужно */
                    if(param.is_start && param.is_connect) {
                        if(bet.step == 0) {
                            param.add_message("BOT SKIP BET");
                        } else {
                            param.add_message("BOT MAKE BET, " + bet.get_color() + ", STEP " + std::to_string(bet.step));
                            /* вызываем кликер */
                            if(bet.signal == casino::Strategy::TypesSignal::RED) {
                                param.click_red(bet.step);
                            } else
                            if(bet.signal == casino::Strategy::TypesSignal::BLACK) {
                                param.click_black(bet.step);
                            }
                        }
                    }
                }
                break;
            case casino::Server::EventTypes::BETS_CLOSED: {
                    param.add_message("BETS CLOSED");
                }
                break;
            case casino::Server::EventTypes::CONNECT: {
                    param.add_message("BRIDGE CONNECT");
                }
                break;
            case casino::Server::EventTypes::DISCONNECT: {
                    param.add_message("BRIDGE DISCONNECT");
                }
                break;
            case casino::Server::EventTypes::GAME_CONNECT: {
                    param.add_message("BRIDGE GAME CONNECT");
                    param.is_connect = true;
                }
                break;
            case casino::Server::EventTypes::GAME_DISCONNECT: {
                    param.add_message("BRIDGE GAME DISCONNECT");
                    param.is_connect = false;
                    /* запускаем bat файл */
                    std::system("open_url.bat");
                }
                break;
            default:
                break;
            };
        };

        ws_server.init();
        /* ждем выхода */
        while(!is_shutdown) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    const char window_name[] = "888 Casino Bot " PROGRAM_VERSION;
    sf::RenderWindow window(sf::VideoMode(param.window_width, param.window_height), window_name, sf::Style::None);
    window.setFramerateLimit(60);

    /* настраиваем стиль и шрифты */
    ImGui::SFML::Init(window, false);
    /* настраиваем стиль */
    ImGui::StyleColorsDark();

    /* настраиваем язык */
    ImFontConfig font_config;
    font_config.OversampleH = 1; //or 2 is the same
    font_config.OversampleV = 1;
    font_config.PixelSnapH = 1;

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x044F, // Cyrillic
        0,
    };
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 14.0f, NULL, ranges);

    ImGui::SFML::UpdateFontTexture();

    /* создаем шапку */
    sf::RectangleShape head_rectangle(sf::Vector2f(param.window_width, param.window_indent));
    head_rectangle.setFillColor(sf::Color(50, 50, 50));

    sf::Vector2i grabbed_offset;
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            } else
            // перетаскивание окна https://en.sfml-dev.org/forums/index.php?topic=14391.0
            if(event.type == sf::Event::MouseButtonPressed) {
                if(event.mouseButton.button == sf::Mouse::Left) {
                    grabbed_offset = window.getPosition() - sf::Mouse::getPosition();
                }
            }
            if(event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::S && (param.is_set_red_pos || param.is_set_black_pos)) {
                    param.is_set_red_pos = false;
                    param.is_set_black_pos = false;
                    param.save();
                }
            }
        }

        if(param.is_set_red_pos) {
            param.red_x = sf::Mouse::getPosition().x;
            param.red_y = sf::Mouse::getPosition().y;
        } else
        if(param.is_set_black_pos) {
            param.black_x = sf::Mouse::getPosition().x;
            param.black_y = sf::Mouse::getPosition().y;
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        const uint32_t main_menu_width = param.window_width;
        const uint32_t main_menu_heigh = param.window_height - head_rectangle.getSize().y;

        ImGui::SetNextWindowPos(ImVec2(param.window_width/2 - main_menu_width/2, param.window_height/2 - main_menu_heigh/2 + head_rectangle.getSize().y/2), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(main_menu_width, main_menu_heigh), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.5);
        ImGui::Begin("##MainMenu", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysVerticalScrollbar);

        /* создаем две колонки минимум */
        ImGui::Columns(2, NULL, false);

        if(ImGui::Button("Set Red")) {
            param.is_set_red_pos ^= true;
        }
        ImGui::SameLine();
        HelpMarker(param.text_data[ParamSpec::STR_MAIN_MENU_HELP_RED]);
        ImGui::Text("Red button coordinates");


        if(!param.is_set_red_pos) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        ImGui::InputInt("X##RED_X", &param.red_x, 1, 10);
        ImGui::InputInt("Y##RED_Y", &param.red_y, 1, 10);

        if(!param.is_set_red_pos) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        ImGui::NextColumn();
        if(ImGui::Button("Set Black")) {
            param.is_set_black_pos ^= true;
        }
        ImGui::SameLine();
        HelpMarker(param.text_data[ParamSpec::STR_MAIN_MENU_HELP_BLACK]);
        ImGui::Text("Black button coordinates");

        if(!param.is_set_black_pos) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        ImGui::InputInt("X##BLACK_X", &param.black_x, 1, 10);
        ImGui::InputInt("Y##BLACK_Y", &param.black_y, 1, 10);

        if(!param.is_set_black_pos) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        /* создаем две колонки минимум */
        ImGui::Columns(2, NULL, false);

        ImGui::InputInt("Click Delay##Click Delay", &param.click_delay, 1, 10);

        ImGui::NextColumn();

        /* запускаем робота */
        {
            ImVec2 start_button_size(128, 64);
            if(!param.is_connect && !param.is_start) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }
            if(param.is_start) {
                if(ImGui::Button(param.text_data[ParamSpec::STR_MAIN_MENU_STOP], start_button_size)) {
                    param.is_start = false;
                }
            } else {
                if(ImGui::Button(param.text_data[ParamSpec::STR_MAIN_MENU_START], start_button_size)) {
                    param.is_start = true;
                }
            }
            if(!param.is_connect && !param.is_start) {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }
            ImGui::SameLine();
            HelpMarker(param.text_data[ParamSpec::STR_MAIN_MENU_HELP_START]);
        }


        ImGui::Columns(1);
        ImGui::Separator();

        /* выводим всякую финормацию */
        {
            std::string note = param.get_note();
            ImVec2 text_size(500, 178);
            ImGui::InputTextMultiline("Note", (char*)note.c_str(), note.size(), text_size, ImGuiInputTextFlags_ReadOnly);
        }

        ImGui::Separator();

        /* выход из программы */
        ImVec2 exit_button_size(128, 64);
        if(ImGui::Button("Exit##ExitBot", exit_button_size)) {
            window.close();
        }
        ImGui::End();

        /* реализуем перемещение окна в экране */
        bool grabbed_window = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        if(grabbed_window &&
            (sf::Mouse::getPosition(window).y < (float)param.window_indent &&
             sf::Mouse::getPosition(window).y > 0.0 &&
             sf::Mouse::getPosition(window).x > 0.0 &&
             sf::Mouse::getPosition(window).x < (float)param.window_width)) {
                window.setPosition(sf::Mouse::getPosition() + grabbed_offset);
        }

        window.clear();
        window.draw(head_rectangle);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    param.save();

    /* ждем остановки потока */
    is_shutdown = true;
    if(casino_future.valid()) {
        try {
            casino_future.wait();
            casino_future.get();
        }
        catch(const std::exception &e) {
            std::cerr << "Error: casino_future, what: " << e.what() << std::endl;
        }
        catch(...) {
            std::cerr << "Error: casino_future" << std::endl;
        }
    }
    return 0;
}


static void HelpMarker(const char* desc) {
    //ImGui::TextDisabled(ICON_FK_QUESTION_CIRCLE_O);
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
