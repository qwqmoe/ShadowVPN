/**
  shell.c

  Copyright (c) 2014 clowwindy

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <stdlib.h>
#include <stdio.h>
#include "shadowvpn.h"
#include "shell.h"

static int shell_run(shadowvpn_args_t *args, int is_up);
static int command_run(const char *command, size_t len);

int shell_up(shadowvpn_args_t *args) {
  return shell_run(args, 1);
}

int shell_down(shadowvpn_args_t *args) {
  return shell_run(args, 0);
}

int ifconfig(shadowvpn_args_t *args, int is_up) {
  char *buf;
  size_t buf_len = 128;
  int r;
#ifdef TARGET_WIN32
  const char *base = "netsh interface";
#else
  const char *base = "ifconfig";
#endif

  buf = malloc(buf_len);
  if (is_up) {
#if defined(TARGET_FREEBSD) || defined(TARGET_DARWIN)
    snprintf(buf, buf_len, "%s %s %s %s netmask %s mtu %d",
            base, args->intf, args->tun_local_ip, args->tun_remote_ip,
            args->tun_netmask, args->mtu);
#endif

#ifdef TARGET_LINUX
    snprintf(buf, buf_len, "%s %s %s dstaddr %s netmask %s mtu %d",
            base, args->intf, args->tun_local_ip, args->tun_remote_ip,
            args->tun_netmask, args->mtu);
#endif

#ifdef TARGET_WIN32
 /* TODO */
#endif
  }
  else {   /* ifconfig down */
#ifdef TARGET_WIN32
 /* TODO */
#else
    snprintf(buf, buf_len, "%s %s down", base, args->intf);
#endif
  }
  r =  command_run(buf, strlen(buf));
  free(buf);
  return r;
}

static int shell_run(shadowvpn_args_t *args, int is_up) {
  const char *script;
  if (is_up) {
    script = args->up_script;
  } else {
    script = args->down_script;
  }
  if (script == NULL || script[0] == 0) {
    errf("warning: script not set");
    return 0;
  }
  return command_run(script, strlen(script));
}

static int command_run(const char *command, size_t len) {
  char *buf;
  int r;
  size_t buf_len = len - (len % 4) + 16; // Optimized length, 4 * N
  buf = malloc(buf_len);
#ifdef TARGET_WIN32
  snprintf(buf, buf_len,  "cmd /c %s", command);
#else
  snprintf(buf, buf_len, "sh -c '%s'", command);
#endif
  logf("executing %s", command);
  if (0 != (r = system(buf))) {
    free(buf);
    errf("script %s returned non-zero return code: %d", command, r);
    return -1;
  }
  free(buf);
  return 0;
}
