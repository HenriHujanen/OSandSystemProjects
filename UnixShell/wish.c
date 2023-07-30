#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_PATH_LENGTH 1024
#define MAX_PARALLEL_COMMANDS 32

char* path[MAX_PATH_LENGTH];
int pathCount = 0;

// Add paths to the search path
void addPath(const char* newPaths) {
    char *token = strtok((char*)newPaths, " \t\n");

    pathCount = 0;
    while (token != NULL) {
        path[pathCount++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }
}

char* findExecutable(const char* command) {
    if (command[0] == '/') {
        // Absolute path provided, check if executable
        if (access(command, X_OK) == 0) {
            return strdup(command);
        }
    } else {
        // Search in paths
        for (int i = 0; i < pathCount; i++) {
            char* fullPath = (char*)malloc(MAX_PATH_LENGTH);
            snprintf(fullPath, MAX_PATH_LENGTH, "%s/%s", path[i], command);
            if (access(fullPath, X_OK) == 0) {
                return fullPath;
            }
            free(fullPath);
        }
    }
    return NULL;
}

// Parse user input into segments
int parseCommand(char* command, char* args[]) {
    int i = 0;
    char* token = strtok(command, " \t\n");

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;

    return i;
}

// Print standard error message
void reportError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Handle built-in and other commands
int executeCommand(char* args[]) {
    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            reportError();
            return 1;
        }
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || args[2] != NULL) {
            reportError();
            return 1;
        }
        if (chdir(args[1]) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    } else if (strcmp(args[0], "path") == 0) {
        if (args[1] == NULL) {
            // Clear path
            pathCount = 0;
        } else {
            // Set new path
            addPath(args[1]);
        }
        return 0;
    }

    int outputRedirectIndex = -1;
    int fd;
    int originalStdout = dup(STDOUT_FILENO); // Save original stdout
    int originalStderr = dup(STDERR_FILENO); // Save original stderr

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL || args[i + 2] != NULL) {
                reportError();
                return 1;
            }
            outputRedirectIndex = i;
            break;
        }
    }

    if (outputRedirectIndex != -1) {
        // Redirect standard output and standard error to the file
        char* outputFile = args[outputRedirectIndex + 1];
        fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            return 1;
        }

        // Replace '>' and the filename with NULL for execv
        args[outputRedirectIndex] = NULL;
        args[outputRedirectIndex + 1] = NULL;

        // Redirect stdout and stderr to the file
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        // Close the file descriptor as it's no longer needed
        close(fd);
    }

    char* executable = findExecutable(args[0]);
    if (executable == NULL) {
        reportError();
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        execv(executable, args);
        perror("execv");
        free(executable);
        exit(1);
    } else {
        // Parent process
        free(executable);
        int status;
        waitpid(pid, &status, 0);

        // Reset stdout and stderr to their original values
        dup2(originalStdout, STDOUT_FILENO);
        dup2(originalStderr, STDERR_FILENO);

        return 0;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        reportError();
        exit(1);
    }

    addPath("/bin");

    FILE* inputFile = NULL;
    int interactiveMode = 1;

    if (argc == 2) {
        inputFile = fopen(argv[1], "r");
        if (!inputFile) {
            reportError();
            exit(1);
        }
        interactiveMode = 0;
    }

    char* args[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];

    pid_t parallelCommands[MAX_PARALLEL_COMMANDS];
    int numParallelCommands = 0;

    while (1) {
        if (interactiveMode) {
            printf("wish> ");
            fflush(stdout);
        }

        if (interactiveMode) {
            if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
                // EOF (Ctrl + D) encountered in interactive mode
                break;
            }
        } else {
            if (fgets(command, MAX_COMMAND_LENGTH, inputFile) == NULL) {
                // End of batch file reached
                break;
            }
            // Remove the trailing newline character from the command
            command[strcspn(command, "\n")] = '\0';
        }

        int numArgs = parseCommand(command, args);

        if (numArgs > 0) {
            if (strcmp(args[numArgs - 1], "&") == 0) {
                // Remove '&' from the command arguments
                args[numArgs - 1] = NULL;
                numArgs--;
            }

            if (numArgs == 0) {
                // Empty command, skip execution
                continue;
            }

            // Execute the command(s)
            int startIndex = 0;
            int i;
            for (i = 0; i < numArgs; i++) {
                if (strcmp(args[i], "&") == 0) {
                    args[i] = NULL; // Replace '&' with NULL for execvp
                    int pid = executeCommand(&args[startIndex]);
                    if (pid != 1) {
                        // Store the PID for parallel commands
                        parallelCommands[numParallelCommands++] = pid;
                    }
                    startIndex = i + 1;
                }
            }
            // Execute the last command if any
            if (startIndex < i) {
                int pid = executeCommand(&args[startIndex]);
                if (pid != 1) {
                    // Store the PID for parallel commands
                    parallelCommands[numParallelCommands++] = pid;
                }
            }
        }
    }

    // Wait for all parallel commands to finish
    for (int i = 0; i < numParallelCommands; i++) {
        int status;
        waitpid(parallelCommands[i], &status, 0);
    }

    if (inputFile) {
        fclose(inputFile);
    }

    // Free allocated path strings
    for (int i = 0; i < pathCount; i++) {
        free(path[i]);
    }

    return 0;
}
