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
    unsigned addr;               //vpn
    char rw;                     //read or write
    struct pageTableEntry *next; //make linked list
};
void oneLevelVMSim(struct procEntry *procTable, int type) {
    int i;
    int frameCounter = 1;
    struct pageTableEntry **first = malloc(sizeof(struct pageTableEntry) * numProcess);
    int *fc = malloc(sizeof(int) * numProcess);

    for (i = 0; i < numProcess; i++) {
        first[i]->next = first[i];
        fscanf(procTable[i].tracefp, "%x %c", &first[i]->addr, &first[i]->rw);
        fc[i] = 1;
    }
    if (type == 0) {                                       //FIFO
        while (!feof(procTable[numProcess - 1].tracefp)) { //process n 다읽으면
            for (i = 0; i < numProcess; i++) {
                unsigned addrTemp;
                char rwTemp;
                fscanf(procTable[i].tracefp, "%x %c", &addrTemp, &rwTemp);

                procTable[i].ntraces++; //trace 횟수 증가

                struct pageTableEntry *curr = first[i];

                while (1) {                       //hit 했나 검사
                    if (curr->addr == addrTemp) { //hit!!
                        procTable[i].numPageHit++;
                        break;
                    }

                    if (curr->next == first[i]) { //page fault (새로 꺼내야함)

                        procTable[i].numPageFault++;

                        if (fc[i] == nFrame) { //자리 비켜줘야함
                            struct pageTableEntry *dummy = first[i];
                            struct pageTableEntry *newPage = malloc(sizeof(struct pageTableEntry));
                            first[i] = first[i]->next;
                            free(dummy); //first trace 제거

                            curr->next = newPage;
                            newPage->next = first[i];
                            newPage->addr = addrTemp;
                            newPage->rw = rwTemp;
                            break;
                        } else { //뒤에만 껴줄때
                            fc[i]++;
                            struct pageTableEntry *newPage = malloc(sizeof(struct pageTableEntry));
                            curr->next = newPage;
                            newPage->next = first[i];
                            newPage->addr = addrTemp;
                            newPage->rw = rwTemp;
                            break;
                        }
                    }
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
        assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
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
    printf("0번째 입니다.");
    int mem = *argv[2];

    for (i = 0; i < mem - 32; i++) {
        nFrame *= 2;
    }

    // initialize procTable for Memory Simulations
    for (i = 0; i < numProcess; i++) {

        // opening a tracefile for the process
        printf("process %d opening %s\n", i, argv[i + 3]);
        procTable[i].tracefp = fopen(argv[i + 3], "r");
        if (procTable[i].tracefp == NULL) {
            printf("ERROR: can't open %s file; exiting...", argv[i + 3]);
            exit(1);
        }
        procTable[i].traceName = argv[i + 3];
        procTable[i].numPageFault = 0;
        procTable[i].numPageHit = 0;
        procTable[i].ntraces = 0;
    }

    printf("Num of Frames %d Physical Memory Size %ld bytes\n", nFrame, (1L << nFrame * 4096));

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
