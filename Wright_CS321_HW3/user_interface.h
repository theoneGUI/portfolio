#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared.h"

typedef enum {
    ALL_PRINTABLE,
    ALPHANUMERIC,
    NUMERIC
} alphabet;

typedef struct {
    alphabet alpha;
    char* hashToCrack;
    int threads;
} args;

void help(int argc, char* argv[]);

args* eval_cmdline(int argc, char* argv[]) {
    if (argc < 2)
        return NULL;

    args* ret = (args*)malloc(sizeof(args));
    ret->alpha = ALPHANUMERIC;
    ret->threads = 2;

    bool skip = false;
    for (int i = 1; i < argc; i++) {
        if (skip) {
            skip = false;
            continue;
        }

        if (strcmp(argv[i],"--printable") == 0){
            ret->alpha = ALL_PRINTABLE;
        }
        else if (strcmp(argv[i],"--alphanum") == 0) {
            ret->alpha = ALPHANUMERIC;
        }
        else if (strcmp(argv[i], "--num") == 0) {
            ret->alpha = NUMERIC;
        }
        else if (strcmp(argv[i], "-t") == 0) {
            skip = true;
            ret->threads = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--help") == 0) {
            help(argc, argv);
            free(ret);
            return NULL;
        }
        else if (i != 1) {
            printf("Unrecognized option '%s'. Quitting.\n", argv[i]);
            free(ret);
            return NULL;
        }
    }

    // get the word we're brute forcing and convert it to our integer representation
    char* targetWord = argv[1];
    if (strlen(targetWord) != 64) {
        printf("Please enter a SHA256 hex string as the first thing after the command name.\n");
        free(ret);
        return NULL;
    }
    ret->hashToCrack = targetWord;

    return ret;
}

void help(int argc, char* argv[]) {
    printf("A SHA256-cracking tool.\nAuthor: Aidan Wright\nCS 321 Threading Homework\n");
    
    printf("\nWarning: on my machine at least, anything over 5 characters takes a very long time.\nUsage:\n\n");
    printf("%s [argument] [options]\n", argv[0]);
    printf("\nArgument:\n");
    printf("\n**** You have to put the argument first, before all options ****\n");
    printf("\tThis program only takes one argument: a hex digest of a SHA256 representation of a piece of text.\n");
    printf("\tThe argument is required");
    printf("\n\nOptions:\n");
    printf("\t--help: Show this message\n");
    printf("\t-t [integer]: Number of threads to use while cracking (2 if not specified)\n");
    printf("\t\t(Maximum 16 threads, change if your number of hardware threads is greater than 16)\n");
    printf("\t--printable: Brute force using all printable ASCII chars (slowest, least limited)\n");
    printf("\t--alphanum: Brute force using all English alphanumeric characters (faster, default if no others specified)\n");
    printf("\t--num: Brute force using only the digits 0-9 (fastest, most limited)\n");
}

#endif