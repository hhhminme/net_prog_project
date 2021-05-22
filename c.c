#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>

#define BOARD_SIZE 5
#define BUF_SIZE 100

void* send_msg(void* arg);
void* recv_msg(void* arg);

void error_handling(char* mse); 
void game_print(int any);
void chatUI(int s);

int turn_count = 0;
int board[BOARD_SIZE][BOARD_SIZE];
char* chatting[BUF_SIZE];
char chat[BUF_SIZE];
int chattingCount = 0;
	

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread, gam_thread;
	void* thread_return;

	srand(time(NULL)); 
	int check_number[BOARD_SIZE * BOARD_SIZE] = { 0 }; 
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			while (1)
			{
				int temp = rand() % 25; 

				if (check_number[temp] == 0) 
				{
					check_number[temp] = 1;
					board[i][j] = temp + 1;
					break;
				}
			}
		}
	}



	if (argc != 3) {
		printf("ip, port");
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect err");

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);

	close(sock);
	return 0;
}

void* send_msg(void* arg) {
	int sock = *((int*)arg);
	char msg[BUF_SIZE-1];
	while (1) {
		chatUI(0);
		fgets(msg, BUF_SIZE-1, stdin);
		if (!strcmp(msg, "q\n")||!strcmp(msg,"Q\n"))//Q를 입력하면 종료로 인식한다.
		{
			close(sock);
			exit(0);
		}
		else if(!strcmp(msg, "c\n")||!strcmp(msg,"C\n")) //C를 입력하면 채팅입력창을 출력한다.
		{
			//C를 인식하였다. 채팅을 입력받기전에 msg[]를 null로 초기화 하여 오류를 방지한다.
			for(int i=0; i<BUF_SIZE;i++){
			msg[i]='\0';
			}
			//초기화된 msg[]에 채팅 내역을 다시 받는다.
			chatUI(1);
			fgets(msg, BUF_SIZE, stdin);
			//msg[]의 맨앞에 C를 끼워넣어서 서버에게 보낸다. 서버가 문자열을 클라이언트에게 재송신하면 recive_msg가 문자열이 C로 시작하는지if처리한다.
			chat[0]='C';
			for(int i=0; i<BUF_SIZE-1;i++){
			chat[i+1]=msg[i];
			}
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
		}
	}
	return NULL;
}
void* recv_msg(void* arg) {
	int sock = *((int*)arg);
	char msg[BUF_SIZE];
	char chat[BUF_SIZE];
	int str_len;
	while (1) {
		str_len = read(sock, msg, BUF_SIZE - 1);
		printf("[Debug]red\n");
		if (str_len == -1)
			return (void*)-1;
		msg[str_len] = 0;

		if(atoi(msg) == 0) 
		{
			chatting[chattingCount++] = msg;
		}
		printf("\t");
		fputs(msg, stdout);
		//game_print(0);
	}
	return NULL;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void game_print(int any)
{
	int i, j, x;

	system("clear"); 
	printf("%c[1;33m", 27); 

	printf("@------ client bingo ------@\n");
	printf("turn: %d\n", turn_count++);
	printf("+----+----+----+----+----+\n");
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			if (board[i][j] == 0)
			{
				printf("| ");
				printf("%c[1;31m", 27);
				printf("%2c ", 88);
				printf("%c[1;33m", 27);
			}
			else
				printf("| %2d ", board[i][j]);
		}
		printf("|\n");
		printf("+----+----+----+----+----+\n");
	}
	for (x = 0; x < chattingCount; x++) {
		printf("%s\n", chatting[x]);
	}
	printf("%c[0m", 27); 
	if (turn_count != 0)
	{
		printf("number: %d\n", 1);
		printf("bingo count: %d\n", 1);
	}
}
void chatUI(int s){
	switch(s){
		case 0:
			printf("\tQ,q : quit | C,c : chat | in ur turn, type number u want\n");
			break;
		case 1:
			printf("\tinput chat : ");
			break;
		default :
			printf("\twrong chatUI call\n");
	}
}