// ============================================================
// Task D — Time-Sliced Multilevel Queue (MLQ) Scheduler
// ============================================================
// PROBLEM: Simulate a CPU scheduler with 3 queues, each using
// a different algorithm and receiving a fixed % of CPU time:
//   Q1 (System):      50% CPU — Round Robin (quantum=2)
//   Q2 (Interactive): 30% CPU — Shortest Job First
//   Q3 (Batch):       20% CPU — First Come First Served
//
// Processes are assigned to queues by priority range:
//   Priority 0–10  → Q1
//   Priority 11–20 → Q2
//   Priority 21–30 → Q3
//
// Compile: g++ -o mlq_scheduler mlq_scheduler.cpp
// Run:     ./mlq_scheduler
// ============================================================

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <iomanip>
using namespace std;

// ----------------------------------------------------------
// PROCESS STRUCT — includes all fields from the assignment
// ----------------------------------------------------------
struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;       // 0-10=Q1, 11-20=Q2, 21-30=Q3
    int start_time;     // First time process gets CPU (-1 = not yet)
    int finish_time;    // When process completed (-1 = not yet)
    bool activated;     // Has been added to its queue?
};

// Determine queue level from priority range
int get_queue(int priority) {
    if (priority <= 10) return 1;
    if (priority <= 20) return 2;
    return 3;
}

