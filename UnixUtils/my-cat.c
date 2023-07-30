#include <stdio.h>
#include <stdlib.h>

void print_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "my-cat: cannot open file '%s'\n", filename);
        exit(1);
    }

    // Read characters from specified file and print them to stdout
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // If no files specified, exit with code 0
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        print_file(argv[i]);
    }

    // If all files printed successfully, exit with code 0
    exit(0);
}
