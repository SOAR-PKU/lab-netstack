//
// Created by Chengke Wong on 2019/10/22.
//

#include "unp.h"

void str_echo(int sockfd) {
  ssize_t n;
  char buf[MAXLINE];
  size_t acc = 0;
again:
  while ((n = read(sockfd, buf, MAXLINE)) > 0) {
    writen(sockfd, buf, n);
    acc += n;
    printf("%zu\n", acc);
  }
  printf("===== %zu\n", acc);
  if (n < 0 && errno == EINTR) {
    goto again;
  } else if (n < 0) {
    printf("str_echo: read error\n");
  }
  close(sockfd);
}

int main(int argc, char *argv[]) {
  struct sockaddr_in cliaddr, servaddr;
  int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(10086);

  Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  int optval = 1;
  int rv = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                      sizeof(int));
  if (rv < 0) {
    printf("%s\n", strerror(errno));
  }
  Listen(listenfd, SOMAXCONN);
  for (;;) {
    socklen_t clilen = sizeof(cliaddr);
    int connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    printf("new connection\n");
    //    int childpid = Fork();
    //    if (childpid == 0) {
    //      close(listenfd);
    str_echo(connfd);
    //      exit(0);
    //    }
    //    close(connfd);
  }
}