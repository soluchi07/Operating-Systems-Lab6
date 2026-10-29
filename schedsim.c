// C program for implementation of Simulation (with arrival times)
#include<stdio.h> 
#include<limits.h>
#include<stdlib.h>
#include<string.h>
#include "process.h"
#include "util.h"

// qsort comparer: earlier arrival first; ties broken by pid (FCFS order)
int cmp_arrival(const void *this, const void *that)
{
    const ProcessType *a = (const ProcessType *)this;
    const ProcessType *b = (const ProcessType *)that;

    if (a->art != b->art) return (a->art < b->art) ? -1 : 1;
    if (a->pid != b->pid) return (a->pid < b->pid) ? -1 : 1;
    return 0;
}

// qsort comparer: higher priority number first; ties broken by arrival, then pid
int my_comparer(const void *this, const void *that)
{ 
    const ProcessType *a = (const ProcessType *)this;
    const ProcessType *b = (const ProcessType *)that;

    if (a->pri != b->pri) return (a->pri < b->pri) ? 1 : -1;
    return cmp_arrival(a, b);
} 

// FCFS: plist must already be sorted by arrival time.
// If the CPU is idle when the next process arrives, the clock jumps forward.
void findWaitingTime(ProcessType plist[], int n)
{ 
    int t = 0;
    for (int i = 0; i < n; i++) {
        if (t < plist[i].art) t = plist[i].art;   // idle gap
        plist[i].wt = t - plist[i].art;
        t += plist[i].bt;
    }
} 

// Priority (non-preemptive): plist must already be sorted by priority.
// At each decision point run the best-priority process that has arrived.
void findWaitingTimePriority(ProcessType plist[], int n)
{
    char *done = calloc(n, sizeof(char));
    int finished = 0;
    int t = 0;

    while (finished < n) {
        int idx = -1;
        int next_arrival = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (done[i]) continue;
            if (plist[i].art <= t) { idx = i; break; }   // first in priority order
            if (plist[i].art < next_arrival) next_arrival = plist[i].art;
        }

        if (idx == -1) {            // nobody has arrived: jump to next arrival
            t = next_arrival;
            continue;
        }

        plist[idx].wt = t - plist[idx].art;
        t += plist[idx].bt;
        done[idx] = 1;
        finished++;
    }

    free(done);
}

// Round Robin with arrival times: plist must be sorted by arrival time.
// Uses a ready queue; a process that arrives during a time slice is queued
// ahead of the process that was just preempted.
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) 
{ 
    int *rem_bt = malloc(n * sizeof(int));
    int *queue  = malloc(n * sizeof(int));     // each process is queued at most once at a time
    int head = 0, count = 0;
    int next = 0;                              // next process (by arrival) not yet queued
    int finished = 0;
    int t = 0;

    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    while (finished < n) {
        // admit everything that has arrived by now
        while (next < n && plist[next].art <= t) {
            queue[(head + count) % n] = next++;
            count++;
        }

        if (count == 0) {                      // idle: jump to next arrival
            t = plist[next].art;
            continue;
        }

        int i = queue[head];
        head = (head + 1) % n;
        count--;

        int run = (rem_bt[i] > quantum) ? quantum : rem_bt[i];
        t += run;
        rem_bt[i] -= run;

        // processes that arrived during this slice go in first
        while (next < n && plist[next].art <= t) {
            queue[(head + count) % n] = next++;
            count++;
        }

        if (rem_bt[i] > 0) {
            queue[(head + count) % n] = i;     // back of the line
            count++;
        } else {
            plist[i].wt = t - plist[i].art - plist[i].bt;
            finished++;
        }
    }

    free(rem_bt);
    free(queue);
} 

// Shortest Remaining Time First (preemptive SJF)
void findWaitingTimeSJF(ProcessType plist[], int n)
{ 
    int *rem_bt = malloc(n * sizeof(int));
    int completed = 0;
    int t = 0;

    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
        if (rem_bt[i] <= 0) completed++;       // zero-length jobs are already done
    }

    while (completed < n) {
        int idx = -1, min = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (plist[i].art <= t && rem_bt[i] > 0 && rem_bt[i] < min) {
                min = rem_bt[i];
                idx = i;
            }
        }

        if (idx == -1) {                       // CPU idle
            t++;
            continue;
        }

        rem_bt[idx]--;
        t++;

        if (rem_bt[idx] == 0) {
            completed++;
            plist[idx].wt = t - plist[idx].art - plist[idx].bt;
        }
    }

    free(rem_bt);
} 

