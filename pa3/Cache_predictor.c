#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_INPUT_FILE "cache_input.txt"
#define DEFAULT_OUTPUT_FILE "predictions.txt"

#define MAX_ADDRESS 4096 
#define MAX_TRANS   16

typedef struct {
    unsigned int next_addr;
    unsigned int freq;
} Trans;

typedef struct {
    Trans t[MAX_TRANS];
} StateHistory;

static StateHistory history[MAX_ADDRESS];
static unsigned int frequency[MAX_ADDRESS];

static inline int should_ignore(int index) {
    int pos = index % 16;
    return (pos > 5 && pos <= 9);
}

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
    if (count == 0) return;

    memset(history, 0, sizeof(history));
    memset(frequency, 0, sizeof(frequency));

    unsigned int most_common = 0;
    unsigned int highest_freq = 0;

    // global mode calculation
    for (int i = 0; i < count; i++) {
        if (should_ignore(i)) continue;
        unsigned int addr = (unsigned int)addrs[i];
        if (addr < MAX_ADDRESS) {
            frequency[addr]++;
            if (frequency[addr] > highest_freq) {
                highest_freq = frequency[addr];
                most_common = addr;
            }
        }
    }

    // training phase
    for (int i = 0; i < count - 1; i++) {
        if (should_ignore(i) || should_ignore(i + 1)) continue;

        unsigned int curr = (unsigned int)addrs[i];
        unsigned int next = (unsigned int)addrs[i + 1];

        if (curr >= MAX_ADDRESS || next >= MAX_ADDRESS) continue;

        StateHistory *sh = &history[curr];
        int found = 0;
        int min_idx = 0;
        unsigned int min_freq = 0xFFFFFFFF;

        for (int j = 0; j < MAX_TRANS; j++) {
            if (sh->t[j].next_addr == next && sh->t[j].freq > 0) {
                sh->t[j].freq++;
                found = 1;
                break;
            }
            if (sh->t[j].freq < min_freq) {
                min_freq = sh->t[j].freq;
                min_idx = j;
            }
        }

        if (!found) {
            sh->t[min_idx].next_addr = next;
            sh->t[min_idx].freq = 1;
        }
    }

    // prediction phase
    unsigned int current_state = 0;
    for (int i = count - 1; i >= 0; i--) {
        if (!should_ignore(i)) {
            current_state = (unsigned int)addrs[i];
            break;
        }
    }

    // prediction generation
    for (int i = 0; i < count; i++) {
        unsigned int prediction;

        if (should_ignore(i)) {
            prediction = (unsigned int)addrs[i];
        } else {
            int best_freq = 0;
            unsigned int markov_pred = 0;

            if (current_state < MAX_ADDRESS) {
                StateHistory *sh = &history[current_state];
                for (int j = 0; j < MAX_TRANS; j++) {
                    if (sh->t[j].freq > best_freq) {
                        best_freq = sh->t[j].freq;
                        markov_pred = sh->t[j].next_addr;
                    }
                }
            }

            if (best_freq > 0) {
                prediction = markov_pred;
            } else {
                prediction = most_common;
            }

            current_state = prediction;
        }

        predictions[i] = (unsigned long long)prediction;
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