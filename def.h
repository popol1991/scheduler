#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DEBUG(msg)          printf("%s\n", msg);
#define ERRORIF(exp, msg)   if (exp) { printf("%s\n", msg); exit(1); }


#define CMD_FIFO            "buf"
#define STAT_FIFO           "statbuf"

#define STATE_READY         1
#define STATE_RUNNING       2
#define STATE_DONE          4
#define TYPE_ENQ            8
#define TYPE_DEQ            16
#define TYPE_STAT           32

#define BUFLEN              100

struct command {
    int     info;
    int     state;
    int     argc;
    int     owner;
    int     pri;
    char    buf[BUFLEN];
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

int job_compare( jobinfo j1, jobinfo j2) {
    if (j1->current_pri > j2->current_pri)
        return -1;
    else
        return 1;
}
void job_destruct( jobinfo job ) {
    int i;
    for (i = 0; job->argv[i] != NULL; i++) {
        free( job->argv[i] );
        job->argv[i] = NULL;
    }
    free(job->argv);
    free(job);
}
#define JOB_COMP_FUN        (int (*)(void*, void*))job_compare
#define JOB_DESTRUCT_FUN    (void (*)(void*))job_destruct

struct command *create_cmd( int type, int pri, int uid, int argc, char *argv[] ) {
    char **p;
    struct command *cmd = malloc(sizeof(struct command));
    cmd->info = type;
    cmd->pri = pri;
    cmd->owner = uid;
    cmd->argc = argc;
    strcat(cmd->buf, *argv++);
    argc--;
    while (argc-- > 0) {
        strcat(cmd->buf, ":");
        strcat(cmd->buf, *argv++);
    }
    return cmd;
}

#endif
