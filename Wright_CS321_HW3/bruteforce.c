/*
Aidan Wright


* One challenge that you faced in writing a multi-threaded program using pthreads

    The biggest challenge I faced was being able to dispatch work to ready threads. I used to have it that the main 
    thread would wait for all n threads to join before continuing to dispatch. You would expect all the threads to
    join at nearly the same time, but sometimes one or two would take much longer than the others, so precious time
    that could be spent trying letter combinations was wasted in idle cycles. I added another thread whose entire 
    purpose in life is to check up on everyone else's workload, and if they're done, assign more dynamically. That 
    was the biggest problem related to threads, aside from that, making segfaults for myself and trying to get a 
    good handle on C were the bigger issues I spent most of my time debugging. The only inter-thread communication
    that I had to do was indicating which threads need new tasks. Those are "safe" from errors in the critical 
    section because each thread only reads once and writes once to its specified index in the master array of 
    threads. 

* One specific way in which a multi-threaded approach to your program represents an improvement over using a single thread:

    It's pretty obvious the improvement when you try 1 thread vs 16. The improvement mostly comes when you have
    only one software thread running per one hardware thread for this application. If you're running 16 software threads
    on a single hardware thread, then it's really only just adding overhread to processing, but in general, the advantage
    to multithreading when brute forcing is that you get to try many iterations at the exact same time, lowering the time
    you're going to be waiting. As opposed to the single threaded approach where one thread has to check the whole space 
    of [a-z][a-z][a-z][a-z] and so on, each thread tries 1/n combinations (n being the total number of threads).


*********************

There are things I call the int rep or integer representation which 
are the character equivalent of strings except converted to their 
characters' indices in the ALL_CHARS string. I did that because
knowing the index in ALL_CHARS lets me make assumptions about the 
string and the characters inside it, their proximity, etc, and it
lets me make performance improvements by using those assumptions
in the brute forcing functions, especially the permute function.

*********************
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "sha256.c"
#include "user_interface.h"
#include "shared.h"
#include "limits.h"
#include "sized.h"

// The string that the hash is equivalent to once the hash is cracked
static sized_thing* found_string = NULL;

// Allocate space for our lookup table
static short* CHAR_LOOKUP[256];

// Generate a lookup table for quick access when converting from the int representation to strings
void gen_lookup();

// Do the lookup for character representation of an int representation
char lookup_by_int(short);

// Do the lookup for the int representation of a character
short lookup_by_char(char);

sized_thing* int_rep_from_str(const sized_thing*);
sized_thing* str_from_int_rep(const sized_thing*);

void free_threads_work(limits** limitsArr, int num_threads);

// Permute the given int representation 
void next_iter(sized_thing* t) {
    int maxxed = 0;
    int len = t->size;
    short* as_int = (short*)t->pointer;

    for (int i = len-1; i >= 0 && as_int[i] == max_charcode; i--) {
        maxxed++;
    }

    if (maxxed == len) {
        // new digit at the end && set to ALL_CHARS[0]
        free(as_int);
        as_int = malloc(sizeof(short) * (len+1));
        t->size = len+1;
        for (int i = 0; i < len+2; i++) {
            as_int[i] = 0;
        }
    }
    else if (maxxed == 0) {
        // simply increment the last character
        as_int[len-1] = as_int[len-1] + 1;
    }
    else {
        // set all maxxed chars to ALL_CHARS[0] and increment the furthest non-maxxed
        for (int j = 1; j <= maxxed; j++) {
            as_int[len-j] = 0;
        }

        // this is safe against indexing to -1 because of the first if statement
        as_int[len-(maxxed+1)] = as_int[len-(maxxed+1)] + 1;
    }
}

// Important information for the task manager thread to operate 
typedef struct {
    int num_threads;
    sized_thing* target;
    int wordLength;
    short lastLim;
} threadMan_t;
threadMan_t* man = NULL;
limits* limits_generator();
limits**    list_of_limits;

/*
    The body function for brute force worker threads.
    Takes a limit pointer argument and uses that to 
    determine how long it will keep permuting for.
    
    It will stop when it hits limit->max or it finds
    the plaintext.
*/
void* bruteWorker(void* input) {
    limits* limit = input;

    SHA256_CTX* ctx = malloc(sizeof(SHA256_CTX));
    BYTE* hash = malloc(sizeof(char) * 32);
    sized_thing* sized_hash = create_allocated(hash, sizeof(char) * 32, sizeof(char));

    sized_thing* as_str;

    sized_thing* q = str_from_int_rep(limit->max);
    printf("Checking up to '%s'\n", q->pointer);
    free_allocated(q);
    while (found_string == NULL && !sized_equal(limit->min, limit->max)) {
        sha256_init(ctx);
        as_str = str_from_int_rep(limit->min);
        sha256_update(ctx, as_str->pointer, as_str->size);
        sha256_final(ctx, hash);
        if (sized_equal(sized_hash, limit->target)) {
            found_string = as_str;
            break;
        }
        next_iter(limit->min);
        free_allocated(as_str);
    }

    free(ctx);
    free_allocated(sized_hash);
    free_limits(list_of_limits[limit->index]);
    list_of_limits[limit->index] = NULL;

    pthread_exit(0);
}

