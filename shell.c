#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "tokenize.c"

#define MAX_CMD_LENGTH 255

void print_help() {
  printf(
      "cd [dircctory]: change the current working directory of the shell to "
      "the path specified as the argument.\n");
  printf("help: explains the built-in commands available\n");
  printf("exit: terminates the shell program\n");
}

void execute_command(Token *tokens, int num_tokens) {
  // Find semicolons and execute commands sequentially
  int start_index = 0;
  int input_fd = 0;   // default input file descriptor
  int output_fd = 1;  // default output file descriptor
  char *input_file = NULL;
  char *output_file = NULL;
  int pipe_fd[2];       // pipe file descriptors
  pid_t pid1, pid2;     // process IDs of the child processes
  int pipe_index = -1;  // index of the pipe symbol in the tokens array

  for (int i = 0; i <= num_tokens; i++) {
    if (i == num_tokens || strcmp(tokens[i].value, ";") == 0) {
      // End of command
      char **args = malloc((i - start_index + 1) * sizeof(char *));
      int args_index = 0;
      for (int j = start_index; j < i; j++) {
        if (strcmp(tokens[j].value, "<") == 0) {
          // Input redirection
          if (j + 1 < i) {
            input_file = tokens[j + 1].value;
            input_fd = open(input_file, O_RDONLY);
            if (input_fd < 0) {
              printf("Failed to open input file: %s\n", input_file);
              exit(1);
            }
            j++;
          } else {
            printf("No input file specified.\n");
            exit(1);
          }
        } else if (strcmp(tokens[j].value, ">") == 0) {
          // Output redirection
          if (j + 1 < i) {
            output_file = tokens[j + 1].value;
            output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (output_fd < 0) {
              printf("Failed to open output file: %s\n", output_file);
              exit(1);
            }
            j++;
          } else {
            printf("No output file specified.\n");
            exit(1);
          }
        } else if (strcmp(tokens[j].value, "|") == 0) {
          // Pipe
          if (pipe(pipe_fd) < 0) {
            printf("Failed to create pipe.\n");
            exit(1);
          }
          pipe_index = j;
        } else {
          args[args_index] = tokens[j].value;
          args_index++;
        }
      }
      args[args_index] = NULL;

      if (pipe_index >= 0) {
        // There is a pipe
        // Create two child processes to execute the two commands
        pid1 = fork();
        if (pid1 < 0) {
          printf("Fork failed.\n");
          exit(1);
        } else if (pid1 == 0) {
          // Child process 1 (left side of pipe)
          close(pipe_fd[0]);  // close unused read end
          dup2(pipe_fd[1], STDOUT_FILENO);
          close(pipe_fd[1]);
          if (input_fd != 1) {
              // redirect stdin from input file
              int fd = open(input_file, O_RDONLY);
              if (fd < 0) {
                printf("Failed to open input file: %s\n", input_file);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);

          }
          execvp(args[0], args);
          printf("Failed to execute command: %s\n", args[0]);
          exit(1);
        } else {
          // Parent process
          pid2 = fork();
          if (pid2 < 0) {
            printf("Fork failed.\n");
            exit(1);
          } else if (pid2 == 0) {
            // Child process 2 (right side of pipe)
            close(pipe_fd[1]);  // close unused write end
            dup2(pipe_fd[0], STDIN_FILENO);  // redirect stdin from read end of pipe
            close(pipe_fd[0]);  // close read end of pipe
            if (output_fd != 1) {
              // redirect stdout to output file
          int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
          if (fd < 0) {
            printf("Failed to open output file: %s\n", output_file);
            exit(1);
          }
          dup2(fd, STDOUT_FILENO);
          close(fd);
            }
            execvp(args[pipe_index + 1], &args[pipe_index + 1]);
            printf("Failed to execute command: %s\n", args[pipe_index + 1]);
            exit(1);
          } else {
            // Parent process
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            input_fd = 0;
            output_fd = 1;
            input_file = NULL;
            output_file = NULL;
            pipe_index = -1;
          }
        }
      } else {
        // No pipe
        pid1 = fork();
        if (pid1 < 0) {
          printf("Fork failed.\n");
          exit(1);
        } else if (pid1 == 0) {
          // Child process
          if (input_fd != 0) {
            if (dup2(input_fd, 0) < 0) {
              printf("Failed to redirect input.\n");
              exit(1);
            }
            close(input_fd);
          }
          if (output_fd != 1) {
            if (dup2(output_fd, 1) < 0) {
              printf("Failed to redirect output.\n");
              exit(1);
            }
            close(output_fd);
          }
          execvp(args[0], args);
          printf("Failed to execute command: %s\n", args[0]);
          exit(1);
        } else {
          // Parent process
          waitpid(pid1, NULL, 0);
          input_fd = 0;
          output_fd = 1;
          input_file = NULL;
          output_file = NULL;
          pipe_index = -1;
        }
      }
      start_index = i + 1;
      free(args);
    }
  }
}

int main() {
  char cmd[MAX_CMD_LENGTH + 1];
  Token *tokens;
  int num_tokens, should_run = 1;
  char prev_cmd[MAX_CMD_LENGTH + 1] = "";

  printf("Welcome to mini-shell.\n");

  while (should_run) {
    printf("shell $ ");
    fflush(stdout);

    // Read the command and its arguments from the user
    fgets(cmd, MAX_CMD_LENGTH, stdin);
    cmd[strcspn(cmd, "\n")] = '\0';

    if (strcmp(cmd, "prev") == 0) {
      // Execute the previous command that was executed
      system(prev_cmd);
      continue;  // Restart the loop
    }

    // Tokenize the input
    tokens = tokenize(cmd, &num_tokens);

    // Check if the user entered exit
    if (strcmp(tokens[0].value, "exit") == 0) {
      printf("Bye bye.\n");
      should_run = 0;
    } else if (strcmp(tokens[0].value, "cd") == 0) {
      // Change directory command
      if (num_tokens < 2) {
        printf("cd: missing operand\n");
      } else {
        if (chdir(tokens[1].value) < 0) {
          printf("cd: %s: No such file or directory\n", tokens[1].value);
        }
      }
    } else if (strcmp(tokens[0].value, "help") == 0) {
      print_help();
    } else if (strcmp(tokens[0].value, "source") == 0) {
      // Source command
      if (num_tokens < 2) {
        printf("source: missing file operand\n");
      } else {
        FILE *fp = fopen(tokens[1].value, "r");
        if (fp == NULL) {
          printf("source: %s: No such file or       directory\n",
                 tokens[1].value);
        } else {
          char line[MAX_CMD_LENGTH + 1];
          while (fgets(line, MAX_CMD_LENGTH, fp) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            Token *line_tokens = tokenize(line, &num_tokens);
            execute_command(line_tokens, num_tokens);
            free(line_tokens);
          }
          fclose(fp);
        }
      }
    } else {
      execute_command(tokens, num_tokens);
      strncpy(prev_cmd, cmd, MAX_CMD_LENGTH);
    }

    free(tokens);
  }
  return 0;
}
