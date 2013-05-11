#include <string.h>

#include "def.h"

void help() {
    printf("Usage: enq [-p num] executable [arg [arg ...]]\n \
             -p num\t\t specify the job priority\n \
             executable\t\tthe absolute path of the executable\n \
             args\t\t arguments\n"
          );
}

struct command *create_cmd( int type, int pri, int uid, int argc, char *argv[] ) {
    struct command *cmd = malloc(sizeof(struct command));
    cmd->info = type;
    cmd->pri = pri;
    cmd->owner = uid;
    cmd->argc = argc;
    cmd->argv = argv;
    return cmd;
}

int main(int argc, char *argv[]) {
    int buf;
    int pri = 0;
    struct command *cmd; 
    if (argc == 1) {
        help();
        return 1;
    }
    argc--;
    argv++;

    if (strcmp( *argv, "-p" ) == 0) {
        pri = atoi( argv[1] );
        argc -= 2;
        argv += 2;
    }

    if ( pri < 0 || pri > 3 ) {
        printf("invalid priority: must between 0 and 3 \n");
        return 1;
    }
    
    cmd = create_cmd( TYPE_ENQ, pri, getuid(), argc, argv ); 

    if ( (buf = open("buf", O_WRONLY )) < 0 ) 
        ERROR("enq open fifo failed");
    if ( write(buf, cmd, CMDSIZE) < 0 )
        ERROR("enq write failed");
    close(buf);

    return 0;
}
