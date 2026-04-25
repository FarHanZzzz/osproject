// Task D — Time-Sliced Multilevel Queue (MLQ) Scheduler
// 3 queues with different scheduling algorithms:
//   Q1 (priority 0-10):  Round Robin (quantum=2), gets 5 ticks per cycle
//   Q2 (priority 11-20): Shortest Job First, gets 3 ticks per cycle
//   Q3 (priority 21-30): First Come First Served, gets 2 ticks per cycle
//
// Compile: g++ -o mlq_scheduler mlq_scheduler.cpp
// Run:     ./mlq_scheduler

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
using namespace std;

struct Process {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int priority;       // 0-10=Q1, 11-20=Q2, 21-30=Q3
    int start_time;     // first time it got CPU (-1 = hasn't run yet)
    int completion;
    int turnaround;
    int waiting;
    int response;
};

// Global state
vector<Process> procs;
int now = 0;  // current time

// Which queue does this process belong to?
int get_queue(int priority) {
    if (priority <= 10) return 1;
    if (priority <= 20) return 2;
    return 3;
}

// Check if all processes are done
bool all_done() {
    for (auto &p : procs)
        if (p.remaining > 0) return false;
    return true;
}

// Run one tick of a process
void run_one_tick(Process *p, string qname) {
    if (p->start_time == -1) p->start_time = now;
    p->remaining--;
    cout << "  [t=" << now << "] P" << p->pid << " (" << qname << ")\n";
    now++;

    if (p->remaining == 0) {
        p->completion  = now;
        p->turnaround  = p->completion - p->arrival;
        p->waiting     = p->turnaround - p->burst;
        p->response    = p->start_time - p->arrival;
    }
}

// Run idle for one tick
void run_idle() {
    cout << "  [t=" << now << "] IDLE\n";
    now++;
}

// ---- Q1: Round Robin (quantum = 2) ----
void run_Q1(int budget) {
    int spent = 0;
    deque<Process*> ready;

    while (spent < budget) {
        // Add newly arrived Q1 processes to ready queue
        for (auto &p : procs) {
            if (p.arrival <= now && p.remaining > 0 && get_queue(p.priority) == 1) {
                bool already = false;
                for (auto *r : ready) if (r->pid == p.pid) already = true;
                if (!already) ready.push_back(&p);
            }
        }

        if (ready.empty()) {
            run_idle();
            spent++;
            continue;
        }

        Process *p = ready.front();
        ready.pop_front();

        // Run for min(quantum=2, remaining time, remaining budget)
        int run_for = min({2, p->remaining, budget - spent});
        for (int i = 0; i < run_for; i++) {
            run_one_tick(p, "Q1-RR");
            spent++;
            // Check for new arrivals after each tick
            for (auto &proc : procs) {
                if (proc.arrival <= now && proc.remaining > 0 && get_queue(proc.priority) == 1) {
                    bool already = false;
                    for (auto *r : ready) if (r->pid == proc.pid) already = true;
                    if (proc.pid != p->pid && !already) ready.push_back(&proc);
                }
            }
        }

        // If process isn't done, put it at the back (Round Robin!)
        if (p->remaining > 0)
            ready.push_back(p);
    }
}

// ---- Q2: Shortest Job First (non-preemptive) ----
void run_Q2(int budget) {
    int spent = 0;

    while (spent < budget) {
        // Find all arrived Q2 processes
        vector<Process*> ready;
        for (auto &p : procs) {
            if (p.arrival <= now && p.remaining > 0 && get_queue(p.priority) == 2)
                ready.push_back(&p);
        }

        if (ready.empty()) {
            run_idle();
            spent++;
            continue;
        }

        // Sort by remaining time (shortest first)
        sort(ready.begin(), ready.end(),
             [](Process *a, Process *b) { return a->remaining < b->remaining; });

        Process *p = ready[0];
        int run_for = min(p->remaining, budget - spent);
        for (int i = 0; i < run_for; i++) {
            run_one_tick(p, "Q2-SJF");
            spent++;
        }
    }
}

