// C program to demonstrate use of fork() and two-way pipe
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main(void)
{
    // fd1: parent -> child
    // fd2: child  -> parent
    int fd1[2], fd2[2];

    char fixed_str[]    = "howard.edu";
    char parent_suffix[] = "gobison.org";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe");
        exit(1);
    }

    printf("Enter a string to concatenate: ");
    scanf("%99s", input_str);

    p = fork();
    if (p < 0) {
        perror("fork");
        exit(1);
    }

    /* ================== PARENT ================== */
    if (p > 0) {
        char child_result[200];

        // parent writes to fd1[1], reads from fd2[0]
        close(fd1[0]);   // unused read end
        close(fd2[1]);   // unused write end

        // send first input to child
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // receive child's concatenated result
        read(fd2[0], child_result, sizeof(child_result));
        close(fd2[0]);

        // parent adds final suffix "gobison.org"
        strcat(child_result, parent_suffix);

        // REQUIRED final output: [input][howard.edu][child input][gobison.org]
        printf("%s\n", child_result);

        wait(NULL);
    }

    /* ================== CHILD =================== */
    else {
        char concat_str[200];
        char second_input[100];

        // child reads from fd1[0], writes to fd2[1]
        close(fd1[1]);   // unused write end
        close(fd2[0]);   // unused read end

        // read initial string from parent
        read(fd1[0], concat_str, sizeof(concat_str));
        close(fd1[0]);

        // first concat with "howard.edu" (no print here – rubric said not to)
        strcat(concat_str, fixed_str);

        // ask user for second string in child
        printf("Enter another string to concatenate: ");
        scanf("%99s", second_input);

        // second concat with child's input
        strcat(concat_str, second_input);

        // (Optional but usually OK) child prints its final version
        printf("%s\n", concat_str);

        // send final child string back to parent
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}
