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
#define NAME_SIZE 10
#define BUF_SIZE 100

void* send_msg(void* arg);
void* recv_msg(void* arg);
void* game_set(void* arg);

void error_handling(char* mse); 
void game_print(int any);
void chatUI(int s);

int Game_on=0;
int my_turn=0;
int board[BOARD_SIZE][BOARD_SIZE];
char name[NAME_SIZE]="[DEFAULT]";
char chat[NAME_SIZE+BUF_SIZE+1];

	

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



	if (argc != 4) {
		printf("ip, port, name");
		exit(1);
	}
	sprintf(name,"[%s]",argv[3]);

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect err");
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_create(&gam_thread, NULL, game_set, (void*)&sock);
	//pthread_detach(snd_thread);
	//pthread_detach(rcv_thread);
	//pthread_detach(gam_thread);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	pthread_join(gam_thread, &thread_return);

	close(sock);
	return 0;
}

void* send_msg(void* arg) {
	int sock = *((int*)arg);
	char msg[BUF_SIZE];
	sprintf(chat,"%s%s","N",name);//이름을 최초 1회 보내는걸로 검증한다.
	write(sock, chat, strlen(chat));
	while (1) 
	{
		fgets(msg, BUF_SIZE-1, stdin);
		if (!strcmp(msg, "q\n")||!strcmp(msg,"Q\n"))//Q를 입력하면 종료로 인식한다.
		{
			close(sock);
			exit(0);
		}
		else if(!strcmp(msg, "c\n")||!strcmp(msg,"C\n")) //C를 입력하면 채팅입력창을 출력한다.
		{
			//C를 인식하였다. 채팅을 입력받기전에 msg[]를 null로 초기화 하여 오류를 방지한다.
			for(int i=0; i<BUF_SIZE;i++)
			{
				msg[i]='\0';
			}
			//초기화된 msg[]에 채팅 내역을 다시 받는다.
			chatUI(1);
			fgets(msg, BUF_SIZE, stdin);
			msg[strlen(msg)-1]='\0';//개행문자 제거
			//입력받은 msg로 chat내용을 세그먼트화 한다. (채팅-10자리이름(공백으로 줄맞춤)-메세지내용)
			sprintf(chat,"%1s%10s%s","C",name,msg);
			
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
		}
		else if(!strcmp(msg, "r\n")||!strcmp(msg,"R\n")) //R를 입력하면 레디내역을 서버에보낸다.
		{
				for(int i=0; i<BUF_SIZE;i++)
				{
				msg[i]='\0';
				}
			sprintf(chat,"%s%s","R",name);
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
		}
		else if(strcmp(msg, "n\n")&&Game_on!=0) //n을 입력하였고, 게임이 시작되었으며, 내턴이면 숫자를 서버에 보낸다.
		{
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
		str_len = read(sock, msg, 1+BUF_SIZE+NAME_SIZE);
		if (str_len == -1)
			return (void*)-1;
		if(strcmp(msg,"GAMEON")==0)
		{
			Game_on=1;
		}
		
	}
	return NULL;
}
void* game_set(void* arg){
	int sock = *((int*)arg);
	fd_set reads, cpy_reads;
	int fd_max, str_len, fd_num;
	struct timeval timeout;
	FD_ZERO(&reads);
	FD_SET(sock, &reads);
	fd_max=sock;
	
	while(1){
		cpy_reads=reads;
		timeout.tv_sec=5;
		timeout.tv_usec=5000;
		if((fd_num=select(fd_max+1,&cpy_reads,0,0,&timeout))==-1) break;
		if(fd_num==0)continue;
		for(int i=0; i<fd_max+1;i++){
			if(FD_ISSET(i,&cpy_reads)){
				if(Game_on==1){game_print(0);}
				printf("=====================================\n");
				if(my_turn==1) printf("its your turn");
				else printf("\n");
				printf("=====================================\n");
				printf("C to chat,R to Ready, Q to quit\n");
				system("clear");
			}		
		}
	}
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
	printf("turn: %d\n", 999);
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
	
	printf("%c[0m", 27);
	/*
	if (turn_count != 0)
	{
		printf("number: %d\n", 1);
		printf("bingo count: %d\n", 1);
	}*/
}
void chatUI(int s){
	switch(s){
		case 0:
			printf("Q(quit)C(chat)R(ready)\n");
			break;
		case 1:
			printf("input chat : ");
			break;
		default :
			printf("wrong chatUI call\n");
	}
}
