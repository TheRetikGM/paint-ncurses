#pragma once
#include <ncurses.h>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <clocale>
#include <cmath>
#include <algorithm>
#include <thread>
#include "Vec2_generic.hpp"
#include "tsqueue.hpp"
#define BLOCK_BOT L"▄"

#define NEW_2D_ARRAY(arr, type, x, y)\
    arr = new type*[x];\
    for (auto i = 0; i < x; i++)\
        arr[i] = new type[y];
#define DELETE_2D_ARRAY(x, arr)\
    for (auto i = 0; i < x; i++)\
        delete[] arr[i];\
    delete[] arr;

using std::string;
using std::vector;
using namespace std::literals::chrono_literals;

namespace cge
{    
    typedef std::chrono::high_resolution_clock Time;
    typedef cge::Vec2_generic<float> Vec2f;

    enum class Align : uint8_t { Left, Center, Right };
    struct Fragment
    {
        uint8_t f;
        uint8_t b;
        //wchar_t ch[2];
        bool state;

        uint8_t GetColor() { return state ? f : b; }
    };
    struct cge_string
    {
        std::wstring str;
        int x, y;
        bool alpha;
        uint8_t f;
    };
    class CursesGameEngine
    {
    private:
        bool y_offset_odd = false;        
        bool *pair_defined;
        int y_last_odd = false;
        int x_offset = 0, y_offset = 0;
        int win_width = 0;
        int win_height = 0;
        float timeFromStart = 0.0f;
        float msPerFrame = 1000.0f / 60.0f;
        std::vector<int> inputBuffer;
        tsqueue<cge_string> strQueue;
        MEVENT mouseEvent;        

    protected:
        WINDOW* win = NULL;        
        // Back buffer width and height are same as win width and height.
        Fragment** back_buffer = NULL;        
        uint8_t outColor = COLOR_BLACK;        

    public:
        std::vector<std::string> Errors;
        CursesGameEngine() {        
            setlocale(LC_CTYPE, "en_US.UTF-8");
            initscr();
            if (has_colors())
                start_color();
            cbreak();
            mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);    
            mouseinterval(0);
            curs_set(false);            

            pair_defined = new bool[COLOR_PAIRS];
            for (auto i = 0; i < COLOR_PAIRS; i++)
                pair_defined[i] = false;
        }
        virtual ~CursesGameEngine()
        {            
            flushinp();
            // Delete allocated memory
            if (win)
                delwin(win);
            endwin();
            //back_buffer[1000][1000] = Fragment{1, 2, 1};

            //DELETE_2D_ARRAY(win_width, back_buffer);
            for (auto i = 0; i < win_width; i++)
                delete[] back_buffer[i];
            delete[] back_buffer;
            delete[] pair_defined;
        }        
        bool Construct(int width, int height, int x, int y, bool sameSides)
        {
            if (width > getmaxx(stdscr) || height > getmaxy(stdscr)*2) {
                Errors.push_back("[ERROR] cge::CursesGameEngine::Construct: Specified width or height is higher than stdscr dimensions! Not constructed.");                
                return false;
            }

            win_width = (width >= 0) ? width : getmaxx(stdscr);
            win_height = (height >= 0) ? height : getmaxy(stdscr) * 2;  // two rows in one character
            if (sameSides) {
                if (win_width > win_height)
                    win_width = win_height;
                else
                    win_height = win_width;
            }

            x = (x >= 0) ? x : (getmaxx(stdscr) - win_width) / 2;
            y = (y >= 0) ? y : (getmaxy(stdscr) - (win_height / 2)) / 2;
            y_offset_odd = y & 1;
            y_last_odd = (y + win_height) & 1;
            win = newwin(win_height / 2 + (y_offset_odd | y_last_odd), win_width, y / 2, x);

            if (win != NULL) 
            {                
                nodelay(win, true);
                keypad(win, true);                                
                NEW_2D_ARRAY(back_buffer, Fragment, win_width, win_height);
                Clear(COLOR_BLACK);                
                x_offset = x;
                y_offset = y;
                return true;
            }
            else            
                return win_width = win_height = y_offset_odd = y_last_odd = 0; // RETURN false            
        }        
        // Sets frame cap. Default is 60 FPS
        void SetMaxFPS(int framesPerSec)
        {
            msPerFrame = 1000.0f / framesPerSec;
        }
        void Start()
        {
            if (!OnGameStart())
                return;

            bool run = true;
            std::chrono::_V2::system_clock::time_point before = Time::now(), after;
            std::chrono::duration<float, std::milli> fsec;
            float delta = 0;
            while (run)
            {
                handle_input();
                run = OnGameUpdate(delta);                
                draw_back_buffer();
                draw_strings();                
                wrefresh(win);

                after = Time::now();
                fsec = after - before;

                if (fsec.count() < msPerFrame)
                {
                    std::chrono::duration<float, std::milli> delta_ms(msPerFrame - fsec.count());
                    auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
                    std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
                }
                
                before = Time::now();
                std::chrono::duration<float, std::milli> sleep_time = before - after;                           
                delta = (fsec + sleep_time).count() / 1000.0f;

                timeFromStart += delta;
            }
        }
        int StdWinWidth() { return getmaxx(stdscr); }
        int StdWinHeight() { return getmaxy(stdscr) * 2; }        
 
