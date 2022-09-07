//
// Created by Chengke Wong on 2019/10/22.
//

#include "unp.h"

void str_cli(FILE *fp, int sockfd) {
  char sendline[MAXLINE];
  char recvline[MAXLINE];
  while (fgets(sendline, MAXLINE, fp) != NULL) {
    writen(sockfd, sendline, strlen(sendline));

    if (readline(sockfd, recvline, MAXLINE) == 0) {
      printf("str_cli: server terminated prematurely\n");
      exit(1);
    }

    fputs(recvline, stdout);
  }
}
void __setns_main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in servaddr;
  if (argc != 2) {
    printf("usage: echo_client <IPaddress>\n");
    return -1;
  }

  sockfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(10086);
  Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  str_cli(stdin, sockfd);
}