void* threadManager(void*) {
    /*
        This thread's only job is to assign more work as new threads need it. 
        The old version of this program would wait for all threads to join and assign new work in a batch.
        That wasted a lot of time and now I have this daemon thread to be a sort of task manager.
    */
    int s;
    while (found_string == NULL)
    {
        for (int i = 0; i < man->num_threads; i++) {
            if (list_of_limits[i] == NULL) {
                list_of_limits[i] = limits_generator();
                list_of_limits[i]->index = i;
                pthread_t newThread;
                pthread_create(&newThread, NULL, bruteWorker, list_of_limits[i]);

                pthread_detach(newThread);
            }
        }
        sleep(1);
    }
}

int main(int argc, char* argv[]) {
    // initialize lookup table
    gen_lookup();

    // if they didn't enter anything, they probably want help
    // if (argc == 1) {
    //     help(argc, argv);
    //     exit(1);
    // }

    args* allArgs = eval_cmdline(argc, argv);
    if (allArgs == NULL) {
        exit(1);
    }

    if (allArgs->alpha == ALL_PRINTABLE) {
        max_charcode = strlen(ALL_CHARS);
    }
    else if (allArgs->alpha == ALPHANUMERIC) {
        max_charcode = (26 * 2) + 10;
    }
    else if (allArgs->alpha == NUMERIC) {
        max_charcode = 10;
    }

    // Can't have less than one character to check per thread
    if (allArgs->threads > max_charcode) {
        allArgs->threads = max_charcode;
    }

    // cap at 16 threads (max hardware threads of my laptop)
    printf("Finding plaintext for hash: %s\n", allArgs->hashToCrack);
    // let the user know the alphabet being used
    printf("The characters being used in the brute force:\n\t");
    for (int i = 0; i < max_charcode; i++) {
        printf("%c", ALL_CHARS[i]);
    }
    printf("\nAnd now we can get cracking...\n");
    
    // turn the string hex SHA256 digest into binary
    char* binSha = malloc(32);
    int bytecount = 0;
    for (int i = 0; i < strlen(allArgs->hashToCrack); i+=2) {
        char b[3] = {allArgs->hashToCrack[i], allArgs->hashToCrack[i+1], '\0'};
        char byte = (char)strtoul(b, NULL, 16);
        binSha[bytecount] = byte;
        bytecount++;
    }
    sized_thing* target = create_allocated(binSha, 32, sizeof(char));

    // create a list of limits that threads will try to crack
    list_of_limits  = malloc(sizeof(limits) * allArgs->threads);
    for (int i = 0; i < allArgs->threads; i++) {
        list_of_limits[i] = NULL;
    }

    // prepare the thread manager object with all the info we have so far
    man = malloc(sizeof(threadMan_t));
    man->lastLim = 0;
    man->num_threads = allArgs->threads;
    man->target = target;
    man->wordLength = 1;

    int startTime = time(NULL);
    pthread_t threadMan;
    pthread_create(&threadMan, NULL, threadManager, NULL);
    pthread_detach(threadMan);

    while (found_string == NULL) {
        //Loop until it's been found
        sleep(1);
    }
    int totalTime = time(NULL) - startTime;

    printf("Found the hashed text: %s\n", found_string->pointer);
    printf("It took %d seconds to brute force the hash\n", totalTime);
    free_allocated(found_string);
    free_threads_work(list_of_limits, allArgs->threads);
    free(target);
    free(allArgs);
    free(man);
}


