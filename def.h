#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

//#define DEBUG
#define DEBUG(msg)          printf("%s\n", msg);
#define ERROR(msg)          printf("%s\n", msg); exit(1);

#define STATE_READY         1
#define STATE_RUNNING       2
#define STATE_DONE          4
#define TYPE_ENQ            8
#define TYPE_DEQ            16
#define TYPE_STAT           32

struct command {
    int     info;
    int     argc;
    int     owner;
    int     pri;
    char    **argv;
};
#define CMDSIZE             sizeof(struct command)


typedef int                 jid_t;
struct jobinfo {
    jid_t   jid;
    pid_t   pid;
    char**  argv;
    int     default_pri;
    int     current_pri;
    int     owner;
    int     wait_time;
    time_t  create_time;
    int     run_time;
    int     state;
};
typedef struct jobinfo      *jobinfo;

int job_compare( jobinfo, jobinfo );

#define JOB_COMP_FUN       (int (*)(void*, void*))job_compare


#endif
