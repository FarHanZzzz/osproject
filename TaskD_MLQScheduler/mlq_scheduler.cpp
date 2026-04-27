// ============================================================
// Task D — Time-Sliced Multilevel Queue (MLQ) Scheduler
// ============================================================
//
// THE PROBLEM:
//   Real operating systems have many types of processes running
//   at once: system processes (critical, must run fast), interactive
//   processes (user apps, must feel responsive), and batch processes
//   (background jobs, no rush). A single queue cannot serve all of
//   these fairly. We need MULTIPLE QUEUES with different rules.
//
// THE SOLUTION — Time-Sliced MLQ:
//   We divide CPU time among 3 queues in fixed proportions.
//   Every "cycle" the CPU gives:
//     Q1 (System):      5 ticks = 50% of CPU time, uses Round Robin
//     Q2 (Interactive): 3 ticks = 30% of CPU time, uses Shortest Job First
//     Q3 (Batch):       2 ticks = 20% of CPU time, uses First Come First Served
//   Then it loops back to Q1 and repeats.
//
// PROCESS-TO-QUEUE ASSIGNMENT (by priority number from the PDF):
//   Priority  0-10  → Q1 (System)
//   Priority 11-20  → Q2 (Interactive)
//   Priority 21-30  → Q3 (Batch)
//
// ALGORITHMS:
//   Round Robin (Q1): Each process gets a max of 2 ticks (quantum),
//     then moves to the BACK of the queue. Fair time-sharing.
//   Shortest Job First (Q2): The process with the SMALLEST remaining
//     time runs first. Minimizes average waiting time.
//   First Come First Served (Q3): Processes run in the order they
//     arrived. Simplest possible algorithm.
//
// KEY METRICS (required by the PDF):
//   Waiting Time    = Completion Time - Arrival Time - Burst Time
//   Turnaround Time = Completion Time - Arrival Time
//   Response Time   = Time of first CPU access - Arrival Time
//
// Compile: g++ -o mlq_scheduler mlq_scheduler.cpp
// Run:     ./mlq_scheduler
// ============================================================

#include <iostream>   // cout, endl — for printing output
#include <vector>     // vector<> — resizable array for the process list
#include <deque>      // deque<> — double-ended queue (efficient front/back ops)
#include <algorithm>  // min() — finds the smaller of two values
#include <iomanip>    // setw() — controls column width for neat table formatting
using namespace std;

// ----------------------------------------------------------
// PROCESS STRUCT — one object per process in the simulation
// ----------------------------------------------------------
// This is the "process control block" (PCB) equivalent.
// Every field that the PDF's struct definition requires is here.
// ----------------------------------------------------------
struct Process {
    int pid;           // Process ID — unique identifier (1, 2, 3...)
    int arrival_time;  // When this process first appears in the system (tick)
    int burst_time;    // TOTAL CPU time this process needs to complete
    int remaining_time;// How much CPU time is still needed (counts DOWN to 0)
    int priority;      // Determines which queue: 0-10=Q1, 11-20=Q2, 21-30=Q3
    int start_time;    // The first tick this process ever got the CPU (-1 = not yet)
    int finish_time;   // The tick when this process finished (-1 = not done yet)
    bool activated;    // Has this process been moved into its queue yet? (starts false)
};

// ----------------------------------------------------------
// HELPER: get_queue
// ----------------------------------------------------------
// Given a priority number, returns which queue (1, 2, or 3)
// the process belongs to, according to the PDF's priority ranges.
// ----------------------------------------------------------
int get_queue(int priority) {
    if (priority <= 10) return 1;  // Priority 0-10:  Queue 1 (System)
    if (priority <= 20) return 2;  // Priority 11-20: Queue 2 (Interactive)
    return 3;                       // Priority 21-30: Queue 3 (Batch)
}

