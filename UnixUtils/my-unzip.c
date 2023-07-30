#include <stdio.h>
#include <stdlib.h>

void unzip_file(FILE *input_file) {
    int count;
    int current_char;

    while (fread(&count, sizeof(int), 1, input_file) == 1) {
        current_char = fgetc(input_file);

        // Repeat the character count and print to standard output
        for (int i = 0; i < count; i++) {
            putchar(current_char);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // If no file specified, exit with status 1
        fprintf(stderr, "my-unzip: file1 [file2 ...]\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "my-unzip: cannot open file '%s'\n", argv[i]);
            exit(1);
        }

        unzip_file(file);
        fclose(file);
    }

    exit(0);
}
