//
// Virual Memory Simulator Homework
// One-level page table system with FIFO and LRU
// Two-level page table system with LRU
// Inverted page table with a hashing system
// Submission Year: 2020
// Student Name: 김준홍
// Student Number: B611047
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define PAGESIZEBITS 12    // page size = 4Kbytes
#define VIRTUALADDRBITS 32 // virtual address space size = 4Gbytes
#define TOTALTRACE 1000000

int numProcess = 1;
int nFrame = 1;

struct procEntry {
    char *traceName;          // the memory trace name
    int pid;                  // process (trace) id
    int ntraces;              // the number of memory traces
    int num2ndLevelPageTable; // The 2nd level page created(allocated);
    int numIHTConflictAccess; // The number of Inverted Hash Table Conflict Accesses
    int numIHTNULLAccess;     // The number of Empty Inverted Hash Table Accesses
    int numIHTNonNULLAcess;   // The number of Non Empty Inverted Hash Table Accesses
    int numPageFault;         // The number of page faults
    int numPageHit;           // The number of page hits
    struct pageTableEntry *firstLevelPageTable;
    FILE *tracefp;
};
struct pageTableEntry {
    unsigned addr; //vpn
    int pid;
    char rw;                     //read or write
    struct pageTableEntry *next; //make linked list
};
void oneLevelVMSim(struct procEntry *procTable, int type) {
    int i;
    int fc = 1;
    struct pageTableEntry *first = malloc(sizeof(struct pageTableEntry));
    unsigned addrTemp;
    char rwTemp;
    fscanf(procTable[0].tracefp, "%x %c", &addrTemp, &rwTemp);
    addrTemp = addrTemp / 4096;
    first->pid = 0;
    first->rw = rwTemp;
    first->addr = addrTemp;
    first->next = first;
    procTable[0].numPageFault++;
    procTable[0].ntraces++;
    struct pageTableEntry *tail = first;
    if (type == 0) {
        for (i = 1; i < numProcess; i++) {
            struct pageTableEntry *newpage = malloc(sizeof(struct pageTableEntry));
            fscanf(procTable[i].tracefp, "%x %c", &addrTemp, &rwTemp);
            addrTemp = addrTemp / 4096;
            newpage->addr = addrTemp;
            newpage->rw = rwTemp;
            newpage->pid = i;
            procTable[i].numPageFault++;
            procTable[i].ntraces++;
            tail->next = newpage;
            newpage->next = first;
            tail = newpage;
            if (fc == nFrame) {
                struct pageTableEntry *temppge = first;
                first = first->next;

                free(temppge);
            } else {

                fc++;
            }
        }
        while (!feof(procTable[numProcess - 1].tracefp)) {
            for (i = 0; i < numProcess; i++) {

                fscanf(procTable[i].tracefp, "%x %c", &addrTemp, &rwTemp);

                addrTemp = addrTemp / 4096;

                struct pageTableEntry *curr = first;
                while (1) {
                    if (feof(procTable[i].tracefp)) {
                        break;
                    }
                    if (curr->addr == addrTemp && curr->pid == i) {
                        procTable[i].numPageHit++;
                        procTable[i].ntraces++;
                        break;
                    }
                    if (curr == tail) {
                        struct pageTableEntry *newpage = malloc(sizeof(struct pageTableEntry));
                        newpage->addr = addrTemp;
                        newpage->rw = rwTemp;
                        newpage->pid = i;
                        procTable[i].numPageFault++;
                        tail->next = newpage;
                        newpage->next = first;
                        tail = newpage;
                        procTable[i].ntraces++;
                        if (fc == nFrame) {
                            struct pageTableEntry *temppge = first;
                            first = first->next;
                            free(temppge);
                        } else {
                            fc++;
                        }

                        break;
                    }
                    curr = curr->next;
                }
            }
        }
    }

    if (type == 1) { //LRU
        for (i = 0; i < numProcess; i++) {
        }
    }
    for (i = 0; i < numProcess; i++) {
        printf("**** %s *****\n", procTable[i].traceName);
        printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
        printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
        printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
        fclose(procTable[i].tracefp);
        //assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
    }
}
void twoLevelVMSim(struct procEntry *procTable) {
    int i;
    for (i = 0; i < numProcess; i++) {
        printf("**** %s *****\n", procTable[i].traceName);
        printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
        printf("Proc %d Num of second level page tables allocated %d\n", i, procTable[i].num2ndLevelPageTable);
        printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
        printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
        assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
    }
}

void invertedPageVMSim(struct procEntry *procTable) {
    int i;

    for (i = 0; i < numProcess; i++) {
        printf("**** %s *****\n", procTable[i].traceName);
        printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
        printf("Proc %d Num of Inverted Hash Table Access Conflicts %d\n", i, procTable[i].numIHTConflictAccess);
        printf("Proc %d Num of Empty Inverted Hash Table Access %d\n", i, procTable[i].numIHTNULLAccess);
        printf("Proc %d Num of Non-Empty Inverted Hash Table Access %d\n", i, procTable[i].numIHTNonNULLAcess);
        printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
        printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
        assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
        assert(procTable[i].numIHTNULLAccess + procTable[i].numIHTNonNULLAcess == procTable[i].ntraces);
    }
}

int main(int argc, char *argv[]) {
    int i, c, simType;
    struct procEntry *procTable = malloc(sizeof(struct procEntry) * argc);
    int mem = atoi(argv[3]);
    int check = 1;
    for (i = 0; i < mem - 12; i++) {
        nFrame *= 2;
        check *= 2;
    }
    numProcess = argc - 4;
    // initialize procTable for Memory Simulations
    for (i = 0; i < numProcess; i++) {

        // opening a tracefile for the process
        printf("process %d opening %s\n", i, argv[i + 4]);
        procTable[i].tracefp = fopen(argv[i + 4], "r");
        if (procTable[i].tracefp == NULL) {
            printf("ERROR: can't open %s file; exiting...", argv[i + 1]);
            exit(1);
        }
        procTable[i].traceName = argv[i + 4];
        procTable[i].numPageFault = 0;
        procTable[i].numPageHit = 0;
        procTable[i].ntraces = 0;
        procTable[i].pid = i;
    }
    long int nF = nFrame * 4096;

    printf("Num of Frames %d Physical Memory Size %ld bytes\n", nFrame, nF);

    if (simType == 0) {
        printf("=============================================================\n");
        printf("The One-Level Page Table with FIFO Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        oneLevelVMSim(procTable, simType);
    }

    if (simType == 1) {
        printf("=============================================================\n");
        printf("The One-Level Page Table with LRU Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        oneLevelVMSim(procTable, simType);
    }

    if (simType == 2) {
        printf("=============================================================\n");
        printf("The Two-Level Page Table Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        twoLevelVMSim(procTable);
    }

    if (simType == 3) {
        printf("=============================================================\n");
        printf("The Inverted Page Table Memory Simulation Starts .....\n");
        printf("=============================================================\n");
        invertedPageVMSim(procTable);
    }

    return (0);
}
