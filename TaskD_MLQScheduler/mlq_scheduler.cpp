// ============================================================
// Task D — Time-Sliced Multilevel Queue (MLQ) Scheduler
// ============================================================
// PROBLEM (from PDF): Simulate a CPU scheduler that uses 3 queues
// with different scheduling algorithms:
//   Q1: Round Robin      — gets 5 CPU ticks per cycle (50% CPU)
//   Q2: Shortest Job First — gets 3 CPU ticks per cycle (30% CPU)
//   Q3: First Come First Served — gets 2 CPU ticks per cycle (20% CPU)
//
// The scheduler cycles through Q1 → Q2 → Q3 → Q1 → Q2 → Q3 ...
// until all processes are finished.
//
// HOW WE SOLVE IT:
//   1. Create hardcoded processes assigned to specific queues.
//   2. Run a big loop that gives Q1 five ticks, Q2 three ticks,
//      Q3 two ticks, and repeats.
//   3. Q1 uses Round Robin: each process gets 1 tick, then we
//      move to the next (like kids sharing a game controller).
//   4. Q2 uses SJF: we always pick the process with the least
//      remaining time.
//   5. Q3 uses FCFS: we just run processes in the order they appear.
//
// Compile: g++ -o mlq_scheduler mlq_scheduler.cpp
// Run:     ./mlq_scheduler
// ============================================================

#include <iostream>  // For cout (printing to screen)
#include <vector>    // For vector (a resizable array)
using namespace std; // So we can write "cout" instead of "std::cout"

// ----------------------------------------------------------
// PROCESS STRUCT
// ----------------------------------------------------------
// Each process has an ID, which queue it belongs to,
// how much total time it needs (burst), and how much time
// is left to run.
struct Process {
    int id;           // Process ID (like a name tag: P1, P2, etc.)
    int queue_level;  // Which queue: 1 (RR), 2 (SJF), or 3 (FCFS)
    int time_needed;  // Total CPU time this process needs (burst time)
    int time_left;    // How much time is STILL remaining
};

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main() {
    cout << "=== MLQ Scheduler ===" << endl;
    cout << "Q1 (Round Robin):          5 ticks/cycle (50% CPU)" << endl;
    cout << "Q2 (Shortest Job First):   3 ticks/cycle (30% CPU)" << endl;
    cout << "Q3 (First Come First Served): 2 ticks/cycle (20% CPU)" << endl;
    cout << endl;

    // ======================================================
    // STEP 1: Create hardcoded processes
    // ======================================================
    // Format: {id, queue_level, time_needed, time_left}
    // time_left starts equal to time_needed (nothing has run yet)
    vector<Process> procs = {
        {1, 1, 5, 5},  // P1 in Queue 1, needs 5 ticks
        {2, 1, 3, 3},  // P2 in Queue 1, needs 3 ticks
        {3, 2, 6, 6},  // P3 in Queue 2, needs 6 ticks
        {4, 2, 2, 2},  // P4 in Queue 2, needs 2 ticks
        {5, 3, 4, 4},  // P5 in Queue 3, needs 4 ticks
        {6, 3, 3, 3},  // P6 in Queue 3, needs 3 ticks
    };

    // Print the initial process table
    cout << "Processes:" << endl;
    cout << "  PID  Queue  Burst" << endl;
    cout << "  ---  -----  -----" << endl;
    for (auto &p : procs) {
        cout << "   " << p.id << "     Q" << p.queue_level
             << "     " << p.time_needed << endl;
    }
    cout << endl;

    int clock_tick = 0;   // The global clock (counts total ticks elapsed)
    bool all_done = false; // Flag: are all processes finished?

    cout << "=== Execution Timeline ===" << endl;

    // ======================================================
    // STEP 2: The Big Scheduling Loop
    // ======================================================
    // We keep cycling through Q1 → Q2 → Q3 until every
    // process has time_left == 0.
    while (!all_done) {
        all_done = true;  // Assume done; prove otherwise below

        // First, check if ANY process still has work left
        for (auto &p : procs) {
            if (p.time_left > 0) {
                all_done = false;
                break;
            }
        }
        if (all_done) break;

        // --------------------------------------------------
        // QUEUE 1: Round Robin (Budget = 5 ticks)
        // --------------------------------------------------
        // Round Robin means: give each process in Q1 exactly
        // 1 tick, then move to the next. Keep cycling until
        // the budget (5 ticks) runs out.
        int q1_budget = 5;
        bool q1_has_work = true;
        while (q1_budget > 0 && q1_has_work) {
            q1_has_work = false;
            for (int i = 0; i < (int)procs.size(); i++) {
                // Only run processes that belong to Q1 and still have work
                if (procs[i].queue_level == 1 && procs[i].time_left > 0 && q1_budget > 0) {
                    q1_has_work = true;

                    // Run this process for exactly 1 tick
                    procs[i].time_left--;
                    q1_budget--;
                    clock_tick++;
                    cout << "  [t=" << clock_tick << "] Q1 (RR)   -> P"
                         << procs[i].id;
                    if (procs[i].time_left == 0)
                        cout << " [FINISHED]";
                    cout << endl;
                }
            }
        }

        // --------------------------------------------------
        // QUEUE 2: Shortest Job First (Budget = 3 ticks)
        // --------------------------------------------------
        // SJF means: always pick the process with the
        // smallest time_left and run it for 1 tick.
        int q2_budget = 3;
        while (q2_budget > 0) {
            // Find the process with the shortest remaining time in Q2
            int shortest_idx = -1;
            int shortest_time = 999999;

            for (int i = 0; i < (int)procs.size(); i++) {
                if (procs[i].queue_level == 2 && procs[i].time_left > 0) {
                    if (procs[i].time_left < shortest_time) {
                        shortest_time = procs[i].time_left;
                        shortest_idx = i;
                    }
                }
            }

            // If no Q2 process is ready, skip the remaining budget
            if (shortest_idx == -1) break;

            // Run the shortest job for 1 tick
            procs[shortest_idx].time_left--;
            q2_budget--;
            clock_tick++;
            cout << "  [t=" << clock_tick << "] Q2 (SJF)  -> P"
                 << procs[shortest_idx].id;
            if (procs[shortest_idx].time_left == 0)
                cout << " [FINISHED]";
            cout << endl;
        }

        // --------------------------------------------------
        // QUEUE 3: First Come First Served (Budget = 2 ticks)
        // --------------------------------------------------
        // FCFS means: run the FIRST process we find that still
        // has work. Stick with it until budget runs out or it finishes.
        int q3_budget = 2;
        for (int i = 0; i < (int)procs.size() && q3_budget > 0; i++) {
            // Find the first Q3 process with remaining work
            while (procs[i].queue_level == 3 && procs[i].time_left > 0 && q3_budget > 0) {
                procs[i].time_left--;
                q3_budget--;
                clock_tick++;
                cout << "  [t=" << clock_tick << "] Q3 (FCFS) -> P"
                     << procs[i].id;
                if (procs[i].time_left == 0)
                    cout << " [FINISHED]";
                cout << endl;
            }
        }
    }

    // ======================================================
    // STEP 3: Print final summary
    // ======================================================
    cout << "\n=== Summary ===" << endl;
    cout << "All processes finished at time t=" << clock_tick << endl;

    return 0;  // Program finished successfully
}
