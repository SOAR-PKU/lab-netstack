//
// Created by Chengke Wong on 2019/11/8.
//

#include "unp.h"
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // activate tcp stack
  getaddrinfo(NULL, NULL, NULL, NULL);

  while (1) {
    sleep(3600);
  }

  return 0;
}