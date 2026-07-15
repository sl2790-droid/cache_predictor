#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_INPUT_FILE "cache_input.txt"
#define DEFAULT_OUTPUT_FILE "predictions.txt"

int count = 0;
unsigned long long *addrs;
unsigned long long *predictions;

int readFromFile(const char *filename);
int writeToFile(const char *filename);
void predict();

int main(int argc, char *argv[]) {
    char *input_file;
    char *output_file;
    if (argc != 3) {
        input_file = DEFAULT_INPUT_FILE;
        output_file = DEFAULT_OUTPUT_FILE;
    } else {
        input_file = argv[1];
        output_file = argv[2];
    }
    if (readFromFile(input_file)) return 1;
    predict();
    if (writeToFile(output_file)) return 1;
    return 0;
}

void predict() {
    if (count == 1) {
        predictions[0] = addrs[0];
    } else {
        for (int i = 0; i < count; i++) {
            predictions[i] = addrs[count - 1];
        }
    }
}

int readFromFile(const char *filename) {
    FILE *in = fopen(filename, "r");
    if (!in) { perror("fopen input"); return 1; }

    // get the number of lines in the input file
    count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), in)) {
        (count)++;
    }
    if (count == 0) {
        fclose(in);
        return 1;
    }

    // allocate memory for addresses
    addrs = (unsigned long long*)malloc(count * sizeof(unsigned long long));
    predictions = (unsigned long long*)malloc(count * sizeof(unsigned long long));
    if (!addrs || !predictions) {
        perror("malloc");
        fclose(in);
        return 1;
    }

    // read addresses from the input file
    rewind(in);
    int i = 0;
    while (fgets(buffer, sizeof(buffer), in) && i < count) {
        unsigned long long addr;
        if (sscanf(buffer, "%llx", &addr) == 1) {
            addrs[i] = addr;
            i++;
        }
    }
    fclose(in);
    return 0;
}

int writeToFile(const char *filename) {
    FILE *out = fopen(filename, "w");
    if (!out) { perror("fopen output"); return 1; }
    for (int i = 0; i < count; i++) {
        fprintf(out, "0x%llx\n", predictions[i]);
    }
    fclose(out);
    return 0;
}

