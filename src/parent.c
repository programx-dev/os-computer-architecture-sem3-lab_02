#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFF_SIZE 4096

#define INPUT_ERROR_EXIT_CODE 2
#define OUTPUT_ERROR_EXIT_CODE 3

int write_all(int fd, const char *buf, ssize_t count) {
    ssize_t total_written = 0;
    while (total_written < count) {
        ssize_t bytes_written = write(fd, buf + total_written, count - total_written);
        if (bytes_written == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        total_written += bytes_written;
    }
    return 0;
}

int main(void) {
    int pipe1[2];
    int pipe_inner[2];
    int pipe2[2];

    if (pipe(pipe1) == -1) {
        perror("parent: pipe1");
        return EXIT_FAILURE;
    }

    if (pipe(pipe_inner) == -1) {
        perror("parent: pipe_inner");
        close(pipe1[0]);
        close(pipe1[1]);
        return EXIT_FAILURE;
    }

    if (pipe(pipe2) == -1) {
        perror("parent: pipe2");
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe_inner[0]);
        close(pipe_inner[1]);
        return EXIT_FAILURE;
    }

    pid_t pid_child1 = fork();

    if (pid_child1 == -1) {
        perror("parent: fork");
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe_inner[0]);
        close(pipe_inner[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        return EXIT_FAILURE;
    }

    if (pid_child1 == 0) {
        close(pipe1[1]);
        close(pipe_inner[0]);
        close(pipe2[0]);
        close(pipe2[1]);

        if (dup2(pipe1[0], STDIN_FILENO) == -1) {
            perror("child1: dup2");
            exit(EXIT_FAILURE);
        }
        if (dup2(pipe_inner[1], STDOUT_FILENO) == -1) {
            perror("child1: dup2");
            exit(EXIT_FAILURE);
        }

        close(pipe1[0]);
        close(pipe_inner[1]);

        execl("./child1", "child1", NULL);

        perror("child1: execl");
        exit(EXIT_FAILURE);
    }

    pid_t pid_child2 = fork();

    if (pid_child2 == -1) {
        perror("parent: fork");
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe_inner[0]);
        close(pipe_inner[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        kill(pid_child1, SIGTERM);
        while (waitpid(pid_child1, NULL, 0) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("parent: waitpid");
            break;
        }
        return EXIT_FAILURE;
    }

    if (pid_child2 == 0) {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe_inner[1]);
        close(pipe2[0]);

        if (dup2(pipe_inner[0], STDIN_FILENO) == -1) {
            perror("child2: dup2");
            exit(EXIT_FAILURE);
        }
        if (dup2(pipe2[1], STDOUT_FILENO) == -1) {
            perror("child2: dup2");
            exit(EXIT_FAILURE);
        }

        close(pipe_inner[0]);
        close(pipe2[1]);

        execl("./child2", "child2", NULL);

        perror("child2: execl");
        exit(EXIT_FAILURE);
    }

    close(pipe1[0]);
    close(pipe_inner[0]);
    close(pipe_inner[1]);
    close(pipe2[1]);

    char buff[BUFF_SIZE];
    char buff_out[BUFF_SIZE];
    ssize_t bytes_read = 0;
    ssize_t bytes_read_out = 0;
    int parent_error = EXIT_SUCCESS;

    while ((bytes_read = read(STDIN_FILENO, buff, BUFF_SIZE)) != 0) {
        if (bytes_read == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("parent: read");
            parent_error = INPUT_ERROR_EXIT_CODE;
            break;
        }

        if (write_all(pipe1[1], buff, bytes_read) == -1) {
            perror("parent: write");
            parent_error = OUTPUT_ERROR_EXIT_CODE;
            break;
        }

        while ((bytes_read_out = read(pipe2[0], buff_out, BUFF_SIZE)) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("parent: read");
            parent_error = INPUT_ERROR_EXIT_CODE;
            goto BREAK_WHILE;
        }

        if (bytes_read_out > 0) {
            if (write_all(STDOUT_FILENO, buff_out, bytes_read_out) == -1) {
                perror("parent: write");
                parent_error = OUTPUT_ERROR_EXIT_CODE;
                break;
            }
        }
    }

BREAK_WHILE:

    close(pipe1[1]);

    if (parent_error == EXIT_SUCCESS) {
        while ((bytes_read_out = read(pipe2[0], buff_out, BUFF_SIZE)) != 0) {
            if (bytes_read_out == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("parent: read");
                parent_error = INPUT_ERROR_EXIT_CODE;
                break;
            }

            if (write_all(STDOUT_FILENO, buff_out, bytes_read_out) == -1) {
                perror("parent: write");
                parent_error = OUTPUT_ERROR_EXIT_CODE;
                break;
            }
        }
    }

    close(pipe2[0]);

    if (parent_error != EXIT_SUCCESS) {
        kill(pid_child1, SIGTERM);
        kill(pid_child2, SIGTERM);
    }

    int status1 = 0;
    int status2 = 0;
    pid_t res_wait1;
    pid_t res_wait2;

    while ((res_wait1 = waitpid(pid_child1, &status1, 0)) == -1) {
        if (errno == EINTR) {
            continue;
        }
        perror("parent: waitpid");
        break;
    }
    if (res_wait1 > 0) {
        if (WIFEXITED(status1) && WEXITSTATUS(status1) != 0) {
            fprintf(stderr, "parent: child1 exited with status: %d\n", WEXITSTATUS(status1));
        } else if (WIFSIGNALED(status1)) {
            fprintf(stderr, "parent: child1 killed by signal: %d\n", WTERMSIG(status1));
        }
    }

    while ((res_wait2 = waitpid(pid_child2, &status2, 0)) == -1) {
        if (errno == EINTR) {
            continue;
        }
        perror("parent: waitpid");
        break;
    }
    if (res_wait2 > 0) {
        if (WIFEXITED(status2) && WEXITSTATUS(status2) != 0) {
            fprintf(stderr, "parent: child2 exited with status: %d\n", WEXITSTATUS(status2));
        } else if (WIFSIGNALED(status2)) {
            fprintf(stderr, "parent: child2 killed by signal: %d\n", WTERMSIG(status2));
        }
    }

    return parent_error;
}