int main() {
    cout << "=== Time-Sliced MLQ Scheduler ===" << endl;
    cout << "Q1 (Round Robin, quantum=2):   5 ticks/cycle (50%)" << endl;
    cout << "Q2 (Shortest Job First):       3 ticks/cycle (30%)" << endl;
    cout << "Q3 (First Come First Served):  2 ticks/cycle (20%)" << endl;
    cout << endl;

    // ======================================================
    // STEP 1: Define the process set with arrival times
    // ======================================================
    // Each entry: {pid, arrival_time, burst_time, remaining_time,
    //              priority, start_time, finish_time, activated}
    // We initialize start_time and finish_time to -1 (= "not happened yet")
    // remaining_time starts equal to burst_time (full work left to do)
    // activated starts false (process hasn't entered any queue yet)
    vector<Process> procs = {
        {1, 0, 5, 5, 5,  -1, -1, false},  // P1: arrives at t=0, needs 5 ticks, Q1
        {2, 1, 3, 3, 8,  -1, -1, false},  // P2: arrives at t=1, needs 3 ticks, Q1
        {3, 0, 6, 6, 15, -1, -1, false},  // P3: arrives at t=0, needs 6 ticks, Q2
        {4, 2, 2, 2, 12, -1, -1, false},  // P4: arrives at t=2, needs 2 ticks, Q2
        {5, 0, 4, 4, 25, -1, -1, false},  // P5: arrives at t=0, needs 4 ticks, Q3
        {6, 3, 3, 3, 22, -1, -1, false},  // P6: arrives at t=3, needs 3 ticks, Q3
    };

    int n = procs.size(); // Total number of processes

    // Print the initial process table so the user can see what we're scheduling
    cout << "Processes:" << endl;
    cout << "  PID  Priority  Queue  Arrival  Burst" << endl;
    cout << "  ---  --------  -----  -------  -----" << endl;
    for (auto &p : procs) {
        // auto &p = reference to each Process in the vector (no copying)
        cout << "   " << p.pid
             << "      " << setw(2) << p.priority  // setw(2) = pad to 2 chars wide
             << "      Q" << get_queue(p.priority)
             << "       " << p.arrival_time
             << "       " << p.burst_time << endl;
    }
    cout << endl;

    // ======================================================
    // STEP 2: Set up the three separate queues
    // ======================================================
    // Each deque stores INDICES into the procs[] vector (not the Process objects).
    // We use indices so we can update procs[idx].remaining_time etc. easily.
    // deque = double-ended queue: efficient push_back (to back) and pop_front (from front).
    deque<int> q1, q2, q3;

    // Gantt chart: stores the PID that ran at each clock tick.
    // Example: gantt[0]=1 means "at tick 0, process P1 ran".
    // 0 in the gantt = idle (no process was ready).
    vector<int> gantt;

    int current_time = 0; // The simulation clock, starts at tick 0
    int completed = 0;    // How many processes have finished (stopping condition)

    // ----------------------------------------------------------
    // LAMBDA: activate_arrivals
    // ----------------------------------------------------------
    // A lambda is an inline anonymous function (C++11 feature).
    // [&] means it captures all local variables by reference.
    // This function scans all processes and moves any that have
    // arrived by current_time into their correct queue.
    // We call this at the start AND after every tick (new arrivals mid-sim).
    // ----------------------------------------------------------
    auto activate_arrivals = [&]() {
        for (int i = 0; i < n; i++) {
            // Check: process not yet activated AND it has arrived by now
            if (!procs[i].activated && procs[i].arrival_time <= current_time) {
                procs[i].activated = true;              // Mark as activated (don't add twice)
                int ql = get_queue(procs[i].priority);  // Which queue does it belong to?
                if (ql == 1) q1.push_back(i);           // Add index to Q1
                else if (ql == 2) q2.push_back(i);      // Add index to Q2
                else q3.push_back(i);                   // Add index to Q3
            }
        }
    };

    // ----------------------------------------------------------
    // LAMBDA: run_one_tick
    // ----------------------------------------------------------
    // Runs process at index idx for exactly ONE clock tick:
    //   1. Records start_time if this is the process's first CPU access.
    //   2. Decrements remaining_time by 1.
    //   3. Appends the PID to the Gantt chart.
    //   4. Advances the clock.
    //   5. Activates any newly arrived processes.
    // ----------------------------------------------------------
    auto run_one_tick = [&](int idx) {
        if (procs[idx].start_time == -1)
            procs[idx].start_time = current_time; // Record first-time CPU access (for Response Time)
        procs[idx].remaining_time--;               // One tick of CPU work done
        gantt.push_back(procs[idx].pid);           // Record this PID in the Gantt chart
        current_time++;                            // Advance the simulation clock
        activate_arrivals();                        // Check if any process just arrived
    };

    // Activate processes that arrive at time 0 before the main loop starts
    activate_arrivals();

    cout << "=== Execution Timeline ===" << endl;

    // ======================================================
    // STEP 3: The Main Scheduling Loop
    // ======================================================
    // We keep running cycles (Q1→Q2→Q3→Q1→...) until all n
    // processes have completed (completed == n).
    while (completed < n) {
        bool any_work = false; // Track if ANY queue did work this cycle
                               // (if not, all queues are empty → idle tick)

        // --------------------------------------------------
        // QUEUE 1: Round Robin (Budget = 5 ticks, Quantum = 2)
        // --------------------------------------------------
        // Round Robin: Each process gets AT MOST 2 ticks (quantum).
        // After 2 ticks (or when it finishes), it goes to the BACK of Q1.
        // The budget (5) limits the total ticks Q1 can use this cycle.
        {
            int budget = 5; // Q1 gets 5 ticks this cycle (50% of a 10-tick cycle)

            while (budget > 0 && !q1.empty()) {
                any_work = true;

                int idx = q1.front(); // Take the process at the FRONT of Q1
                q1.pop_front();       // Remove it from the front of the queue

                // Run for: min(quantum=2, remaining budget, remaining process time)
                // min() ensures we don't over-run on any of the three limits
                int ticks = min({2, budget, procs[idx].remaining_time});

                for (int i = 0; i < ticks; i++) {
                    cout << "  [t=" << current_time << "] Q1 (RR)   -> P"
                         << procs[idx].pid << endl;
                    run_one_tick(idx); // Run for 1 tick
                    budget--;         // Deduct from Q1's budget
                }

                if (procs[idx].remaining_time == 0) {
                    // Process is done!
                    procs[idx].finish_time = current_time; // Record completion time
                    completed++;                           // One more process done
                    cout << "  *** P" << procs[idx].pid << " FINISHED ***" << endl;
                } else {
                    q1.push_back(idx); // NOT done yet: send to BACK of queue (Round Robin!)
                }
            }
        }

        // --------------------------------------------------
        // QUEUE 2: Shortest Job First (Budget = 3 ticks)
        // --------------------------------------------------
        // SJF: Among all processes currently in Q2, pick the one
        // with the SMALLEST remaining_time and run it until it
        // finishes or the budget runs out. Then re-sort and pick again.
        {
            int budget = 3; // Q2 gets 3 ticks this cycle (30% of a 10-tick cycle)

            while (budget > 0 && !q2.empty()) {
                any_work = true;

                // Scan Q2 to find the index (si) of the process with shortest remaining time
                int si = 0; // Assume the first element is the shortest initially
                for (int i = 1; i < (int)q2.size(); i++) {
                    // If q2[i]'s remaining_time is less than current shortest, update si
                    if (procs[q2[i]].remaining_time < procs[q2[si]].remaining_time)
                        si = i;
                }

                int idx = q2[si];             // Index into procs[] of the shortest job
                q2.erase(q2.begin() + si);    // Remove it from its position in the deque

                // Run for min(budget, remaining time) — can't exceed either
                int ticks = min(budget, procs[idx].remaining_time);
                for (int i = 0; i < ticks; i++) {
                    cout << "  [t=" << current_time << "] Q2 (SJF)  -> P"
                         << procs[idx].pid << endl;
                    run_one_tick(idx); // Run for 1 tick
                    budget--;          // Deduct from Q2's budget
                }

                if (procs[idx].remaining_time == 0) {
                    procs[idx].finish_time = current_time; // Record completion
                    completed++;
                    cout << "  *** P" << procs[idx].pid << " FINISHED ***" << endl;
                } else {
                    q2.push_back(idx); // Not done — return to BACK of Q2
                }
            }
        }

        // --------------------------------------------------
        // QUEUE 3: First Come First Served (Budget = 2 ticks)
        // --------------------------------------------------
        // FCFS: The process that arrived EARLIEST (front of queue)
        // runs until it finishes or budget runs out. No reordering.
        {
            int budget = 2; // Q3 gets 2 ticks this cycle (20% of a 10-tick cycle)

            while (budget > 0 && !q3.empty()) {
                any_work = true;

                int idx = q3.front(); // Always take from the FRONT (arrival order)
                q3.pop_front();       // Remove from front

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
                    q3.push_front(idx); // FCFS: put back at FRONT (same process continues next cycle)
                }
            }
        }

        // If all three queues were empty but processes still haven't arrived,
        // advance time by one idle tick and check for new arrivals.
        if (!any_work && completed < n) {
            cout << "  [t=" << current_time << "] IDLE" << endl;
            gantt.push_back(0); // 0 = idle in the Gantt chart
            current_time++;
            activate_arrivals(); // Maybe a process arrives now
        }
    }

    // ======================================================
    // STEP 4: Print the Gantt Chart
    // ======================================================
    // Format: | P1 | P1 | P2 | P3 | P4 | ...
    //         0    1    2    3    4    5  ...
    cout << endl << "=== Gantt Chart ===" << endl << "  |";
    for (int i = 0; i < (int)gantt.size(); i++) {
        if (gantt[i] == 0) cout << " -- |"; // Idle tick
        else cout << " P" << gantt[i] << " |"; // Running process PID
    }
    cout << endl << "  ";
    // Print tick numbers below the chart (time axis)
    for (int i = 0; i <= (int)gantt.size(); i++) {
        cout << setw(4) << i << " "; // setw(4) = align each number in 4 char wide column
    }
    cout << endl;

    // ======================================================
    // STEP 5: Print Performance Metrics (required by PDF)
    // ======================================================
    // For each process, calculate and display:
    //   Waiting Time    = Turnaround - Burst = (finish-arrival) - burst
    //   Turnaround Time = finish_time - arrival_time
    //   Response Time   = start_time  - arrival_time
    cout << endl << "=== Performance Metrics ===" << endl;
    cout << "  PID  Queue  Arrival  Burst  Finish  Waiting  Turnaround  Response" << endl;
    cout << "  ---  -----  -------  -----  ------  -------  ----------  --------" << endl;

    double total_wt = 0, total_tat = 0, total_rt = 0; // Accumulators for averages

    for (auto &p : procs) {
        int turnaround = p.finish_time - p.arrival_time; // Total time process was in system
        int waiting    = turnaround - p.burst_time;      // Time spent waiting (not running)
        int response   = p.start_time - p.arrival_time;  // Delay before first CPU access

        total_wt  += waiting;     // Accumulate for average
        total_tat += turnaround;  // Accumulate for average
        total_rt  += response;    // Accumulate for average

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
    // Divide totals by n to get per-process averages
    cout << "  Avg Waiting Time:    " << fixed << setprecision(2) << total_wt / n << endl;
    cout << "  Avg Turnaround Time: " << fixed << setprecision(2) << total_tat / n << endl;
    cout << "  Avg Response Time:   " << fixed << setprecision(2) << total_rt / n << endl;

    cout << endl << "All " << n << " processes finished at time t=" << current_time << endl;
    return 0;
}
