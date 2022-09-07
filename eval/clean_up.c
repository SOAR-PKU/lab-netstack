//
// Created by Chengke Wong on 2019/11/9.
//

// TODO: consider moving this file to a suitable directory

#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include "../src/posix/posix_socket.h"

void __attribute__((destructor)) __clean_up() {
  fprintf(stderr, "\033[0;31m"
                  "cleaning up...\n"
                  "\033[0m");
  __destory_protocol_stack();

  // TODO: check whether all sockets are in CLOSED state
  while (!__can_close_protocol_stack()) {
    fprintf(stderr, "\033[0;31m"
                    "waiting until all sockets are CLOSED\n"
                    "\033[0m");
    sleep(10);
  }

  fprintf(stderr, "\033[0;31m"
                  "done! exit now\n"
                  "\033[0m");
}