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
#define BACKLOG 3 //연결대기 큐 숫자
#define MAX_CLNT 256

void socket_settings(char *port); //소켓의 세팅
void error_check(int validation, char *message); //실행 오류 검사
void server_game_init(); //서버 빙고판 생성
void client_game_init(); //클라이언트 빙고판 생성
void game_print(int turn_count); //빙고판 화면출력
void server_turn(); //서버 차례
void client_turn(); //클라이언트 차례
void board_X(int board[][BOARD_SIZE], int number); //빙고판에 X 체크
void game_run(); //게임 진행 및 승리여부 체크
int bingo_check(int board[][BOARD_SIZE]); //빙고 줄 검사, 게임 종료조건 검사
void read_childproc(int sig);
void* handle_clnt(void *arg);

int server_board[BOARD_SIZE][BOARD_SIZE]; //서버 보드판 배열
int client_board[BOARD_SIZE][BOARD_SIZE]; //클라이언트 보드판 배열
int check_number[BOARD_SIZE*BOARD_SIZE+1]={0}; //중복검사용 배열
int server_fd, client_fd; //소켓 파일디스크립터
int turn[4]; //어플리케이션 프로토콜 정의
/*
	turn[0]=플레이어 숫자선택
	turn[1]=클라이언트 빙고 수
	turn[2]=서버 빙고 수
	turn[3]=게임종료여부(0=진행중, 1=클라이언트 승리, 2=서버 승리, 3=무승부)
*/

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
pthread_t t_id;

void main(int argc, char *argv[])
{
	int i, j;

	if(argc!=2)
	{
		fprintf(stderr, "./실행파일 port번호 형식으로 입력해야합니다\n");
		exit(1);
	}

	socket_settings(argv[1]);

	printf("빙고게임을 시작합니다.\n");
	server_game_init();
	client_game_init();
	game_print(0);

	handle_clnt(argv[1]);

	close(client_fd);
	close(server_fd);

	printf("빙고게임을 종료합니다\n");
}
void socket_settings(char *port)
{
	int state;
	struct sockaddr_in server_adr, client_adr;
	socklen_t client_adr_size;

	server_fd=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //TCP 소켓 생성
	error_check(server_fd, "소켓 생성");

	memset(&client_adr, 0, sizeof(client_adr)); //구조체 변수 값 초기화
	memset(&server_adr, 0, sizeof(server_adr)); //구조체 변수 값 초기화
	server_adr.sin_family=AF_INET; //IPv4
	server_adr.sin_port=htons(atoi(port)); //포트 할당
	server_adr.sin_addr.s_addr=htonl(INADDR_ANY); //IP주소 할당

	error_check(bind(server_fd, (struct sockaddr *)&server_adr,sizeof(server_adr)), "소켓주소 할당"); //주소 바인딩
	error_check(listen(server_fd, BACKLOG), "연결요청 대기");

	while(1) {
		client_adr_size=sizeof(client_adr);
		client_fd=accept(server_fd, (struct sockaddr *)&client_adr, &client_adr_size); //특정 클라이언트와 데이터 송수신용 TCP소켓 생성

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = client_fd;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&client_adr);
		pthread_detach(t_id);

		printf("* %s:%d의 연결요청\n", inet_ntoa(client_adr.sin_addr), ntohs(client_adr.sin_port));	
		error_check(client_fd, "연결요청 승인");
	}
}

