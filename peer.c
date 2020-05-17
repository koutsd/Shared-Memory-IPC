#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "./headers/entryType.h"
#include "./headers/randomUtil.h"


int main(int argc, char const *argv[]) {
    srand(time(NULL));

    pid_t pid = getpid();   // ID of current process
    int isAvailable,
        shmID = atoi(argv[1]),      // ID of shared memory segment
        entry = atoi(argv[2]),      // Entry num
        readers = atoi(argv[3]),
        writers = atoi(argv[4]),
        reps = readers + writers,
        reads = readers,
        writes = writers;

    // Get pointer to shared memory
    entryPtr shmPtr = shmat(shmID, NULL, 0);
    if (shmPtr == (void *) -1) {
        printf("PID %d Error: Shared memory attach\n", pid);
        exit(1);
    }

    clock_t wait, start;
    double runTime, waitTime = 0.0;   

    while(readers + writers) {
        runTime = exponentialDist_rand(0.00001);    // Time for process to occupy entry
        wait = clock();                             // start waiting to access entry

        if(rand() % (readers + writers) < writers) {    // writer
            writers--;
            // Wait writer to stop using entry
            sem_wait(&shmPtr[entry].writeSem);
            // Wait all readers to stop using entry
            while(shmPtr[entry].nowReading);
            start = clock();    // Start using entry

            shmPtr[entry].nowWriting++;     // Indicator that entry is occupied by writer
            waitTime += (double)(start - wait) / CLOCKS_PER_SEC;    // Time waited to access entry
            shmPtr[entry].writes++;         // Write in entry

            usleep(runTime);
            shmPtr[entry].nowWriting--;     // Indicates that peer stoped using entry

            sem_post(&shmPtr[entry].writeSem);  // release entry

        } else {        // reeder
            readers--;
            // Wait for writer to finish using entry
            while(shmPtr[entry].nowWriting);
            start = clock();    // Start using entry
            
            sem_wait(&shmPtr[entry].readSem);
            shmPtr[entry].reads++;          // Increment number of times entry was read
            shmPtr[entry].nowReading++;     // Indicate reader is occupying entry
            sem_post(&shmPtr[entry].readSem);
            // Time waited to access entry
            waitTime += (double)(start - wait) / CLOCKS_PER_SEC;
            usleep(runTime);

            sem_wait(&shmPtr[entry].readSem);
            shmPtr[entry].nowReading--;     // Indicate peer stopped reading entry
            sem_post(&shmPtr[entry].readSem);
        }

        sleep(0);
    }
    // Print statistics for peer
    printf("PID %d  |  Entry %d  |  Reads %d  |  Writes %d  |  Avg Wait  %.3fs\n",
        pid, entry, reads, writes, waitTime / reps);

    if (shmdt(shmPtr) == -1) {
        printf("PID %d Error: Shared memory detach\n", pid);
        exit(1);
    } else if(shmctl(shmID, IPC_RMID, 0) == -1) {
        printf("PID %d Error: Shared memory control\n", pid);
        exit(1);
    }

    exit(0);
}