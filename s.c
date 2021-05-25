#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define BOARD_SIZE 5
#define NAME_SIZE 10
#define BUF_SIZE 100
#define BACKLOG 3 
#define MAX_CLNT 256

void error_handling(char* mse);
void* handle_clnt(void* arg);
void* handle_game(void* arg);
void* status_board(void* arg);
void send_msg(char* msg, int len,int index);


int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
int Game_on=0;//게임이 시작되면 1이 되고, 추가 접속을 막는다. 게임이 끝나면 2가 되고, exit한다(쓰레드정리+동적할당정리+main끝)
int win_check =0;//RCV쓰레드와 GM쓰레드가 함 사용하므로 전역변수
		//가장 어려운 세마포를 제어하는 전역변수이다. 클라이언트가 N(숫자)를 입력할때 마다 win_check이 1씩 증가한다.
		//하지만 서버쪽에서 실제로는 win_check이 clnt_cnt만큼 증가한다. 클라이언트가 1번 입력하면 AUTO_SendALL을 통해 모든 클라이언트가 그값에 반응을 하기때문이다.
		//그러므로 win_check이 clnt_cnt과 같다면 단일 클라이언트가 1번입력한것에 대해 모든 클라이언트가 반응했다는 뜻이다.
		//즉 내가 빙고를 완성했을때, 다른 클라이언트도 반응했는지 체크하고, 통신이 원활하게 이루어졌는지 검증할수 있게된다.
		
struct Clnt{
	char IP[16];
	int PORT;
	char NAME[10];//동적할당으로 수정전에 NAME_SIZE를 10으로 임의 
	int R;//0은 준비중 1은 준비완료 2는 게임중 3은 게임중+
	int Bingo;//
};
struct Clnt C[MAX_CLNT]; //what a massive
char msgQ[5][NAME_SIZE+BUF_SIZE]; //SND쓰레드와 RCV쓰레드가 함께 사용하므로 전역변수
pthread_mutex_t mutx;
pthread_t t_id;
pthread_t t_id2;
pthread_t t_id3;

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	char name[NAME_SIZE]="[DEFAULT]";
	pthread_t t_id;
	if (argc != 2) {
		printf("insert port.");
	}
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr)); 
	serv_adr.sin_family = AF_INET; 
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_adr.sin_port = htons(atoi(argv[1])); 

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == 1) {
		error_handling("bind() error");
	}
	if (listen(serv_sock, 5) == -1) {
		error_handling("listen() error");
	}
	printf("var reset, ip port allocate, listen, setting all cleared. start in 5sec \n");
	sleep(5);
	
	pthread_create(&t_id2, NULL, status_board, (void*)&clnt_sock);
	pthread_detach(t_id2);
	
	pthread_create(&t_id3, NULL, handle_game, (void*)&clnt_sock);
	pthread_detach(t_id3);
	
	while (Game_on!=1) {
		int str_len;
		char msg[1+NAME_SIZE+BUF_SIZE];
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		strcpy(C[clnt_cnt].IP,inet_ntoa(clnt_adr.sin_addr));//클라이언트 주소저장
		C[clnt_cnt].PORT=ntohs(clnt_adr.sin_port);//클라이언트의 포트저장
		C[clnt_cnt].R=0;//클라이언트가 레디하지않음으로 설정
		clnt_cnt++;
	}
	if(Game_on!=2){
	close(serv_sock);
	return 0;
	}
}

