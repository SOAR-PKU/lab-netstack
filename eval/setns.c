//
// Created by Chengke Wong on 2019/11/8.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>

/****** hijack at linking time so that debugging becomes easy!  ********/

// Solution 1: no guarantee that it works
// reference: https://stackoverflow.com/questions/56678458
// __attribute__((constructor)) void __setns_main(int argc, char *argv[])
// NASTY HACK !!!!!!!!! no guarantee that it receive argc and argv

/**
 * https://code.woboq.org/gcc/libgcc/crtstuff.c.html
665	static void __attribute__((used))
666	__do_global_ctors_aux (void)
667	{
668	  func_ptr *p;
669	  for (p = __CTOR_END__ - 1; *p != (func_ptr) -1; p--)
670	    (*p) ();
671	}
 */

// Solution 2: see the code below
// https://sourceware.org/ml/libc-help/2009-11/msg00006.html
// https://stackoverflow.com/questions/34915875/find-argc-and-argv-from-a-library
// http://dbp-consulting.com/tutorials/debugging/linuxProgramStartup.html

// https://code.woboq.org/userspace/glibc/csu/elf-init.c.html
/**
void
__libc_csu_init (int argc, char **argv, char **envp)
{

  _init ();

  const size_t size = __init_array_end - __init_array_start;
  for (size_t i = 0; i < size; i++)
      (*__init_array_start [i]) (argc, argv, envp);
}
 */

static int starts_with(const char *pre, const char *str) {
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return lenstr < lenpre ? 0 : memcmp(pre, str, lenpre) == 0;
}

static void wrerr(const char *p) {
  if (p != NULL) {
    const char *const q = p + strlen(p);
    ssize_t n;

    while (p < q) {
      n = write(STDERR_FILENO, p, (size_t)(q - p));
      if (n > (ssize_t)0)
        p += n;
      else if (n != (ssize_t)-1)
        return;
      else if (errno != EINTR)
        return;
    }
  }
}

int __setns_main(int argc, char *argv[], char *env[]) {
  int *argc_loc = (int *)argv - 2;
  errno = 0;

  if (argc != *argc_loc) {
    goto fail;
  }

  if (argc < 2) {
    goto fail;
  }

  if (!starts_with("ns", argv[1])) {
    goto fail;
  }

  char buf[64 + 32];
  static const char *ns_path = "/var/run/netns/";
  memcpy(buf, ns_path, strlen(ns_path) + 1);
  strncat(buf, argv[1], 64);
  int fd = open(buf, O_RDONLY);
  if (fd == -1) {
    goto fail;
  }
  /* Join that namespace */
  if (setns(fd, CLONE_NEWNET) == -1) {
    goto fail;
  }

  fprintf(stderr,
          "\033[0;31m"
          "network namespace hijack succeeds! (with %s)\n"
          "\033[0m",
          argv[1]);

  /* remove the first argument */
  for (int i = 1; i + 1 < argc; i++) {
    argv[i] = argv[i + 1];
  }

  *argc_loc = argc - 1;
  return 0;

fail:
  fprintf(stderr, "\033[0;31m");
  if (errno != 0) {
    fprintf(stderr, "setns failed %s\n", strerror(errno));
  }
  fprintf(stderr, "fall back to default network setting\n");
  fprintf(stderr, "\033[0m");
  return 0;
}

__attribute__((section(".preinit_array"))) static void *foo_constructor =
    &__setns_main;
