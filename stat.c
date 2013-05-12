#include "def.h"

int main(int argc, char *argv[]) {
    struct command *cmd;
    int fd;
    
    cmd = create_cmd(TYPE_STAT, 0, getuid(), 0, argv);

    ERRORIF( (fd = open(CMD_FIFO, O_WRONLY)) < 0, "deq open fifo failed" );
    ERRORIF( write(fd, cmd, CMDSIZE) < 0, "deq write failed" );

    close(fd);
    return 0;
}
