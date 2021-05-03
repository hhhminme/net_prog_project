#ifndef LINUX_KBHIT_H_
#define LINUX_KBHIT_H_
//리눅스에서 kbhit(conio.h)를 구현
//출처: https://anythink.tistory.com/entry/Linux-linux에서-getch-및-kbhit-사용하기 [투명잉크]
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

 

int linux_kbhit(void)
{
    struct termios oldt, newt;
    int ch;

 

    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;

 

    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );

 

    ch = getchar();

 

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

 

    return ch;
}

 

#endif