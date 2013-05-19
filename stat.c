#include "def.h"

void print_job(jobinfo job) {
    char time[BUFLEN];
    strcpy(time, ctime(&(job->create_time)));
    time[strlen(time) - 1] = '\0';
    printf("%d\t%d\t%d\t%d\t%d\t%s\t%s\n", 
            job->jid,
            job->pid,
            job->owner,
            job->run_time,
            job->wait_time, 
            time,
            job->state & STATE_RUNNING ? "RUNNING" : "READY"
          );
}

int main(int argc, char *argv[]) {
    struct command *cmd;
    struct jobinfo job;
    int fd, fifo, len;
    
    cmd = create_cmd(TYPE_STAT, 0, getuid(), 0, argv);

    ERRORIF( (fd = open(CMD_FIFO, O_WRONLY)) < 0, "deq open fifo failed" );
    ERRORIF( write(fd, cmd, CMDSIZE) < 0, "deq write failed" );

    ERRORIF( (fifo = open(STAT_FIFO, O_RDONLY)) < 0, "open fifo failed");

    printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITTIME\tCREATTIME\t\tSTATE\n");
    while ( (len = read(fifo, &job, sizeof(struct jobinfo))) != 0) {
        printf("return %d\n", len);
        print_job(&job);
    }

    close(fifo);
    close(fd);
    return 0;
}