// ---- Q3: First Come First Served ----
void run_Q3(int budget) {
    int spent = 0;

    while (spent < budget) {
        // Find the earliest-arrived Q3 process
        Process *earliest = nullptr;
        for (auto &p : procs) {
            if (p.arrival <= now && p.remaining > 0 && get_queue(p.priority) == 3) {
                if (!earliest || p.arrival < earliest->arrival)
                    earliest = &p;
            }
        }

        if (!earliest) {
            run_idle();
            spent++;
            continue;
        }

        int run_for = min(earliest->remaining, budget - spent);
        for (int i = 0; i < run_for; i++) {
            run_one_tick(earliest, "Q3-FCFS");
            spent++;
        }
    }
}

int main() {
    cout << "=== MLQ Scheduler ===" << endl;
    cout << "Q1 (System,      prio 0-10):  Round Robin, 50% CPU" << endl;
    cout << "Q2 (Interactive, prio 11-20): SJF,         30% CPU" << endl;
    cout << "Q3 (Batch,       prio 21-30): FCFS,        20% CPU" << endl << endl;

    // 9 test processes across all 3 queues
    //           pid  arr  burst  rem  prio  start compl turn wait resp
    procs = {
        {1,  0, 6, 6,  5,  -1, 0, 0, 0, 0},   // Q1
        {2,  1, 4, 4,  3,  -1, 0, 0, 0, 0},   // Q1
        {3,  2, 8, 8, 15,  -1, 0, 0, 0, 0},   // Q2
        {4,  0, 5, 5, 18,  -1, 0, 0, 0, 0},   // Q2
        {5,  3, 3, 3, 25,  -1, 0, 0, 0, 0},   // Q3
        {6,  1, 7, 7, 28,  -1, 0, 0, 0, 0},   // Q3
        {7,  4, 2, 2,  8,  -1, 0, 0, 0, 0},   // Q1
        {8,  2, 6, 6, 12,  -1, 0, 0, 0, 0},   // Q2
        {9,  5, 4, 4, 22,  -1, 0, 0, 0, 0},   // Q3
    };

    // Print input table
    cout << "Input Processes:" << endl;
    cout << "  PID  Arrival  Burst  Priority  Queue" << endl;
    cout << "  ---  -------  -----  --------  -----" << endl;
    for (auto &p : procs) {
        string qname = (get_queue(p.priority) == 1) ? "Q1" :
                        (get_queue(p.priority) == 2) ? "Q2" : "Q3";
        cout << "   " << p.pid << "      " << p.arrival << "      " << p.burst
             << "       " << p.priority << "      " << qname << endl;
    }

    // Run the scheduler: cycle through Q1(5), Q2(3), Q3(2) until all done
    cout << "\n=== Execution Timeline ===" << endl;
    while (!all_done() && now < 200) {
        run_Q1(5);
        if (all_done()) break;
        run_Q2(3);
        if (all_done()) break;
        run_Q3(2);
    }

    // Print results
    cout << "\n=== Results ===" << endl;
    cout << "  PID  Arrival  Burst  Completion  Turnaround  Waiting  Response" << endl;
    cout << "  ---  -------  -----  ----------  ----------  -------  --------" << endl;

    double total_tat = 0, total_wt = 0, total_rt = 0;
    for (auto &p : procs) {
        cout << "   " << p.pid
             << "      " << p.arrival
             << "      " << p.burst
             << "        " << p.completion
             << "          " << p.turnaround
             << "         " << p.waiting
             << "        " << p.response << endl;
        total_tat += p.turnaround;
        total_wt  += p.waiting;
        total_rt  += p.response;
    }

    int n = procs.size();
    cout << "\nAverages:" << endl;
    cout << "  Turnaround: " << total_tat / n << endl;
    cout << "  Waiting:    " << total_wt / n << endl;
    cout << "  Response:   " << total_rt / n << endl;

    return 0;
}
