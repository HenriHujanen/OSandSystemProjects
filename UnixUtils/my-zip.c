#include <stdio.h>
#include <stdlib.h>

void zip_file(FILE *input_file) {
    int prev_char = EOF; // Initialize with a value that won't match any character
    int count = 0;
    int current_char;

    while ((current_char = fgetc(input_file)) != EOF) {
        // If the character is the same as the previous one, increment the count
        if (current_char == prev_char) {
            count++;
        } else {
            // If a new character is encountered, print the count and the character
            if (prev_char != EOF) {
                fwrite(&count, sizeof(int), 1, stdout);
                putchar(prev_char);
            }
            count = 1;
            prev_char = current_char;
        }
    }

    // Print the last character and its count
    if (prev_char != EOF) {
        fwrite(&count, sizeof(int), 1, stdout);
        putchar(prev_char);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // If no file specified, exit with status 1
        fprintf(stderr, "my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "my-zip: cannot open file '%s'\n", argv[i]);
            exit(1);
        }

        zip_file(file);
        fclose(file);
    }

    exit(0);
}