sized_thing* int_rep_from_str(const sized_thing* c) {
    char* arr = (char*)c->pointer;
    sized_thing* ret = create_allocated(malloc(sizeof(short) * c->size), c->size, sizeof(short));

    for (int ix = 0; ix < ret->size; ix++) {
        ((short*)ret->pointer)[ix] = lookup_by_char(arr[ix]);
    }
    return ret;
}
sized_thing* str_from_int_rep(const sized_thing* i) {
    short* arr = (short*)i->pointer;
    sized_thing* ret = create_allocated(malloc(sizeof(char) * i->size), i->size, sizeof(char));

    for (int ix = 0; ix < i->size; ix++) {
        ((char*)ret->pointer)[ix] = lookup_by_int(arr[ix]);
    }

    ((char*)ret->pointer)[ret->size] = '\0';
    return ret;
}



limits* limits_generator() {
    /*
        Dividing up thread work according to this example:

        Given 2 threads and only lowercase English alphabetic characters and word length of 4:
            Thread 1 checks all words looking like [a-n][a-z][a-z][a-z]
            Thread 2 checks all words looking like [o-z][a-z][a-z][a-z]

        It divides the work amongst threads by the most significant character

        This function acts like a Python generator so that when you call the function, it provides
        the next iteration without needing to do a bunch of loops again.
    */
    double splitSize = ((double)max_charcode / ((double)man->num_threads));
    if (man->lastLim == man->num_threads) {
        man->lastLim = 0;
        man->wordLength++;
    }

    limits* nextLimit = create_limits(man->wordLength, man->target);

    ((short*)(nextLimit->min->pointer))[0] = ((short)splitSize * man->lastLim) + (man->lastLim == 0 ? 0 : 1);

    if (man->lastLim == man->num_threads-1){
        short diff = max_charcode - (splitSize * (man->lastLim+1));
        ((short*)(nextLimit->max->pointer))[0] = ((splitSize * (man->lastLim+1)) + diff) - 1;
    }
    else {
        ((short*)(nextLimit->max->pointer))[0] = (short)splitSize * (man->lastLim+1);
    }

    for (int j = 1; j <= man->wordLength; j++) {
        ((short*)(nextLimit->min->pointer))[j] = 0;
        ((short*)(nextLimit->max->pointer))[j] = max_charcode-1;
    }

    man->lastLim++;
    return nextLimit;
}
void free_threads_work(limits** limitsArr, int num_threads) {
    for (short lim = 0; lim < num_threads; lim++) {
        free_limits(limitsArr[lim]);
    }
    free(limitsArr);
}



void gen_lookup() {
    int maxJ = strlen(ALL_CHARS);

    // generate table for the from char lookup
    // indexing by a char gives the char's position in the string
    for (int j = 0; j < maxJ; j++) {
        short c = (short)ALL_CHARS[j];
        CHAR_LOOKUP[c] = j;
    }
}
short lookup_by_char(char i) {
    return CHAR_LOOKUP[(short)i];
}
char lookup_by_int(short i) {
    return ALL_CHARS[i];
}