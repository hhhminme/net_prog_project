#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BOARD_SIZE 5

void socket_settings(char *ip, char *port); //소켓의 세팅
void error_check(int validation, char *message); //실행 오류 검사
int value_check(int number); //점수 유효값 검사
void game_init(); //빙고판 생성
void game_print(int number, int turn_count); //게임진행
void server_turn(); //서버 차례
void client_turn(int turn_count); //클라이언트 차례

int board[BOARD_SIZE][BOARD_SIZE]; //보드판 배열
int check_number[BOARD_SIZE*BOARD_SIZE+1]={0}; //중복검사용 배열
int socket_fd; //소켓 파일디스크립터
int turn[4]; //어플리케이션 프로토콜 정의
/*
	turn[0]=플레이어 숫자선택
	turn[1]=클라이언트 빙고 수
	turn[2]=서버 빙고 수
	turn[3]=게임종료여부(0=진행중, 1=클라이언트 승리, 2=서버 승리, 3=무승부)
*/

void main(int argc, char *argv[])
{
	int i, j;

	if(argc!=3)
	{
		printf("./실행파일 IP주소 PORT번호 형식으로 실행해야 합니다.\n");
		exit(1);
	}

	socket_settings(argv[1], argv[2]);

	printf("빙고게임을 시작합니다.\n");
	game_init();
	game_print(0, 0);
	
	for(i=1;i<BOARD_SIZE*BOARD_SIZE;i++)
	{
		if(i%2==1)
			client_turn(i);
		else
			server_turn();

		game_print(turn[0], i);
		//for(j=0;j<4;j++) printf("turn[%d]=%d\n", j, turn[j]); //디버깅용

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
	close(socket_fd);

	printf("빙고게임을 종료합니다\n");
}
void socket_settings(char *ip, char *port)
{
	struct sockaddr_in server_adr;

	socket_fd=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //TCP소켓 생성
	error_check(socket_fd, "소켓 생성");

	memset(&server_adr, 0, sizeof(server_adr)); //구조체 변수 값 초기화
	server_adr.sin_family=AF_INET; //IPv4
	server_adr.sin_port=htons(atoi(port)); //포트 할당
	server_adr.sin_addr.s_addr=inet_addr(ip); //IP 할당

	error_check(connect(socket_fd, (struct sockaddr *)&server_adr, sizeof(server_adr)), "연결 요청");
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
int value_check(int number) //점수 유효값 검사
{
	if(number<1||number>25||check_number[number]==1)
	{
		printf("값이 유효하지 않습니다. 다시입력해주세요 : ");
		scanf("%d", &number);
		number=value_check(number); //유효값 입력할때까지 재귀호출
	}
	return number;
}
void game_init()
{
	int i, j; //카운트용 변수
	int recv_len, recv_count;

	recv_len=0;
	while(recv_len!=sizeof(board)) // 패킷이 잘려서 올수도 있으므로 예외처리를 위한 조건문
	{
		recv_count=read(socket_fd, board, sizeof(board));
		error_check(recv_count, "데이터수신");
		if(recv_count==-0) break;
		printf("%d 바이트: board를 수신하였습니다\n", recv_count);
		recv_len+=recv_count;
	}
}
void game_print(int number, int turn_count)
{
	int i, j;

	system("clear"); //동적 효과를 위한 화면 초기화
	printf("%c[1;33m",27); //터미널 글자색을 노랑색으로 변경
	
	printf("@------ 클라 빙고판 ------@\n");
	printf("진행 턴수: %d\n", turn_count); 
	printf("+----+----+----+----+----+\n"); 
	for(i=0; i < BOARD_SIZE; i++)
	{
		for(j=0; j < BOARD_SIZE; j++)
		{
			if(board[i][j]==number)
				board[i][j]=0; //X표 처리
			if(board[i][j]==0)
			{
				printf("| ");
				printf("%c[1;31m",27);
				printf("%2c ", 88); 
				printf("%c[1;33m",27);
			}
			else
				printf("| %2d ", board[i][j]); 
		}
		printf("|\n");
		printf("+----+----+----+----+----+\n"); 
	}      
	printf("%c[0m", 27); //터미널 글자색을 원래색으로 변경
	if(turn_count!=0)
	{
		printf("숫자: %d\n", turn[0]);
		printf("빙고수: %d\n", turn[1]);
	}
}
void server_turn()
{
	int recv_len=0;

	while(recv_len!=sizeof(turn)) // 패킷이 잘려서 올수도 있으므로 예외처리를 위한 조건문
	{
		int recv_count;

		recv_count=read(socket_fd, turn, sizeof(turn));
		error_check(recv_count, "데이터수신");
		if(recv_count==0) break;
		printf("%d 바이트: 서버의 턴 정보를 수신하였습니다\n", recv_count);
		recv_len+=recv_count;
	}
	check_number[turn[0]]=1;
}
void client_turn(int turn_count)
{
	int array_len, recv_len=0;;

	printf("%d턴 숫자를 입력해주세요 : ", turn_count);
	scanf("%d", &turn[0]);
	turn[0]=value_check(turn[0]);
	check_number[turn[0]]=1;
	
	array_len=write(socket_fd, turn, sizeof(turn));
	printf("%d 바이트: 클라이언트의 턴 정보를 전송하였습니다\n", array_len);
	error_check(array_len, "데이터전송");

	while(recv_len!=sizeof(turn))
	{
		int recv_count;

		recv_count=read(socket_fd, turn, sizeof(turn));
		error_check(recv_count, "데이터수신");
		if(recv_count==0) break;
		printf("%d 바이트: 서버의 턴 정보를 수신하였습니다\n", recv_count);
		recv_len+=recv_count;
	}
}