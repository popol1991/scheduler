#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>

#include "def.h"
#include "heap.h"

int fifo;
jid_t jid_rm, job_id = 0;
int siginfo = 1;
jobinfo current = NULL;
heap *wait_q = NULL;
int child_start;

void sig_handler(int, siginfo_t*, void*);
void schedule(void);
void job_switch(void);
void update_joblist(void);
void enqueue(struct command*);
void dequeue(struct command*);
void show_stat(void);

void toggle_child_start() {
    child_start = 1;
}

int main() {
    struct stat statbuf;
    struct timeval interval;
    struct itimerval new, old;
    struct sigaction newact, oldact1, oldact2;

    // make and open fifo
    if ( stat(CMD_FIFO, &statbuf) == 0) 
        ERRORIF( remove(CMD_FIFO) < 0, "remove failed" );
    ERRORIF( mkfifo(CMD_FIFO, 0666) < 0, "mkfifo failed" );
    ERRORIF( (fifo = open(CMD_FIFO, O_RDONLY | O_NONBLOCK)) < 0, "open fifo failed" );

    // register signals
    newact.sa_sigaction = sig_handler;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &newact, &oldact1);
    sigaction(SIGVTALRM, &newact, &oldact2);

    // set time interval
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    new.it_interval = interval;
    new.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &new, &old);
    
    wait_q = create_heap( JOB_COMP_FUN, JOB_DESTRUCT_FUN );

    while (siginfo == 1) ;

    close(fifo);
    return 0;
}

void sig_handler(int sig, siginfo_t *info, void *oldact) {
    int status, ret; 
    DEBUG("get signal");
    switch (sig) {
        case SIGVTALRM:
            DEBUG("time signal");
            schedule();
            break;
        case SIGCHLD:
            ret = waitpid(-1, &status, WNOHANG);
            if (ret == 0)
                return;
            if (WIFEXITED(status)) {
                current->state = STATE_DONE;
                printf( "normal termination, exit status = %d\n", WEXITSTATUS(status) );
            } else if (WIFSIGNALED(status)) {
                printf( "abnormal termination, signal number = %d\n", WTERMSIG(status) );
            } else if (WIFSTOPPED(status)) {
                printf( "child stopped, signal number = %d\n", WSTOPSIG(status) );
            }
            break;
        default:
            break;
    }
}

void schedule() {
    jobinfo newjob = NULL;
    struct command cmd;
    int len = 0;
    memset(&cmd, 0, CMDSIZE);

    ERRORIF( (len = read(fifo, &cmd, CMDSIZE)) < 0, "read fifo failed");
    DEBUG("updating");
    // update job list
    update_joblist();
    DEBUG("update finished");
#ifdef DEBUG_INFO 
    if (len != 0) {
        printf("cmd type: %d\n", cmd.info);
    }
#endif
    if (cmd.info & TYPE_ENQ) {
        DEBUG("enqueue");
        enqueue( &cmd );
    } else if (cmd.info & TYPE_DEQ) {
        dequeue( &cmd );
    } else if (cmd.info & TYPE_STAT) {
        show_stat();
    }

    job_switch();
}

void update( jobinfo job ) {
    job->wait_time += 1000;
    if (job->wait_time >= 5000 && job->current_pri < 3) {
        job->current_pri += 1;
        job->wait_time = 0;
    }
}

void update_joblist() {
   if (current)
       current->run_time += 1;

   traverse(wait_q, (void (*)(void*))update);
}

void job_switch() {
    jobinfo next = NULL;
    int i;
    
    if (current && current->state & STATE_DONE) {
        for (i = 0; current->argv[i] != NULL; i++) {
            free( current->argv[i]);
        }
        free(current->argv);
        free(current);
        current = NULL;
    }

    if (current == NULL)
        DEBUG("current is NULL");
    if ( size(wait_q) > 0 && (current == NULL || current->current_pri < ((jobinfo) peek(wait_q))->current_pri) ) {
        next = (jobinfo) pop(wait_q);
    } else {
        return; // no need to switch
    }

    if (current != NULL) {
        printf("switch to Pid: %d\n", next->pid);
        kill(current->pid, SIGSTOP);
        current->current_pri = current->default_pri;
        current->wait_time = 0;
        current->state = STATE_READY;
        insert(wait_q, current);
    } else {
        printf("begin start new job\n");
    }

    current = next;
    current->state = STATE_RUNNING;
    printf("send SIGGONT to pid: %d\n", current->pid);
    kill(current->pid, SIGCONT);

}

jobinfo cmd2job(struct command *cmd) {
    int i;
    char *arg, *p, *tmp;
    jobinfo job = malloc(sizeof(struct jobinfo));
    job->jid = job_id++;
    job->default_pri = cmd->pri;
    job->current_pri = job->default_pri;
    job->owner = cmd->owner;
    job->state = STATE_READY;
    job->create_time = time(NULL);
    job->wait_time = 0;
    job->run_time = 0;
    job->argv = malloc(sizeof(char*) * (cmd->argc + 1));
    
    arg = cmd->buf;
    p = cmd->buf;
    i = 0;
    while (i < cmd->argc) {
        if (*p == ':') {
            *p++ = '\0';
            tmp = malloc(p - arg);
            strcpy(tmp, arg);
            job->argv[i++] = tmp;
            arg = p;
        } else {
            p++;
        }
    }
    job->argv[i] = NULL;
    return job;        
}

void enqueue(struct command *cmd) {
    pid_t pid;
    
    DEBUG("convert cmd 2 job");
    jobinfo job = cmd2job( cmd );
    DEBUG("conversion finished, insert into wait_q");
    insert(wait_q, job);
    DEBUG("insertion finished");

    child_start = 0;
    signal(SIGUSR1, toggle_child_start);

    ERRORIF( (pid = fork()) < 0, "enq fork failed" );
    if (pid == 0) {
        DEBUG("child process begin");
        kill(getppid(), SIGUSR1);
        raise(SIGSTOP);
        DEBUG("child process continue");
        if ( execvp(job->argv[0], job->argv) == -1 ) {
            printf("exec failed");
            printf("errno: %d\n", errno);
        }
        exit(1);
    } else {
        while (!child_start) ;
        job->pid = pid;
        printf("pid: %d\n", job->pid);
    }
    DEBUG("fork finished");
}

int to_remove( jobinfo job ) {
    if (job->jid == jid_rm)
        return 1;
    else
        return 0;
}

void dequeue(struct command *cmd) {
    jid_rm = atoi(cmd->buf);

    if (current && current->jid == jid_rm) {
        printf("terminate current job\n");
        kill(current->pid, SIGKILL);
        job_destruct( current );
    } else {
        remove_if(wait_q, (int (*)(void *))to_remove );
    }
}

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

void show_stat() {
    printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITTIME\tCREATTIME\t\tSTATE\n");
    if (current) 
        print_job(current);
    traverse(wait_q, (void (*)(void*))print_job);
}
