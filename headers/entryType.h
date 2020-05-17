#include <semaphore.h>

#define entryPtr struct entry*

struct entry {
    int writes, reads;
    int nowReading, nowWriting;
    sem_t writeSem, readSem;
};
