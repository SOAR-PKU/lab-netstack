//
// Created by Chengke Wong on 2019-09-05.
//

#ifndef TCPSTACK_ASSERT_H
#define TCPSTACK_ASSERT_H

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void backtrace();

// #define static_assert(x) switch (x) case 0: case (x):
#define DCHECK(x)                                                              \
  if (!(x))                                                                    \
  backtrace()

static void backtrace() {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}

#endif // TCPSTACK_ASSERT_H
