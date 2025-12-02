#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv)
{
    // Need exactly one argument: pattern for grep
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pattern>\n", argv[0]);
        exit(1);
    }

    int pipe1[2];   // between cat and grep
    int pipe2[2];   // between grep and sort
    pid_t pid1, pid2;

    char *cat_args[]  = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};  // use argv[1] here
    char *sort_args[] = {"sort", NULL};

    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(1);
    }
    if (pipe(pipe2) == -1) {
        perror("pipe2");
        exit(1);
    }

    // First fork: create child that will handle grep (and fork sort)
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0) {
        /* ================= CHILD: grep + creates grandchild for sort ================ */
        pid2 = fork();
        if (pid2 < 0) {
            perror("fork");
            exit(1);
        }

        if (pid2 == 0) {
            /* ---------------- GRANDCHILD: executes "sort" ---------------- */
            // stdin from pipe2 read end
            dup2(pipe2[0], 0);

            // close all unused fds
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[1]);   // only reading pipe2
            close(pipe2[0]);   // original after dup2

            execvp("sort", sort_args);
            perror("execvp sort");
            exit(1);
        } else {
            /* ---------------- CHILD: executes "grep argv[1]" ------------- */
            // stdin from pipe1 read end
            dup2(pipe1[0], 0);
            // stdout to pipe2 write end
            dup2(pipe2[1], 1);

            // close unused ends
            close(pipe1[1]);
            close(pipe1[0]);   // original after dup2
            close(pipe2[0]);
            close(pipe2[1]);   // original after dup2

            execvp("grep", grep_args);
            perror("execvp grep");
            exit(1);
        }
    } else {
        /* ================= PARENT: executes "cat scores" ================= */
        // stdout to pipe1 write end
        dup2(pipe1[1], 1);

        // close unused ends
        close(pipe1[0]);
        close(pipe1[1]);   // original after dup2
        close(pipe2[0]);
        close(pipe2[1]);

        execvp("cat", cat_args);
        perror("execvp cat");
        exit(1);
    }
}