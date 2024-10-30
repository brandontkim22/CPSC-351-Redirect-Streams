#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

void add_char_to_string(char* word, char c) {
    int len = strlen(word);
    word[len] = c;
    word[len + 1] = '\0';
}

void break_into_words(char* input, char* words[], char break_on) {
    int word_count = 0; // index into words output array
    char* current_char = input;

    char word_so_far[1000];
    strcpy(word_so_far, "");

    while (*current_char != '\0') {
        if (*current_char == break_on) {
            words[word_count++] = strdup(word_so_far);
            word_so_far[0] = '\0'; // set back to empty string
        } else {
            add_char_to_string(word_so_far, *current_char);
        }
        current_char++;
    }
    words[word_count++] = strdup(word_so_far);

    words[word_count] = NULL;
}

bool find_absolute_path(char* no_path, char* with_path) {
    char* directories[1000];

    break_into_words(getenv("PATH"), directories, ':');

    for (int ix = 0; ix < 1000 && directories[ix] != NULL; ix++) {
        strcpy(with_path, directories[ix]);
        strcat(with_path, "/");
        strcat(with_path, no_path);
        if (access(with_path, X_OK) == 0) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        fprintf(stderr,
            "Usage: %s <input.txt> <command> <output.txt>\n", argv[0]);
        return 1;
    }

    char* command_args[100];
    char absolute_path[100];

    // Separate command argument into arguments (if needed)
    // and get the absolute path of the command
    break_into_words(argv[2], command_args, ' ');
    if (find_absolute_path(command_args[0], absolute_path) == false) {
        printf("Could not find absolute path for %s\n", command_args[0]);
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

    // Command takes in input from fd_in and outputs to fd_out
    int cmd_pid = fork();
    if (cmd_pid == 0) {
        dup2(fd_in, STDIN_FILENO);   // Redirect input
        close(fd_in);

        dup2(fd_out, STDOUT_FILENO); // Redirect output
        close(fd_out);

        execve(absolute_path, command_args, NULL);

        fprintf(stderr, "Failed to execute first: %s\n", command_args[0]);
        // children should exit if exec fails
        _exit(1);
    }

    printf("parent %s pid is %d. forked %d. Parent exiting\n",
        argv[0], getpid(), cmd_pid);
    
    return 0;
}