#include "prompt.h"
#include "rand.h"
#include "words.h"
#include "malloc.h"

static int randidx(int max) {
    return rand() % max;
}

// generate prompt of length n
const char** getprompt(int n) {
    int total = lenwordlist();
    const char** prompt = malloc(n * sizeof(const char*));
    for (int i = 0; i < n; i++) {
        prompt[i] = getword(randidx(total));
    }
    return prompt;
}