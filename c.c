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
int bingo_check(int board[][BOARD_SIZE]);

//게임관련 구조체로 묶을 변수
struct Game{
int Game_on;
int game_turn;
int my_turn;
int my_bingo; 
int winFlag; //Wflag: 0진행 1패배 2무승부 3승리
int board[BOARD_SIZE][BOARD_SIZE];
int bingo[BOARD_SIZE][BOARD_SIZE];

};
struct Game MyGame ={0,};


//채팅관련 구조체로 묶을변수

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
	char set[111];
	sprintf(set,"%1s%10s","S",name);//이름을 최초 1회 보내는걸로 검증한다.
	write(sock, set, strlen(set));
	while (1) 
	{	
		char msg[BUF_SIZE];
		char chat[NAME_SIZE+BUF_SIZE+2];
		//배열버퍼, stdin버퍼 초기화

		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n")||!strcmp(msg,"Q\n"))//Q를 입력하면 종료로 인식한다.
		{
			close(sock);
			exit(0);
		}
		if(!strcmp(msg, "c\n")||!strcmp(msg,"C\n")) //C를 입력하면 채팅입력창을 출력한다.
		{
			printf("type msg: ");
			/*
			for(int i=0; i<BUF_SIZE;i++)
			{
				msg[i]='\0';
			}
			for(int i=0; i<NAME_SIZE+BUF_SIZE+1;i++)
			{
				chat[i]='\0';
			}
			*/
			fgets(msg, BUF_SIZE, stdin);
			msg[strlen(msg)-1]='\0';
			
			//입력받은 msg로 chat내용을 세그먼트화 한다. (채팅-10자리이름(공백으로 줄맞춤)-메세지내용)
			sprintf(chat,"%1s%10s%s","C",name,msg);
			write(sock, chat, strlen(chat));
			printf("[Debug]writed\n");
			
			
		}
		if(MyGame.my_turn==1&&!strcmp(msg,"N\n")) //내턴일때 N을 입력하면 숫자를 입력받는다.
		{
			while(1) {
				printf("NUM:");
				fgets(msg, BUF_SIZE, stdin);

				if(atoi(msg)==0) {
					continue;
				} else {
					msg[strlen(msg)-1]=10;//개행문자
					sprintf(chat,"%1s%10s%2s","N",name,msg);
					write(sock, chat, strlen(chat));
					MyGame.my_turn--;
				}
				break;
			}
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
	char chat[BUF_SIZE];
	char FLAG[1+NAME_SIZE+BUF_SIZE];
	ssize_t str_len;
	char msg[BUF_SIZE];
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
			if(msg[0]==87)//W로 시작하는 제어문이 오면 Wflag: 0진행 1패배 2무승부 3승리
			{
				if(strcmp(tmpName, name)==0){
					printf("\n승리플래그 메세지 검증%d\n",tmpMsg[0]);
					switch (tmpMsg[0]) {
						case 48 : MyGame.winFlag=0; break;
						case 49 : MyGame.winFlag=1; break;
						case 50 : MyGame.winFlag=2; break;
						case 51 : MyGame.winFlag=3; break;						
						default : MyGame.winFlag=-1; break;						
					}
				}
			}
			if(msg[0]==67) //C로 시작하는 채팅내역이오면
			{	
				strcpy(msgQ[4],msgQ[3]);
				strcpy(msgQ[3],msgQ[2]);
				strcpy(msgQ[2],msgQ[1]);
				strcpy(msgQ[1],msgQ[0]);
				sprintf(msgQ[0],"%s%s",tmpName,tmpMsg);
			}
			if(msg[0]==84)//T로 시작하는 제어문이 오면
			{
				if(strcmp(tmpName, name)==0) MyGame.my_turn++;
			}
			if(msg[0]==78)//N로 시작하는 제어문이 오면
			{
					//2자리문자열로 온 숫자를 아스키코드표에 따라 숫자로 변환
					//printf("숫자[%d][%d]",tmpMsg[0],tmpMsg[1]);
				int NUM=0;
				pintf("서버입력받아 변환할 값 %d %d",tmpMsg[0],tmpMsg[1]);
				if(tmpMsg[0]==32){NUM=tmpMsg[1]-48;}			
				else{NUM=(10*(tmpMsg[0]-48))+tmpMsg[1]-48;}
				//printf("받아서 변환된숫자: %d\n",NUM);
				for(int i=0; i<BOARD_SIZE;i++){
					for(int j=0; j<BOARD_SIZE;j++){
						if(MyGame.board[i][j]==NUM){
							MyGame.bingo[i][j]=1;
							MyGame.game_turn++;
							//printf("smp checker");
						}
					}
				  }
				for(int i=0; i<BUF_SIZE;i++)
				{
				FLAG[i]='\0';
				}
				MyGame.my_bingo=bingo_check(MyGame.bingo);
				//리시브가 모두 끝나고 난 뒤에, 승리플래그를 보낼지 검증해야한다.
				if(MyGame.my_bingo==3)
				{
					sprintf(FLAG,"%1s%10s%s","W",name,"1");
					int k= write(sock, FLAG, strlen(FLAG));
					if(k!=-1) {printf("[Debug][bingo3]writed\n");}
				}
				else{
					sprintf(FLAG,"%1s%10s%s","W",name,"0");
					write(sock, FLAG, strlen(FLAG));
					printf("[Debug]writed\n");
				}
			}
			for(int i=0; i<BUF_SIZE;i++){
				msg[i]='\0';
			}
			//UI표시부
			game_print(0);
			
						
		}
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

	printf("@----- client bingo -----@\n");
	printf("turn: %3d bingo: %3d\n", MyGame.game_turn, MyGame.my_bingo);
	printf("+----+----+----+----+----+\n");
	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			/*
			if (MyGame.board[i][j] == 0)
			{
				printf("| ");
				printf("%c[1;31m", 27);
				printf("%2c ", 88);
				printf("%c[1;33m", 27);
			}
			else
				printf("| %2d ", MyGame.board[i][j]);
			*/
			if(MyGame.bingo[i][j]==1){
				printf("|\033[1;31m %2d \033[1;33m", MyGame.board[i][j]);
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
	if(MyGame.winFlag==3){printf("YOU WIN!!\n");}
	else if(MyGame.winFlag==2){printf("DRAW!!\n");}
	else if(MyGame.winFlag==1){printf("LOSE\n");}
	else if(MyGame.winFlag==-1){printf("ERR\n");}
	else if(MyGame.my_turn==1){printf("its your turn\n");}
	else {printf("\n");}
	printf("=====================================\n");
	printf("5:%s \n4:%s \n3:%s \n2:%s \n1:%s \n",msgQ[4],msgQ[3],msgQ[2],msgQ[1],msgQ[0]);
	printf("=====================================\n");
	printf("C to chat,R to Ready,N to Number Q to quit\n");
}
int bingo_check(int board[][BOARD_SIZE])
{
	int i;
	int count=0;

	for(i=0; i < BOARD_SIZE; i++) //가로
	{
		if(board[i][0]==1&&board[i][1]==1&&board[i][2]==1&&board[i][3]==1&&board[i][4]==1) //가로
		{
			count++;
		}
		if(board[0][i]==1&&board[1][i]==1&&board[2][i]==1&&board[3][i]==1&&board[4][i]==1) //세로
			count++;
	}
	if(board[0][0]==1&&board[1][1]==1&&board[2][2]==1&&board[3][3]==1&&board[4][4]==1)
		count++;
	if(board[0][4]==1&&board[1][3]==1&&board[2][2]==1&&board[3][1]==1&&board[4][0]==1)
		count++;
	return count;
}
