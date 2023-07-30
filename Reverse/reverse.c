#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

// Print error messages to stderr
void print_error(const char *message) {
    fprintf(stderr, "%s\n", message);
}

void reverse_lines(FILE *input, FILE *output) {
    // Dynamic array to store lines
    char **lines = (char **)malloc(INITIAL_CAPACITY * sizeof(char *));
    if (!lines) {
        print_error("malloc failed");
        exit(1);
    }

    size_t capacity = INITIAL_CAPACITY;
    size_t count = 0;
    char *line = NULL;
    size_t length = 0;

    while (getline(&line, &length, input) != -1) {
        if (count >= capacity) {
            capacity *= 2;
            char **temp = (char **)realloc(lines, capacity * sizeof(char *));
            if (!temp) {
                print_error("malloc failed");
                free(line);
                exit(1);
            }
            lines = temp;
        }

        lines[count++] = strdup(line);
    }
    free(line);

    if (input != stdin) {
        fclose(input);
    }

    // Output revesed lines to either to output.txt or stdout
    if (output) {
        for (size_t i = count; i > 0; i--) {
            fprintf(output, "%s", lines[i - 1]);
            free(lines[i - 1]);
        }
        fclose(output);
    } else {
        for (size_t i = count; i > 0; i--) {
            printf("%s", lines[i - 1]);
            free(lines[i - 1]);
        }
    }

    free(lines);
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    FILE *output = NULL;

    // Check for errors and reverse input
    if (argc > 3) {
        print_error("Usage: reverse <input> <output>");
        exit(1);
    }

    if (argc >= 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            char error_message[100];
            snprintf(error_message, sizeof(error_message), "error: cannot open file '%s'", argv[1]);
            print_error(error_message);
            exit(1);
        }

        if (argc == 3) {
            if (strcmp(argv[1], argv[2]) == 0) {
                print_error("Input and output file must differ");
                fclose(input);
                exit(1);
            }
            output = fopen(argv[2], "w");
            if (!output) {
                char error_message[100];
                snprintf(error_message, sizeof(error_message), "error: cannot open file '%s'", argv[2]);
                print_error(error_message);
                fclose(input);
                exit(1);
            }
        }
    }

    reverse_lines(input, output);

    return 0;
}
