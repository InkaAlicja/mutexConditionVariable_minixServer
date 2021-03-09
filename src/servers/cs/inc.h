#ifndef CONDITIONVARIABLESSERVER_INKAALICJA_INC_H
#define CONDITIONVARIABLESSERVER_INKAALICJA_INC_H

#define _POSIX_SOURCE      1    /* tell headers to include POSIX stuff */
#define _MINIX             1    /* tell headers to include MINIX stuff */
#define _SYSTEM            1    /* get OK and negative error codes */

#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/ipc.h>
#include <minix/endpoint.h>
#include <minix/sysutil.h>
#include <minix/const.h>
#include <minix/type.h>
#include <minix/syslib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <machine/vm.h>
#include <machine/vmparam.h>
#include <sys/vm.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int do_lock(message *, endpoint_t);
int do_unlock(message *, endpoint_t);
int do_wait(message *, endpoint_t);
int do_broadcast(message *, endpoint_t);
int get_mutex_own(int);

EXTERN int identifier;
EXTERN endpoint_t who_e;
EXTERN int call_type;
EXTERN endpoint_t SELF_E;

#endif //CONDITIONVARIABLESSERVER_INKAALICJA_INC_H
