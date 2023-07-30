#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mygrep(FILE *file, const char *search_term) {
    char *line = NULL;
    size_t len = 0;
    int read;

    // Read lines from given file and check if term is present
    while ((read = getline(&line, &len, file)) != -1) {
        if (strstr(line, search_term) != NULL) {
            printf("%s", line);
        }
    }

    free(line);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "my-grep: searchterm [file ...]\n");
        exit(1);
    }

    const char *search_term = argv[1];

    // If no file given, read from stdin
    if (argc == 2) {
        mygrep(stdin, search_term);
        exit(0);
    }

    for (int i = 2; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "my-grep: cannot open file '%s'\n", argv[i]);
            exit(1);
        }

        mygrep(file, search_term);
        fclose(file);
    }

    exit(0);
}
