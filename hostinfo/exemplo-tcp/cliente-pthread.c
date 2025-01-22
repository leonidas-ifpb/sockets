#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curses.h>
#include <errno.h>
#include "sockets.h"

void erro(char *erromsg)
{
	fprintf(stderr, "%s: %s\n", erromsg, strerror(errno));
	exit(1);
}

void *recebe_msg(void *psockfd)
{
	char msgRec[MAX_MSG];
	int n_bytes;
	int sockfd = *(int *)psockfd;
	int c;

	WINDOW * win = newwin(10, 80, 0, 0);
	while ((n_bytes = recv(sockfd, msgRec, sizeof(msgRec), 0)) != 0) {
		if (n_bytes < 0) {
			fprintf(stderr, "Erro na recepcao: %s\n", strerror(errno));
			break;
		}
		msgRec[n_bytes] = '\0';
		wprintw(win, "Mensagem recebida: %s\n", msgRec);
		wrefresh(win);
	}
	printf("[Processo %d] Tarefas Concluidas...\n",  getpid());
	close(sockfd);
}

int main (int argc, char *argv[])
{
	int n_bytes;
	char msgServ[MAX_MSG];
	struct sockaddr_in serv_addr;
	pthread_t th;
	char *hostServer;
	int sockConexao;

	if (argc == 1) hostServer = "localhost";
	else hostServer = argv[1];
	puts("Cliente TCP do servico de Echo...");
	if ((sockConexao = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		erro("Erro no socket");
	serv_addr.sin_family = AF_INET;
	if (inet_aton(hostServer, &serv_addr.sin_addr) == 0) {
		struct hostent *he;
		if ((he = gethostbyname(hostServer)) == NULL) {
			fprintf(stderr, "%s: Host nao acessivel\n", hostServer);
			exit(1);
		}
		serv_addr.sin_addr = *(struct in_addr *)he->h_addr;
	}
	serv_addr.sin_port = htons(SERVER_TCP_PORT);
	puts("Estabelecendo a ligacao ...");
	if (connect(sockConexao, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		erro("Erro em connect");
	puts("Ligacao estabelecida com o servidor...");
	initscr();
	if (pthread_create(&th, NULL, recebe_msg, &sockConexao) == 0) {
		WINDOW * win = newwin(10, 80, 15, 0);
		for(;;) {
			wprintw(win, "Mensagem (CTRL-C termina): ");
			if (wgetnstr(win, msgServ, sizeof(msgServ) - 1) == ERR) break;
			wprintw(win, "Enviando mensagem \"%s\"\n", msgServ);
			wrefresh(win);
			if (send(sockConexao, msgServ, strlen(msgServ), 0) < 0)
				erro("Erro no envio");
		} 
		puts("\nCliente terminou\n");
		pthread_cancel(th);
	} else {
		fprintf(stderr, "Erro na criação da thread de recepção\n");
	}
	close(sockConexao);
}
