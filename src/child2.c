#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 4096

int main(void) {
    char buff[BUFF_SIZE];
    ssize_t bytes_read = 0;

    while ((bytes_read = read(STDIN_FILENO, buff, BUFF_SIZE))) {
        if (bytes_read == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read error");
            exit(EXIT_FAILURE);
        }

        ssize_t i;
        for (i = 0; i < bytes_read; ++i) {
            if (isspace((unsigned char)buff[i])) {
                buff[i] = '_';
            }
        }

        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            ssize_t bytes_written = write(STDOUT_FILENO, buff + total_written, bytes_read - total_written);
            if (bytes_written == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("write error");
                exit(EXIT_FAILURE);
            }

            total_written += bytes_written;
        }
    }

    return EXIT_SUCCESS;
}
