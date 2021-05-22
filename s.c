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
#define BUF_SIZE 100
#define BACKLOG 3 
#define MAX_CLNT 256

void error_handling(char* mse);
void* handle_clnt(void* arg);
void send_msg(char* msg, int len);

int turn[4]; 
int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
int msgQ[5][BUF_SIZE];
pthread_mutex_t mutx;
pthread_t t_id;

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
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
	printf("var reset, ip port allocate, listen");
	while (1) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected Clnt IP : %s \n", inet_ntoa(clnt_adr.sin_addr));

		printf("* %s:%d connect\n", inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));
		printf(".\n");
	}
	close(serv_sock);
	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* handle_clnt(void* arg) {
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];
	//handle_clnt의 메세지 수신부분
	
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) //eliminated disconnections
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void send_msg(char* msg, int len) {
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
