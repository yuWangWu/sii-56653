#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>

int main(int argc, char **argv) {
    int fd;
    char buffer[1024];
    int readLength;

    // borrar FIFO por si existiera previamente
    unlink("/tmp/tenisFIFO");

    /* crear el FIFO */
    if (mkfifo("/tmp/tenisFIFO", 0666) < 0) {
        perror("No puede crearse el FIFO");
        return(1);
    }

    /* Abrir el FIFO */
    if ((fd = open("/tmp/tenisFIFO", O_RDONLY)) < 0) {
        perror("No puede abrirse el FIFO");
        return(1);
    }

    /* Leer de la FIFO */
    while ((readLength = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[readLength] = '\0';
        std::cout << buffer << std::flush;
    }

    close(fd);
    unlink("/tmp/tenisFIFO");
    return(0);
}