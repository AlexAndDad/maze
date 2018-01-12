//
// Created by Richard Hodges on 12/01/2018.
//

#include "clear_screen.hpp"

#if defined(__unix__) || defined(__APPLE__)

#include <stdio.h>

void clear_screen(void) {
    printf("\x1B[2J");
}

#elif defined(_WIN32)

#include <windows.h>

void clear_screen(void) {
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = {0, 0};
    DWORD dwCount, dwSize;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    dwSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hOutput, 0x20, dwSize, topLeft, &dwCount);
    FillConsoleOutputAttribute(hOutput, 0x07, dwSize, topLeft, &dwCount);
    SetConsoleCursorPosition(hStdOut, topLeft);
}

#endif /* __unix__ */