    protected:
        virtual bool OnGameStart() { return false; }
        virtual bool OnGameUpdate(float delta) { return false; }
        virtual void OnMouseEvent(int x, int y,  mmask_t bstate) {}
        virtual void OnKeyPressed(int key) {}

        /*
            Draw functions 
        */
        // Draw a single pixel
        virtual void Draw(int x, int y, uint8_t fColor) 
        {
            if (!inRange(0, win_width - 1, x) || !inRange(0, win_height - 1, y))
                return;

            back_buffer[x][y].state = true;
            back_buffer[x][y].f = fColor;
        }
        void DrawString(int x, int y, std::wstring str, uint8_t fColor, bool alpha = false, Align alignment = Align::Left)
        {
            cge_string s;
            s.str = str;
            s.alpha = alpha;
            s.f = fColor;

            switch(alignment) {
                case Align::Left: break;
                case Align::Center: x -= str.length() / 2; break;
                case Align::Right:  x -= str.length(); break;
            }            
            s.x = std::clamp(x, 0, win_width);
            s.y = y / 2;

            strQueue.push(s);        
        }
        void DrawString(int x, int y, std::string str, uint8_t fColor, bool alpha = false, Align alignment = Align::Left) {
            DrawString(x, y, std::wstring(str.begin(), str.end()), fColor, alpha, alignment);
        }
        void DrawLine(int x0, int y0, int x1, int y1, uint8_t color)
        {
            /* Use of Bresenham's line algorithm. */
            int dx = std::abs(x1 - x0);
            int sx = x0 < x1 ? 1 : -1;
            int dy = -abs(y1 - y0);
            int sy = y0 < y1 ? 1 : -1;
            int err = dx + dy;

            while (true)
            {
                Draw(x0, y0, color);
                if (x0 == x1 && y0 == y1) break;
                int e2 = 2 * err;
                if (e2 >= dy) {
                    err += dy;
                    x0 += sx;
                }
                if (e2 <= dx) {
                    err += dx;
                    y0 += sy;
                }
            }

        }
        void DrawCircle(int center_x, int center_y, int r, uint8_t color) 
        {
            /* Use of Bresenham's circle algorithm. */
            int x = 0, y = r;
            int d = 3 - 2 * r;
            drawCircle_8p(center_x, center_y, x, y, color);
            while (y >= x)
            {
                x++;
                if (d > 0) {
                    y--;
                    d = d + 4 * (x - y) + 10;
                }
                else
                    d = d + 4 * x + 6;
                drawCircle_8p(center_x, center_y, x, y, color);                
            }
        }
        // Two corners
        void DrawRectangle(int x0, int y0, int x1, int y1, uint8_t color) {
            int x, y;
            int xi = (x1 - x0) > 0 ? 1 : -1;
            int yi = (y1 - y0) > 0 ? 1 : -1;
            
            for (x = x0; x != x1 + xi; x += xi) {
                Draw(x, y0, color);
                Draw(x, y1, color);
            }
            for (y = y0; y != y1 + yi; y += yi) {
                Draw(x0, y, color);
                Draw(x1, y, color);
            }
        }
        void FillRectangle(int x0, int y0, int x1, int y1, uint8_t bColor) {}
        void FillPie(int x, int y, float angle, uint8_t bColor) {}
        void FillCircle(int center_x, int center_y, int r, uint8_t color)
        {
            /* Use of Bresenham's circle algorithm. */
            int x = 0, y = r;
            int d = 3 - 2 * r;
            fillCircle_8p(center_x, center_y, x, y, color);
            while (y >= x)
            {
                x++;
                if (d > 0) {
                    y--;
                    d = d + 4 * (x - y) + 10;
                }
                else
                    d = d + 4 * x + 6;
                fillCircle_8p(center_x, center_y, x, y, color);                
            }
        }
        /*
            State setting functions
        */
        void SetCursorState(bool on)
        {
            curs_set(on);
        }
        void Clear(uint8_t color)
        {
            for (int x = 0; x < win_width; x++)
                for (int y = 0; y < win_height; y++)
                {
                    back_buffer[x][y].b = back_buffer[x][y].f = color;
                    back_buffer[x][y].state = false;
                }
        }
        void ClearStd(uint8_t color)
        {
            cchar_t c;
            setcchar(&c, L" ", WA_NORMAL, get_pair(COLOR_BLACK, color), NULL);
            bkgrnd(&c);
            outColor = color;
            refresh();
        }
        /*
            State getting functions
        */
        int WinWidth() { return win_width; }        
        int WinHeight() {return win_height; }
        /* Returs time elapsed after calling Start() in seconds. */
        float GetTime() { return timeFromStart; }
        /* Gets one character from stdin. */
        int GetChar() { return wgetch(win); } 
    private:
        uint16_t get_pair(const uint8_t& fore, const uint8_t& back)
        {
            static uint16_t pair;
            pair = fore * 256 + back + 1;

            // Define pair only if it isn't already defined.
            if (!pair_defined[pair])
            {
                init_extended_pair(pair, fore, back);
                pair_defined[pair] = true;
            }
            return pair;                        
        }
        void draw_back_buffer()
        {
            static uint8_t F, B;
            for (int x = 0; x < win_width; x++)
            {
                for (int y = (y_offset_odd) ? 2 : 1; y < win_height; y += 2)
                {
                    F = back_buffer[x][y].GetColor();
                    B = back_buffer[x][y - 1].GetColor();
                    wattr_set(win, WA_NORMAL, get_pair(F, B), NULL);
                    mvwaddwstr(win, y / 2, x, BLOCK_BOT);
                }
            }
            if (y_offset_odd)
            {
                for (int x = 0; x < win_width; x++)
                {
                    F = back_buffer[x][0].GetColor();
                    B = outColor;
                    wattr_set(win, WA_NORMAL, get_pair(F, B), NULL);
                    mvwaddwstr(win, 0, x, BLOCK_BOT);
                }
            }
            if (y_last_odd)
            {
                for (int x = 0; x < win_width; x++)
                {
                    F = outColor;
                    B = back_buffer[x][win_height - 1].GetColor();
                    wattr_set(win, WA_NORMAL, get_pair(F, B), NULL);
                    mvwaddwstr(win, (win_height) / 2, x, BLOCK_BOT);
                }
            }
        }
        void drawCircle_8p(const int& center_x, const int& center_y, const int& x, const int& y, const uint8_t& color)
        {
            Draw(center_x + x, center_y + y, color);
            Draw(center_x - x, center_y + y, color);
            Draw(center_x + x, center_y - y, color);
            Draw(center_x - x, center_y - y, color);
            Draw(center_x + y, center_y + x, color);
            Draw(center_x - y, center_y + x, color);
            Draw(center_x + y, center_y - x, color);
            Draw(center_x - y, center_y - x, color);
        }      
        void drawSVLine(const int& x, int y0, const int& y1, const uint8_t& color)
        {
            for (; y0 >= y1; y0--)
                Draw(x, y0, color);
        }          
        void fillCircle_8p(const int& center_x, const int& center_y, const int& x, const int& y, const uint8_t& color)
        {
            drawCircle_8p(center_x, center_y, x, y, color);
            drawSVLine(center_x + x, center_y + y, center_y - y, color);
            drawSVLine(center_x - x, center_y + y, center_y - y, color);
            drawSVLine(center_x + y, center_y + x, center_y - x, color);
            drawSVLine(center_x - y, center_y + x, center_y - x, color);
        }  
        bool inRange(const int& low, const int& high, const int& x)
        {            
            return (low <= x && x <= high);
        }
        void draw_strings()
        {
            auto i = strQueue.pop();
            cge_string str;
            uint8_t b;
            wchar_t ch[2];            
            while (i)
            {
                str = *i;

                for (int j = 0; j < str.str.length(); j++)
                {
                    if (str.x + j >= win_width)
                        break;
                    if (str.alpha && str.str[j] == L' ')
                        continue;
                    
                    b = back_buffer[str.x + j][str.y * 2].GetColor();

                    wattr_set(win, WA_NORMAL, get_pair(str.f, b), NULL);
                    ch[0] = str.str[j]; ch[1] = L'\0';
                    mvwaddwstr(win, str.y, str.x + j, ch);
                }
                i = strQueue.pop();
            }
        }
        void handle_input()
        {
            int input;
            while ((input = wgetch(win)) != ERR)
            {
                if (input == KEY_MOUSE)
                {
                    if (getmouse(&mouseEvent) == OK) {
                        int x = std::clamp(mouseEvent.x - x_offset, 0, win_width - 1);
                        int y = std::clamp(mouseEvent.y * 2 - y_offset, 0, win_height - 1);
                        OnMouseEvent(x, y, mouseEvent.bstate);
                    }
                }
                else {
                    OnKeyPressed(input);
                }
            }
        }
    };
}