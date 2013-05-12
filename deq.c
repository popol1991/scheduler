#include "def.h"

void help() {
    printf("Usage: deq jid\n \
            \tjid\t\tthe job id\n");
}

int main(int argc, char *argv[]) {
    struct command* cmd;
    int fd;
    if (argc != 2) {
        help();
        return 1;
    }

    cmd = create_cmd(TYPE_DEQ, 0, getuid(), 1, ++argv);
    printf("jid %s\n", cmd->buf);

    ERRORIF( (fd = open(CMD_FIFO, O_WRONLY)) < 0, "deq open fifo failed" );
    ERRORIF( write(fd, cmd, CMDSIZE) < 0, "deq write failed" );

    close(fd);
    return 0;
}