void error_check(int validation, char* message)
{
	if(validation==-1)
	{
		fprintf(stderr, "%s 오류\n", message);
		exit(1);
	}
	else
	{
		fprintf(stdout, "%s 완료\n", message);
	}
}
void server_game_init()
{
	int check_number[BOARD_SIZE*BOARD_SIZE]={0}; //전역변수 check_number와는 다른 중복제거용 배열
	int i, j;

	srand(time(NULL)); //시간을 인자로 주어서 완전한 랜덤을 구현

	for(i=0; i < BOARD_SIZE; i++)
	{
		for(j=0; j < BOARD_SIZE; j++)
		{
			while(1)
			{
				int temp = rand()%25; //0~24 난수 발생
				
				if(check_number[temp]==0) //중복제거 알고리즘
				{
					check_number[temp]=1;
					server_board[i][j]=temp+1;
					break;
				}
			}
		}
	}      
}
void client_game_init()
{
	int check_number[BOARD_SIZE*BOARD_SIZE]={0};
	int i, j;
	int array_len;

	srand(time(NULL)+100); //서버 보드판과 다르게 하기위해 100을 추가

	for(i=0; i < BOARD_SIZE; i++)
	{
		for(j=0; j < BOARD_SIZE; j++)
		{
			while(1)
			{
				int temp = rand()%25; //0~24 난수 발생
				
				if(check_number[temp]==0) //중복제거 알고리즘
				{
					check_number[temp]=1;
					client_board[i][j]=temp+1;
					break;
				}
			}
		}
	}      
	array_len=write(client_fd, client_board, sizeof(client_board));
	printf("%d 바이트를 전송하였습니다\n", array_len);
	error_check(array_len, "데이터전송");
}
void game_print(int turn_count)
{
	int i, j;

//	system("clear"); //동적 효과를 위한 화면 초기화
	printf("%c[1;33m",27); //터미널 글자색을 노랑색으로 변경
	
	printf("@------ 서버 빙고판 ------@\n");
	printf("진행 턴수: %d\n", turn_count); 
	printf("+----+----+----+----+----+\n"); 
	for(i=0; i < BOARD_SIZE; i++)
	{
		for(j=0; j < BOARD_SIZE; j++)
		{
			if(server_board[i][j]==0)
			{
				printf("| ");
				printf("%c[1;31m",27);
				printf("%2c ", 88); 
				printf("%c[1;33m",27);
			}
			else
				printf("| %2d ", server_board[i][j]); 
		}
		printf("|\n");
		printf("+----+----+----+----+----+\n"); 
	}      
	printf("%c[0m", 27); //터미널 글자색을 원래색으로 변경
}
int bingo_check(int board[][BOARD_SIZE])
{
	int i;
	int count=0;

	for(i=0; i < BOARD_SIZE; i++) //가로
	{
		if(board[i][0]==0&&board[i][1]==0&&board[i][2]==0&&board[i][3]==0&&board[i][4]==0) //가로
		{
			count++;
		}
		if(board[0][i]==0&&board[1][i]==0&&board[2][i]==0&&board[3][i]==0&&board[4][i]==0) //세로
			count++;
	}
	if(board[0][0]==0&&board[1][1]==0&&board[2][2]==0&&board[3][3]==0&&board[4][4]==0)
		count++;
	if(board[0][4]==0&&board[1][3]==0&&board[2][2]==0&&board[3][1]==0&&board[4][0]==0)
		count++;
	return count;
}
void board_X(int board[][BOARD_SIZE], int number)
{
	int i, j;
	
	for(i=0; i < BOARD_SIZE; i++)
	{
		for(j=0; j < BOARD_SIZE; j++)
		{
			if(board[i][j]==number)
				board[i][j]=0; //X표 처리
		}
	}
}
void server_turn()
{
	int array_len;

	while(1)
	{
		turn[0]=rand()%25+1;
		if(check_number[turn[0]]==0)
		{
			check_number[turn[0]]=1;
			break;
		}
	}
	game_run();

	array_len=write(client_fd, turn, sizeof(turn));
	printf("%d 바이트: 서버의 턴 정보를 전송하였습니다\n", array_len);
	error_check(array_len, "데이터전송");
}
void client_turn()
{
	int recv_len=0, array_len;

	while(recv_len!=sizeof(turn)) // 패킷이 잘려서 올수도 있으므로 예외처리를 위한 조건문
	{
		int recv_count;

		recv_count=read(client_fd, turn, sizeof(turn));
		error_check(recv_count, "데이터수신");
		if(recv_count==0) break;
		printf("%d 바이트: 클라이언트의 턴 정보를 수신하였습니다\n", recv_count);
		recv_len+=recv_count;
	}
	game_run();
	check_number[turn[0]]=1;

	array_len=write(client_fd, turn, sizeof(turn));
	printf("%d 바이트: 클라이언트의 턴 정보를 전송하였습니다\n", array_len);
	error_check(array_len, "데이터전송");
}
void game_run()
{
	board_X(server_board, turn[0]);
	board_X(client_board, turn[0]);
	turn[1]=bingo_check(client_board);
	turn[2]=bingo_check(server_board);

	if(turn[1]>=5&&turn[2]>=5)
		turn[3]=3; //무승부
	else if(turn[1]>=5)
		turn[3]=1; //클라이언트 승리
	else if(turn[2]>=5)
		turn[3]=2; //서버 승리
}

void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n", pid);
}

void* handle_clnt(void *arg) {
	int clnt_sock=*((int*)arg);
	int i, j;
	for(i=1;i<=BOARD_SIZE*BOARD_SIZE;i++)
	{
		if(i%2==1)
			client_turn();
		else
		{
			sleep(1); //대전을 체감하기위한 1초의 딜레이
			server_turn();
		}

		game_print(i);
		for(j=0;j<4;j++) printf("turn[%d]=%d\n", j, turn[j]); //디버깅용
		if(turn[3]==1)
		{
			printf("클라이언트 승리\n");
			break;
		}
		else if(turn[3]==2)
		{
			printf("서버 승리\n");
			break;
		}
		else if(turn[3]==3)
		{
			printf("무승부\n");
			break;
		}
	}

}
