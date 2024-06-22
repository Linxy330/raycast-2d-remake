#pragma once

#include<windows.h>
#include<iostream>
#include"color.h"

constexpr int dW = 9, dH = 9; // 字体大小

//窗口大小
int WIDTH = (GetSystemMetrics(SM_CXSCREEN) - dW) / dW + 1;
int HEIGHT = (GetSystemMetrics(SM_CYSCREEN) - dH) / dH;

struct CHAR_INFO_BUFFER {
    CHAR_INFO *buffer;
    COORD size;
    COORD coord;
    SMALL_RECT rect;
};

class Screen {
public:

    Screen() {
        Setup();
        Clear();
        initCharInfoBuffer();
    }

    //画布清除
    void Clear() {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (i >= HEIGHT / 2) canvas[i * WIDTH + j] = 15;
                else canvas[i * WIDTH + j] = 0;
            }
        }
    }

    //画点
    void DrawPoint(int x, int y, byte luminuns) { //(x,y)处亮度为luminuns的点
        if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) {
            return;
        } else {
            canvas[y * WIDTH + x] = luminuns;
        }
    }

    //画线 bresenham
    void DrawLine(int x0, int y0, int x1, int y1, byte luminuns) {
        if (x0 < 0 || y0 < 0 || x0 >= WIDTH || y0 >= HEIGHT) return;
        if (x1 < 0 || y1 < 0 || x1 >= WIDTH || y1 >= HEIGHT) return;
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int erro = (dx > dy ? dx : -dy) / 2;

        while (canvas[y0 * WIDTH + x0] = luminuns, x0 != x1 || y0 != y1) {
            int e2 = erro;
            if (e2 > -dx) {
                erro -= dy;
                x0 += sx;
            }
            if (e2 < dy) {
                erro += dx;
                y0 += sy;
            }
        }
    }

    //将画布渲染至屏幕
    void Show() {
        // 染色+字符
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH - 1; j++) {
                charInfoBuffer.buffer[i * WIDTH + j].Char.AsciiChar = brightness(canvas[WIDTH * i + j]);
                charInfoBuffer.buffer[i * WIDTH + j].Attributes = getColor(canvas[WIDTH * i + j]);
            }
        }

        // 为每一行的开头和结尾添加边界字符 '@'
        for (int i = 0; i < HEIGHT; ++i) {
            charInfoBuffer.buffer[WIDTH * i].Char.AsciiChar = '@';
            charInfoBuffer.buffer[WIDTH * i].Attributes = WHITE;
            charInfoBuffer.buffer[WIDTH * i + WIDTH - 2].Char.AsciiChar = '@';
            charInfoBuffer.buffer[WIDTH * i + WIDTH - 2].Attributes = WHITE;
        }

        // 为第一行和最后一行添加边界字符 '@'
        for (int j = 0; j < WIDTH - 1; ++j) {
            charInfoBuffer.buffer[j].Char.AsciiChar = '@';
            charInfoBuffer.buffer[j].Attributes = WHITE;
            charInfoBuffer.buffer[WIDTH * (HEIGHT - 1) + j].Char.AsciiChar = '@';
            charInfoBuffer.buffer[WIDTH * (HEIGHT - 1) + j].Attributes = WHITE;
        }

        COORD bufferSize = {(SHORT) WIDTH, (SHORT) HEIGHT};
        COORD bufferCoord = {0, 0};
        SMALL_RECT writeRegion = {0, 0, (SHORT) (WIDTH - 1), (SHORT) (HEIGHT - 1)};
        WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), charInfoBuffer.buffer, bufferSize, bufferCoord,
                           &writeRegion);
    }

    int Height() {
        return HEIGHT;
    }

    int Width() {
        return WIDTH;
    }

    ~Screen() {
        delete[] canvas;
        delete[] charInfoBuffer.buffer;
    }

private:
    byte *canvas = new byte[WIDTH * HEIGHT];
    CHAR_INFO_BUFFER charInfoBuffer;

    void Setup();

    void initCharInfoBuffer() {
        charInfoBuffer.size = {(SHORT) WIDTH, (SHORT) HEIGHT};
        charInfoBuffer.coord = {0, 0};
        charInfoBuffer.rect = {0, 0, (SHORT) (WIDTH - 1), (SHORT) (HEIGHT - 1)};
        charInfoBuffer.buffer = new CHAR_INFO[WIDTH * HEIGHT];
    }

    char brightness(byte n) {
        char s[] = " .,^:-+abcdwf$&%#@";
        return s[n * 18 / 256];
    }

    // 根据亮度值设置颜色
    int getColor(byte luminuns) {
        if (luminuns == 255)return 4; // 亮面
        else if (luminuns == 15)return 1;// 地面
        else return luminuns % 16; // 暗面
    }
};

void Screen::Setup() {
    // 获取标准输出的句柄
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    //全屏
    SetConsoleDisplayMode(hConsole, CONSOLE_FULLSCREEN_MODE, nullptr);

    // 设置字体大小
    CONSOLE_FONT_INFOEX cf = {0};
    cf.cbSize = sizeof cf;
    cf.dwFontSize.X = dW;
    cf.dwFontSize.Y = dH;
    wcscpy_s(cf.FaceName, L"Terminal");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), 0, &cf);
    //光标隐藏
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false; // 隐藏光标
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    HWND console = GetConsoleWindow();
    //设置控制台屏幕缓冲区大小
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hstdout, &csbi);
    csbi.dwSize.X = csbi.dwMaximumWindowSize.X;
    csbi.dwSize.Y = csbi.dwMaximumWindowSize.Y;
    SetConsoleScreenBufferSize(hstdout, csbi.dwSize);

    //设置为全屏模式
    SetConsoleDisplayMode(hstdout, CONSOLE_FULLSCREEN_MODE, 0);
    //移动窗口Y
    MoveWindow(console, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);
}