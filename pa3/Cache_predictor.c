#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_INPUT_FILE "cache_input.txt"
#define DEFAULT_OUTPUT_FILE "predictions.txt"

unsigned long long* readFromFile(const char *filename, int *count);
int writeToFile(const char *filename, unsigned long long *predictions, int count);

int main(int argc, char *argv[]) {
    // read command line arguments or use default file names
    char *input_file;
    char *output_file;
    if (argc != 3) {
        input_file = DEFAULT_INPUT_FILE;
        output_file = DEFAULT_OUTPUT_FILE;
    } else {
        input_file = argv[1];
        output_file = argv[2];
    }


    // read addresses from the input file
    int count;
    unsigned long long *addrs = readFromFile(input_file, &count);
    if (!addrs) {
        return 1;
    }

    // generate predictions
    unsigned long long *predictions = (unsigned long long*)malloc(count * sizeof(unsigned long long));
    if (count == 1) {
        predictions[0] = addrs[0];
    } else {
        for (int i = 0; i < count; i++) {
            predictions[i] = addrs[count - 1];
        }
    }


    // write predictions to the output file
    if (writeToFile(output_file, predictions, count)) {
        return 1;
    }

    return 0;
}

unsigned long long* readFromFile(const char *filename, int *count) {
    FILE *in = fopen(filename, "r");
    if (!in) { perror("fopen input"); return NULL; }

    // get the number of lines in the input file
    *count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), in)) {
        (*count)++;
    }
    if (*count == 0) {
        fclose(in);
        return NULL;
    }

    // allocate memory for addresses
    unsigned long long *addrs = (unsigned long long*)malloc(*count * sizeof(unsigned long long));
    if (!addrs) {
        perror("malloc");
        fclose(in);
        return NULL;
    }

    // read addresses from the input file
    rewind(in);
    int i = 0;
    while (fgets(buffer, sizeof(buffer), in) && i < *count) {
        unsigned long long addr;
        if (sscanf(buffer, "%llx", &addr) == 1) {
            addrs[i] = addr;
            i++;
        }
    }
    fclose(in);
    return addrs;
}

int writeToFile(const char *filename, unsigned long long *predictions, int count) {
    FILE *out = fopen(filename, "w");
    if (!out) { perror("fopen output"); return 1; }
    for (int i = 0; i < count; i++) {
        fprintf(out, "0x%llx\n", predictions[i]);
    }
    fclose(out);
    return 0;
}