int main() {
    cout << "=== Time-Sliced MLQ Scheduler ===" << endl;
    cout << "Q1 (Round Robin, quantum=2):   5 ticks/cycle (50%)" << endl;
    cout << "Q2 (Shortest Job First):       3 ticks/cycle (30%)" << endl;
    cout << "Q3 (First Come First Served):  2 ticks/cycle (20%)" << endl;
    cout << endl;

    // ======================================================
    // STEP 1: Create processes with arrival times & priorities
    // ======================================================
    // {pid, arrival, burst, remaining, priority, start, finish, activated}
    vector<Process> procs = {
        {1, 0, 5, 5, 5,  -1, -1, false},  // P1 → Q1
        {2, 1, 3, 3, 8,  -1, -1, false},  // P2 → Q1
        {3, 0, 6, 6, 15, -1, -1, false},  // P3 → Q2
        {4, 2, 2, 2, 12, -1, -1, false},  // P4 → Q2
        {5, 0, 4, 4, 25, -1, -1, false},  // P5 → Q3
        {6, 3, 3, 3, 22, -1, -1, false},  // P6 → Q3
    };

    int n = procs.size();

    // Print the process table
    cout << "Processes:" << endl;
    cout << "  PID  Priority  Queue  Arrival  Burst" << endl;
    cout << "  ---  --------  -----  -------  -----" << endl;
    for (auto &p : procs) {
        cout << "   " << p.pid
             << "      " << setw(2) << p.priority
             << "      Q" << get_queue(p.priority)
             << "       " << p.arrival_time
             << "       " << p.burst_time << endl;
    }
    cout << endl;

    // ======================================================
    // STEP 2: Set up separate queues (using deques)
    // ======================================================
    // Each deque stores INDICES into the procs vector.
    deque<int> q1, q2, q3;

    // Gantt chart: stores the PID that ran at each tick (0 = idle)
    vector<int> gantt;

    int current_time = 0;
    int completed = 0;

    // --- Helper: activate processes that have arrived by current_time ---
    auto activate_arrivals = [&]() {
        for (int i = 0; i < n; i++) {
            if (!procs[i].activated && procs[i].arrival_time <= current_time) {
                procs[i].activated = true;
                int ql = get_queue(procs[i].priority);
                if (ql == 1) q1.push_back(i);
                else if (ql == 2) q2.push_back(i);
                else q3.push_back(i);
            }
        }
    };

    // --- Helper: execute one tick for a process ---
    auto run_one_tick = [&](int idx) {
        if (procs[idx].start_time == -1)
            procs[idx].start_time = current_time;
        procs[idx].remaining_time--;
        gantt.push_back(procs[idx].pid);
        current_time++;
        activate_arrivals();
    };

    // Initial activation at time 0
    activate_arrivals();

    cout << "=== Execution Timeline ===" << endl;

    // ======================================================
    // STEP 3: The Big Scheduling Loop
    // ======================================================
    // Cycles: Q1 (5 ticks) → Q2 (3 ticks) → Q3 (2 ticks) → repeat
    while (completed < n) {
        bool any_work = false;

        // --------------------------------------------------
        // QUEUE 1: Round Robin (Budget=5, Quantum=2)
        // --------------------------------------------------
        // Each process gets at most 2 ticks (quantum), then
        // goes to back of queue. Budget limits total Q1 time.
        {
            int budget = 5;
            while (budget > 0 && !q1.empty()) {
                any_work = true;
                int idx = q1.front();
                q1.pop_front();

                // Run for min(quantum, budget, remaining_time) ticks
                int ticks = min({2, budget, procs[idx].remaining_time});
                for (int i = 0; i < ticks; i++) {
                    cout << "  [t=" << current_time << "] Q1 (RR)   -> P"
                         << procs[idx].pid << endl;
                    run_one_tick(idx);
                    budget--;
                }

                if (procs[idx].remaining_time == 0) {
                    procs[idx].finish_time = current_time;
                    completed++;
                    cout << "  *** P" << procs[idx].pid << " FINISHED ***" << endl;
                } else {
                    q1.push_back(idx);  // Back of queue (Round Robin)
                }
            }
        }

        // --------------------------------------------------
        // QUEUE 2: Shortest Job First (Budget=3)
        // --------------------------------------------------
        // Always pick the process with the smallest remaining
        // time. Run it until budget runs out or it finishes.
        {
            int budget = 3;
            while (budget > 0 && !q2.empty()) {
                any_work = true;

                // Find the process with shortest remaining time
                int si = 0;
                for (int i = 1; i < (int)q2.size(); i++) {
                    if (procs[q2[i]].remaining_time < procs[q2[si]].remaining_time)
                        si = i;
                }
                int idx = q2[si];
                q2.erase(q2.begin() + si);

                int ticks = min(budget, procs[idx].remaining_time);
                for (int i = 0; i < ticks; i++) {
                    cout << "  [t=" << current_time << "] Q2 (SJF)  -> P"
                         << procs[idx].pid << endl;
                    run_one_tick(idx);
                    budget--;
                }

                if (procs[idx].remaining_time == 0) {
                    procs[idx].finish_time = current_time;
                    completed++;
                    cout << "  *** P" << procs[idx].pid << " FINISHED ***" << endl;
                } else {
                    q2.push_back(idx);
                }
            }
        }

        // --------------------------------------------------
        // QUEUE 3: First Come First Served (Budget=2)
        // --------------------------------------------------
        // Run the first process in queue until it finishes
        // or budget runs out.
        {
            int budget = 2;
            while (budget > 0 && !q3.empty()) {
                any_work = true;
                int idx = q3.front();
                q3.pop_front();

                int ticks = min(budget, procs[idx].remaining_time);
                for (int i = 0; i < ticks; i++) {
                    cout << "  [t=" << current_time << "] Q3 (FCFS) -> P"
                         << procs[idx].pid << endl;
                    run_one_tick(idx);
                    budget--;
                }

                if (procs[idx].remaining_time == 0) {
                    procs[idx].finish_time = current_time;
                    completed++;
                    cout << "  *** P" << procs[idx].pid << " FINISHED ***" << endl;
                } else {
                    q3.push_front(idx);  // FCFS: keep at front
                }
            }
        }

        // If all queues are empty but processes remain, advance time (idle)
        if (!any_work && completed < n) {
            cout << "  [t=" << current_time << "] IDLE" << endl;
            gantt.push_back(0);
            current_time++;
            activate_arrivals();
        }
    }

    // ======================================================
    // STEP 4: Print Gantt Chart
    // ======================================================
    cout << endl << "=== Gantt Chart ===" << endl << "  |";
    for (int i = 0; i < (int)gantt.size(); i++) {
        if (gantt[i] == 0) cout << " -- |";
        else cout << " P" << gantt[i] << " |";
    }
    cout << endl << "  ";
    for (int i = 0; i <= (int)gantt.size(); i++) {
        cout << setw(4) << i << " ";
    }
    cout << endl;

    // ======================================================
    // STEP 5: Print Performance Metrics
    // ======================================================
    cout << endl << "=== Performance Metrics ===" << endl;
    cout << "  PID  Queue  Arrival  Burst  Finish  Waiting  Turnaround  Response" << endl;
    cout << "  ---  -----  -------  -----  ------  -------  ----------  --------" << endl;

    double total_wt = 0, total_tat = 0, total_rt = 0;

    for (auto &p : procs) {
        int turnaround = p.finish_time - p.arrival_time;
        int waiting     = turnaround - p.burst_time;
        int response    = p.start_time - p.arrival_time;

        total_wt  += waiting;
        total_tat += turnaround;
        total_rt  += response;

        cout << "   " << p.pid
             << "     Q" << get_queue(p.priority)
             << "       " << p.arrival_time
             << "       " << p.burst_time
             << "      " << setw(2) << p.finish_time
             << "       " << setw(2) << waiting
             << "          " << setw(2) << turnaround
             << "         " << response << endl;
    }

    cout << endl;
    cout << "  Avg Waiting Time:    " << fixed << setprecision(2) << total_wt / n << endl;
    cout << "  Avg Turnaround Time: " << fixed << setprecision(2) << total_tat / n << endl;
    cout << "  Avg Response Time:   " << fixed << setprecision(2) << total_rt / n << endl;

    cout << endl << "All " << n << " processes finished at time t=" << current_time << endl;
    return 0;
}