void error_handling(char* msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* handle_clnt(void* arg) {//클라이언트를 1대1로 담당하는 쓰레드이자, 수신을 담당하는 쓰레드. 이쓰레드에서 사용되는 변수는 전역변수로 설정할지 검증되어야 한다.
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	//int win_check=0; //전역변수로  되었음
	char msg[1+NAME_SIZE+BUF_SIZE];
	for(int i=0; i<BUF_SIZE;i++){
				msg[i]='\0';
			}
	//handle_clnt의 메세지 수신부분
	send_msg("",1,0);//서로 연결이 확정되면 의미없는 문장을 보내서, 클라이언트의 RCV와 game_print를 활성화시킨다
	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
	{	
		//입력에 대한 기본처리
		printf("[Debug]red is it correct? :%s\n",msg);
		char tmpName[10]; //
			for(int i=0,j=0;i<10;i++){
			if(msg[i+1]!=32) {tmpName[j++]=msg[i+1];}
			}
		char tmpMsg[100]; //
			for(int i=0;i<111;i++){
			tmpMsg[i]=msg[i+11];
			}
	
		 //순서1.W로 시작하는 숫자내역이왔을때 처리. 맨위에 있어야 무승부 처리 순서에 올바르다.
		if(msg[0]==87)
		{
			for(int i=0; i<clnt_cnt;i++){
				if(tmpMsg[0]==49&&(strcmp(C[i].NAME,tmpName)==0))
				{
					printf("[Debug]BINGO!!");
					C[i].Bingo=1;
				}
			}
			pthread_mutex_lock(&mutx);
			win_check++;//반드시 값처리가 끝난뒤 win_check을 올려줘야한다.
			printf("[Debug]win_check++\n");
			pthread_mutex_unlock(&mutx);
		}
		//순서2.C로 시작하는 채팅내역이오면
		if(msg[0]==67)
		{
			char tmpNameMsg[110];
			//sprintf(tmpNameMsg,"%s",tmpMsg);
			sprintf(tmpNameMsg,"%s%s",tmpName,tmpMsg);
		strcpy(msgQ[4],msgQ[3]);
		strcpy(msgQ[3],msgQ[2]);
		strcpy(msgQ[2],msgQ[1]);
		strcpy(msgQ[1],msgQ[0]);
		strcpy(msgQ[0],tmpNameMsg);
		send_msg(msgQ[0], 1+NAME_SIZE+BUF_SIZE,1);
		}
		
			//S로 시작하는 네임세팅이 오면
			if(msg[0]==83)
			{
				strcpy(C[clnt_cnt-1].NAME,tmpName);
			}
			
			//R로 시작하는 레디내역이오면
			if(msg[0]==82) 
			{
				for(int i=0; i<clnt_cnt;i++){
					if(strcmp(C[i].NAME,tmpName)==0){C[i].R++;}
					//printf("C[i].NAME:%s tmp2:%s tmp:%s\n",C[i].NAME,tmp2,tmp);
				}
				send_msg("",1,2);//의미없는문자열을 보내서 클라이언트쪽 화면을 제어해준다
			}
		
			//N로 시작하는 숫자내역이오면
			if(msg[0]==78) 
			{
				for(int i=0; i<clnt_cnt;i++)
				{
					if(strcmp(C[i].NAME,tmpName)==0)
					{
						C[i].R=2;
						char tmp[1+NAME_SIZE+BUF_SIZE];
						if(i==clnt_cnt-1){
							C[0].R=3;
							sprintf(tmp,"%1s%10s","T",C[0].NAME);
							send_msg(tmp,1+NAME_SIZE+BUF_SIZE,5);		
						}
						else{
							C[i+1].R=3;
							sprintf(tmp,"%1s%10s","T",C[i+1].NAME);
							send_msg(tmp,1+NAME_SIZE+BUF_SIZE,5);		
						}
						char tmp2[1+NAME_SIZE+BUF_SIZE];
						sprintf(tmp2,"%1s%10s%2s","N","SERV",tmpMsg);
						send_msg(tmp2,1+NAME_SIZE+BUF_SIZE,5);						
					}		
				}
			}
		
	}
	//본인이 담당하면 클라이언트가 끊어졌다면, 서버에서 클라이언트의 정보를 지우고 재설정한다.
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) //eliminated disconnections
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
				memcpy(&C[i],&C[i+1],sizeof(struct Clnt));
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void* handle_game(void* arg){
	while(1)
	{
		char tmp[1+BUF_SIZE+NAME_SIZE];
		//sum으로 R준비여부를 수집하여 연산
		if(clnt_cnt>1&&C[0].R==1&&Game_on==0)
		{
			
			int sum=0;
			for(int i=0; i<clnt_cnt;i++)
			{
				sum+=C[i].R;
			}
			if(sum==clnt_cnt)
			{	
				//게임을 시작하는 동안에는 다른 연산을 멈추고 게임에 맞도록 변수를 설정한다.
				pthread_mutex_init(&mutx, NULL);
				send_msg("GAMEON",1+BUF_SIZE+NAME_SIZE,3);
				sleep(1);
				send_msg("GAMEON",1+BUF_SIZE+NAME_SIZE,3);//왜 인지 모르겠지만 가장 전송누락이 잦은 부분. 주의
				for(int i=0;i<clnt_cnt;i++)
				{
					C[i].R=2;
				}
				C[0].R=3;
				sprintf(tmp,"%1s%10s","T",C[0].NAME);
				send_msg(tmp,1+BUF_SIZE+NAME_SIZE,4);
				pthread_mutex_unlock(&mutx);
			}
		}
		//w승리 배열에 승리여부를 총집합한뒤, 여러명이 승리했으면 무승부, 1명이 승리했으면 승리.(나머지인원에게는 패배)
		//핸들클라이언트 쓰레드들은 지금 if(win_check==3)에 의해 입력이 제한된 상태이다. 
		//혹시라도 win_check이나 c[].bingo를 가져오는 동안 값이 변조될수있다.(CASE:게임핸들 쓰레드가 연산을 하는중 클라이언트가 뒤늦게 발송)
		pthread_mutex_init(&mutx, NULL);
		if((clnt_cnt!=0)&&(win_check==clnt_cnt))
		{
			int cnt=0;
			for(int i=0; i<clnt_cnt;i++)
			{
				if(C[i].Bingo==1) 
				{
					cnt++;
				}
			}
			//Wflag: 0진행 1패배 2무승부 3승리
			if(cnt==1)//1명의 단독승리
			{
				for(int i=0; i<clnt_cnt; i++){
					if(C[i].Bingo==1){
						sprintf(tmp,"%1s%10s%s","W",C[i].NAME,"3");
						send_msg(tmp,1+BUF_SIZE+NAME_SIZE,7);
					}
					else{
						sprintf(tmp,"%1s%10s%s","W",C[i].NAME,"1");
						send_msg(tmp,1+BUF_SIZE+NAME_SIZE,8);
					}
				}
			}
			else if(cnt>1)//무승부
			{
				for(int i=0; i<clnt_cnt; i++){
					if(C[i].Bingo==1){
						sprintf(tmp,"%1s%10s%s","W",C[i].NAME,"2");
						send_msg(tmp,1+BUF_SIZE+NAME_SIZE,9);
					}
					else{
						sprintf(tmp,"%1s%10s%s","W",C[i].NAME,"1");
						send_msg(tmp,1+BUF_SIZE+NAME_SIZE,10);
					}
				}
			}
			printf("[Debug]입력검증끝");
			win_check=0;
		}
		pthread_mutex_unlock(&mutx);
	}
}
void* status_board(void* arg){
	while(1){
	//접속클라이언트 현황
		printf("CLNT\t|IP\t\t|PORT\t|NAME\t|Ready\t\t|BINGO\t\t|\n");
		for(int i=0; i<clnt_cnt;i++){
		printf("%d\t|%s\t|%d\t|%s\t|",i,C[i].IP,C[i].PORT,C[i].NAME);
		if(C[i].R==0)printf("WAIT\t\t|%d\t\t|\n",C[i].Bingo);
		if(C[i].R==1)printf("READY\t\t|%d\t\t|\n",C[i].Bingo);
		if(C[i].R==2)printf("INGAME\t\t|%d\t\t|\n",C[i].Bingo);
		if(C[i].R==3)printf("TRUN\t\t|%d\t\t|\n",C[i].Bingo);
		}
	//채팅현황
		printf("\n================================\n");
		printf("5 recnet msgs\n");
		for(int i=0; i<5;i++){
		printf("%d:%s\n",i,msgQ[i]);
		}
	//게임 현황
		printf("\n================================\n");
		printf("입력검증 %d / %d",win_check,clnt_cnt*clnt_cnt);
	//디버그 출력
		printf("\n================================\n");
	//딜레이
		sleep(8);
		system("clear");
	}
}
void send_msg(char* msg, int len, int index) {//index는 디버그용, 아무값이나 넣어도됌
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
	printf("[Debug] %d sendALL\n",index);
}