// Function to calculate turn around time 
void findTurnAroundTime( ProcessType plist[], int n)
{ 
    for (int  i = 0; i < n ; i++) 
        plist[i].tat = plist[i].bt + plist[i].wt; 
} 

void findavgTimeFCFS( ProcessType plist[], int n) 
{ 
    qsort(plist, n, sizeof(ProcessType), cmp_arrival);
    findWaitingTime(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nFCFS\n");
}

void findavgTimeSJF( ProcessType plist[], int n) 
{ 
    findWaitingTimeSJF(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nSJF\n");
}

void findavgTimeRR( ProcessType plist[], int n, int quantum) 
{ 
    qsort(plist, n, sizeof(ProcessType), cmp_arrival);
    findWaitingTimeRR(plist, n, quantum); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

void findavgTimePriority( ProcessType plist[], int n) 
{ 
    // 1- Sort the processes according to priority
    qsort(plist, n, sizeof(ProcessType), my_comparer);

    // 2- Apply FCFS-style scheduling to the sorted list (respecting arrivals)
    findWaitingTimePriority(plist, n);
    findTurnAroundTime(plist, n);
  
    printf("\n*********\nPriority\n");
}

void printMetrics(ProcessType plist[], int n)
{
    int total_wt = 0, total_tat = 0; 
    float awt, att;
    
    printf("\tProcesses\tBurst time\tArrival time\tWaiting time\tTurn around time\n"); 
  
    for (int  i=0; i<n; i++) 
    { 
        total_wt = total_wt + plist[i].wt; 
        total_tat = total_tat + plist[i].tat; 
        printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", plist[i].pid, plist[i].bt, plist[i].art, plist[i].wt, plist[i].tat); 
    } 
  
    awt = ((float)total_wt / (float)n);
    att = ((float)total_tat / (float)n);
    
    printf("\nAverage waiting time = %.2f", awt); 
    printf("\nAverage turn around time = %.2f\n", att); 
} 

ProcessType * initProc(char *filename, int *n) 
{
    FILE *input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        fflush(stdout);
        exit(0);
    }

    ProcessType *plist = parse_file(input_file, n);
  
    fclose(input_file);
  
    return plist;
}

// Give each algorithm a fresh copy of the parsed processes
static ProcessType *copyProcs(const ProcessType *orig, int n)
{
    ProcessType *copy = malloc(n * sizeof(ProcessType));
    if (!copy) {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }
    memcpy(copy, orig, n * sizeof(ProcessType));
    return copy;
}
  
// Driver code 
int main(int argc, char *argv[]) 
{ 
    int n = 0; 
    int quantum = 2;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./schedsim <input-file-path> [quantum]\n");
        fflush(stdout);
        return 1;
    }

    if (argc >= 3) {
        quantum = atoi(argv[2]);
        if (quantum < 1) {
            fprintf(stderr, "Error: quantum must be a positive integer\n");
            return 1;
        }
    }

    ProcessType *orig = initProc(argv[1], &n);
    if (n <= 0) {
        fprintf(stderr, "Error: no processes found in input file\n");
        free(orig);
        return 1;
    }

    ProcessType *proc_list;

    // FCFS
    proc_list = copyProcs(orig, n);
    findavgTimeFCFS(proc_list, n);
    printMetrics(proc_list, n);
    free(proc_list);

    // SJF
    proc_list = copyProcs(orig, n);
    findavgTimeSJF(proc_list, n); 
    printMetrics(proc_list, n);
    free(proc_list);

    // Priority
    proc_list = copyProcs(orig, n);
    findavgTimePriority(proc_list, n); 
    printMetrics(proc_list, n);
    free(proc_list);

    // RR
    proc_list = copyProcs(orig, n);
    findavgTimeRR(proc_list, n, quantum); 
    printMetrics(proc_list, n);
    free(proc_list);

    free(orig);
    return 0; 
}