#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "./headers/entryType.h"
#include "./headers/randomUtil.h"


void initShm(entryPtr shmPtr, int bufferSize) {
    for(int entry = 0; entry < bufferSize; entry++) {
        sem_init(&shmPtr[entry].writeSem, 1, 1);
        sem_init(&shmPtr[entry].readSem, 1, 1);
    }
}


int main(int argc, char const *argv[]) {
    srand(time(NULL));

    key_t shmKey = rand() % 1000;
    pid_t pid;
    int bufferSize = atoi(argv[1]),         // Size of shared memory buffer
        peers = atoi(argv[2]),              // Num of reader peers
        readersRatio = atoi(argv[3]),       // Num of writer peers
        writersRatio = atoi(argv[4]),
        reps = atoi(argv[5]),               // Times that peer tries to access entry
        readers = peers * reps * readersRatio / (readersRatio + writersRatio),  // Totals amount of readers
        writers = peers * reps * writersRatio / (readersRatio + writersRatio);  // Totals amount of writers

    if(bufferSize < 1) {
        printf("Error: Invalid shared memory buffer size\n");
        exit(1);
    } else if(peers < 0) {
        printf("Error: Invalid number of peers\n");
        exit(1);
    } else if(readers + writers != peers * reps) {
        printf("Error: Invalid ratio\n");
        exit(1);
    } else if(reps < 0) {
        printf("Error: Invalid number of repetitions\n");
        exit(1);
    }
    // Get id to shared memory segment
    int shmID = shmget(shmKey, bufferSize * sizeof(struct entry), 0666|IPC_CREAT);
    if (shmID == -1) {
        printf("Error: Shared memory segment\n");
        exit(1);
    }
    // Get pointer to shared memory
    entryPtr shmPtr = shmat(shmID, NULL, 0);
    if (shmPtr == (void *) -1) {
        printf("Error: Shared memory attach\n");
        exit(1);
    }
    // Initialise shared memory
    initShm(shmPtr, bufferSize);

    char shmidStr[10], entryStr[10], readersStr[10], writersStr[10];
    int peerReads, max, min;
    sprintf(shmidStr, "%d", shmID);
    // Start peers
    for(int currPeer = 0; currPeer < peers; currPeer++) {
        max = (readers < reps) ? readers : reps;
        min = (writers < reps) ? reps - writers : 0;
        peerReads = rand() % (max - min + 1) + min;     // Num of reads for currPeer

        readers -= peerReads;               // reduce remaining reads
        writers -= (reps - peerReads);      // reduce remaining writes

        sprintf(readersStr, "%d", peerReads);
        sprintf(writersStr, "%d", reps - peerReads);
        sprintf(entryStr, "%d", uniformDist_rand(0, bufferSize - 1));   // Random entry
        // Create new process and execute peer
        if(!(pid = fork()))
            execl("./peer", "peer", shmidStr, entryStr, readersStr, writersStr, NULL);
        else if(pid == -1) {
            printf("Error: Fork\n");
            exit(1);
        }
    }
    // Wait for all peers to finish
    int status = 0;
    while(wait(&status) > 0);

    int totalReads = 0, totalWrites = 0;
    // Print Stats
    printf("\nCoordinator PID %d\n\nShared Memory:\n", getpid());
    for(int entry = 0; entry < bufferSize; entry++) {
        printf("Entry %d  |  Reads %d  |  Writes %d\n",
            entry, shmPtr[entry].reads, shmPtr[entry].writes);

        totalReads += shmPtr[entry].reads;
        totalWrites += shmPtr[entry].writes;
    }

    printf("\nTotal Reads : %d\nTotal Writes: %d\n", totalReads, totalWrites);

    if (shmdt(shmPtr) == -1) {
        printf("Error: Shared memory detach\n");
        exit(1);
    }

    return 0;
}