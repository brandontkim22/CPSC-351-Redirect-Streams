#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// adds NULL at the end of cmd
void copy_ptrs_from_to(char** to, char** from, int from_ix, int to_ix)
{
    for (int dest_ix = 0; dest_ix < to_ix - from_ix + 1; dest_ix++)
        to[dest_ix] = from[from_ix + dest_ix];
    to[to_ix - from_ix + 1] = NULL;
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        fprintf(stderr,
            "Usage: %s <command1> [<arg1> <arg2> ...] /// <command2> [<arg1> "
            "<arg2> ...] \n",
            argv[0]);
        return 1;
    }

    // Create input and output file descriptors
    int fd_in;
    if (strcmp(argv[1], "-") == 0) {
        fd_in = STDIN_FILENO;
    } else {
        fd_in = open(argv[1], O_RDONLY);
        if (fd_in == -1) {
            fprintf(stderr, "Failed to open %s\n", argv[1]);
            return 1;
        }
    }

    int fd_out;
    if (strcmp(argv[argc-1], "-") == 0) {
        fd_out = STDOUT_FILENO;
    } else {
        fd_out = open(argv[argc-1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd_out == -1) {
            fprintf(stderr, "Failed to open %s\n", argv[3]);
            return 1;
        }
    }

    // +1 for NULL
    char** first_cmd = (char**)malloc(sizeof(char*) * argc + 1);
 
    // SKIP FIRST TWO ARGS (./this <inputFilename>)
    // Then read until output filename
    copy_ptrs_from_to(first_cmd,  argv, 2, argc - 2);

    // check if the command is an absolute path
    if (access(first_cmd[0], X_OK) != 0) {
        fprintf(stderr, "%s is not an ABSOLUTE PATH!\n", first_cmd[0]);
        for (char* path = strtok(getenv("PATH"), ":"); path != NULL;
                   path = strtok(NULL, ":")) {
            printf("PATH: %s\n", path);
            
            char new_path[64];
            new_path[0] = '\0';
            strcat(new_path, path);
            strcat(new_path, "/");
            strcat(new_path, first_cmd[0]);
            
            printf("Testing PATH: %s\n", new_path);
            if (access(new_path, X_OK) == 0) {
                printf("Found executable in PATH: %s\n", path);
                printf("ABSOLUTE path: %s\n", new_path);
                first_cmd[0] = new_path;
                break;
            }
        }
    }

    // first_cmd takes in input from fd_in and outputs to fd_out
    int cmd_pid = fork();
    if (cmd_pid == 0) {
        dup2(fd_in, STDIN_FILENO);   // Redirect input
        close(fd_in);

        dup2(fd_out, STDOUT_FILENO); // Redirect output
        close(fd_out);

        execve(first_cmd[0], first_cmd, NULL);

        fprintf(stderr, "Failed to execute first: %s\n", first_cmd[0]);
        // children should exit if exec fails
        _exit(1);
    }

    printf("parent %s pid is %d. forked %d. Parent exiting\n",
        argv[0], getpid(), cmd_pid);
    
    return 0;
}