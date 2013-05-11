#include "def.h"

int main() {
    int fifo;
    int count;
    struct command cmd;
    struct stat statbuf;
    if ( stat("buf", &statbuf) == 0) {
        if (remove("buf") < 0 ) 
            ERROR("remove failed");
    }

    if ( mkfifo("buf", 0666) < 0 )
        ERROR("mkfifo failed");
    DEBUG("after");
    
    if ( (fifo = open("buf", O_RDONLY | O_NONBLOCK)) < 0 )
        ERROR("open fifo failed");


    //printf("%s\n", *cmd.argv);

    close(fifo);
    return 0;
}
