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

void error_handling(char* mse); //실행 오류 검사
void game_print(int any);

int turn_count = 0;
int board[BOARD_SIZE][BOARD_SIZE];


int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread, gam_thread;
	void* thread_return;
	
	
	
	//게임보드 메이크

	srand(time(NULL)); //시간을 인자로 주어서 완전한 랜덤을 구현
	int check_number[BOARD_SIZE * BOARD_SIZE] = { 0 }; //중복제거용 배열
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			while (1)
			{
				int temp = rand() % 25; //0~24 난수 발생

				if (check_number[temp] == 0) //중복제거 알고리즘. 0~24의 값을 받아오고, 사용한 값은 중복제거용 배열에 표시해놓는다.
				{
					check_number[temp] = 1;
					board[i][j] = temp + 1;
					break;
				}
			}
		}
	}



	if (argc != 3) {
		printf("ip, 포트번호 순으로 입력");
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
	char msg[BUF_SIZE];
	while (1) {
		fgets(msg, BUF_SIZE, stdin);
		if (!strcmp(msg, "q\n")||!strcmp(msg,"Q\n"))
		{
			close(sock);
			exit(0);
		}
		sprintf(msg, "%s", msg);
		write(sock, msg, strlen(msg));
	}
	return NULL;
}
void* recv_msg(void* arg) {
	int sock = *((int*)arg);
	char msg[BUF_SIZE];
	int str_len;
	while (1) {
		str_len = read(sock, msg, BUF_SIZE - 1);
		if (str_len == -1)
			return (void*)-1;
		msg[str_len] = 0;
		fputs(msg, stdout);
		game_print(0);
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
	int i, j;

	system("clear"); //동적 효과를 위한 화면 초기화
	printf("%c[1;33m", 27); //터미널 글자색을 노랑색으로 변경

	printf("@------ 클라 빙고판 ------@\n");
	printf("진행 턴수: %d\n", turn_count++);
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
	printf("%c[0m", 27); //터미널 글자색을 원래색으로 변경
	if (turn_count != 0)
	{
		printf("숫자: %d\n", 1);
		printf("빙고수: %d\n", 1);
	}
}