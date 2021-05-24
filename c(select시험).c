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

//게임관련 구조체로 묶을 변수
struct Game{
int Game_on;
int game_turn;
int my_turn;
int my_bingo;
int board[BOARD_SIZE][BOARD_SIZE];
int bingo[BOARD_SIZE][BOARD_SIZE];

};
struct MyGame ={0,0,0,0};


//채팅관련 구조체로 묶을변수
char chat[NAME_SIZE+BUF_SIZE+1];
char msgQ[5][NAME_SIZE+BUF_SIZE];

//main의 매개변수용 char
char name[NAME_SIZE]="[DEFAULT]";

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
					MyGame.board[i][j] = temp + 1;
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
	if(1){
	close(sock);
	return 0;}
}

void* send_msg(void* arg) {
	int sock = *((int*)arg);
	char msg[BUF_SIZE];
	sprintf(chat,"%1s%10s","S",name);//이름을 최초 1회 보내는걸로 검증한다.
	write(sock, chat, strlen(chat));
	while (1) 
	{	
		//배열버퍼, stdin버퍼 초기화
		for(int i=0; i<BUF_SIZE;i++)
			{
				msg[i]='\0';
			}
		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n")||!strcmp(msg,"Q\n"))//Q를 입력하면 종료로 인식한다.
		{
			close(sock);
			exit(0);
		}
		if(!strcmp(msg, "c\n")||!strcmp(msg,"C\n")) //C를 입력하면 채팅입력창을 출력한다.
		{
			chatUI(1);
			fgets(msg, BUF_SIZE, stdin);
			msg[strlen(msg)-1]='\0';//개행문자 제거
			//입력받은 msg로 chat내용을 세그먼트화 한다. (채팅-10자리이름(공백으로 줄맞춤)-메세지내용)
			sprintf(chat,"%1s%10s%s","C",name,msg);
			
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
		}
		if(MyGame.my_turn==1&&!strcmp(msg,"N\n")) //내턴일때 N을 입력하면 숫자를 입력받는다.
		{
			chatUI(1);
			fgets(msg, BUF_SIZE, stdin);
			msg[strlen(msg)-1]='\0';//개행문자 제거
			//입력받은 msg로 chat내용을 세그먼트화 한다. (채팅-10자리이름(공백으로 줄맞춤)-메세지내용)
			sprintf(chat,"%1s%10s%s","N",name,msg);
			write(sock, chat, strlen(chat));
			MyGame.my_turn--;
			printf("[Debug]writed\n");
		}
		else if(!strcmp(msg, "r\n")||!strcmp(msg,"R\n")) //R를 입력하면 레디내역을 서버에보낸다.
		{
				for(int i=0; i<BUF_SIZE;i++)
				{
				msg[i]='\0';
				}
			sprintf(chat,"%1s%10s","R",name);
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
	ssize_t str_len;
	while (1)
	{
		if(str_len=read(sock, msg, 1+BUF_SIZE+NAME_SIZE)!=0){
			char tmpName[10]; //
			for(int i=0,j=0;i<10;i++){
			if(msg[i+1]!=32) {tmpName[j++]=msg[i+1];}
			}
			char tmpMsg[100]; //
			for(int i=0;i<111;i++){
			tmpMsg[i]=msg[i+11];
			}
			system("clear");
			if(strcmp(msg,"GAMEON")==0) MyGame.Game_on=1;
			if(msg[0]==67) //C로 시작하는 채팅내역이오면
			{
				//for(int i=0; i<BUF_SIZE-1;i++){
				//msg[i]=msg[i+1];
				strcpy(msgQ[4],msgQ[3]);
				strcpy(msgQ[3],msgQ[2]);
				strcpy(msgQ[2],msgQ[1]);
				strcpy(msgQ[1],msgQ[0]);
				strcpy(msgQ[0],msg);
			}
			if(msg[0]==84)//T로 시작하는 제어문이 오면
			{
				if(strcmp(tmpName, name)==0) MyGame.my_turn++;
			}
			if(msg[0]==78)//N로 시작하는 제어문이 오면
			{
				if(strcmp(tmpName,"SERV")==0)printf("GET NUMBER!");
			}
			//UI표시부
			game_print(0);
		
		}
		//if(str_len==1+BUF_SIZE+NAME_SIZE) game_print(0);
		
	}
	return NULL;
}
void* game_set(void* arg){
	int sock = *((int*)arg);
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void game_print(int any)
{
	if(MyGame.Game_on==1){
	int i, j, x;
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
				printf("| %2d ", MyGame.board[i][j]);
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
	else{
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}
	printf("=====================================\n");
	if(MyGame.my_turn==1) printf("its your turn\n");
	else printf("\n");
	printf("=====================================\n");
	printf("5:%s \n4:%s \n3:%s \n2:%s \n1:%s \n",msgQ[0],msgQ[1],msgQ[2],msgQ[3],msgQ[4]);
	printf("=====================================\n");
	printf("C to chat,R to Ready,N to Number Q to quit\n");
}
void chatUI(int s){
	switch(s){
		case 0:
			printf("Q(quit)C(chat)R(ready)\n");
			break;
		case 1:
			printf("input : ");
			break;
		default :
			printf("wrong chatUI call\n");
	}
}
