#include <stdio.h>
#include <iostream>
#include "linux_kbhit.h"
//키보드 입력시 입력을 화면으로 반환하는 프로그램
//화살표입력시 3개씩 반환하므로 WASD기반으로 향후 코딩하는 것이 네트워크 환경구축에 쉬움.(값을 서버에게 3개씩 보내면 더 어려울수있음)

//출처: https://anythink.tistory.com/entry/Linux-linux에서-getch-및-kbhit-사용하기 [투명잉크]
 

int main(void)
{
    int previous_keystroke = 0;
   
    printf("\n");
    printf("Type any key to see the return ASCII int of the key pressed.\n");
    printf("The program will start in 1sec. Press <Esc> to quit...\n\n");
    sleep(1);
   
    while(1)
    {
        int value = linux_kbhit();
       
        // key stroke detected
        if (value != -1)
        {
            // printing of key integer value
            printf("%d\n", value);
        }

 

        // detection of <Esc> key.    # of integer value set:    1
        //                            integer value:            27
        if ((previous_keystroke == 27) && (value == -1))
            break;
        previous_keystroke = value;
       
        // printing of '.'s to prove non-blocking of kbhit()
        printf(".");
    }
   
    printf("\n");
    printf("<Esc> key pressed. Bye bye\n\n");
   
    return 0;
}
