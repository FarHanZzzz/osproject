// ============================================================
// Task E — Real-Time Scheduler: Rate Monotonic vs EDF
// ============================================================
// PROBLEM (from PDF): Simulate two real-time scheduling algorithms
// (Rate Monotonic and Earliest Deadline First) on a set of periodic
// tasks. Show the execution timeline and check if deadlines are met.
//
// HOW WE SOLVE IT:
//   1. Define 3 hardcoded periodic tasks with known exec times and periods.
//   2. Hardcode the Hyperperiod to 20 (LCM of 5, 10, 20).
//   3. For RM: priority is fixed — shorter period = higher priority.
//      We just use an if-else chain (Task1 always checked first).
//   4. For EDF: priority is dynamic — whoever's deadline is closest
//      gets the CPU. We compute deadlines on the fly.
//   5. At the start of each period, we "wake up" (activate) the task
//      by resetting its remaining execution time.
//
// Compile: g++ -o rt_scheduler rt_scheduler.cpp
// Run:     ./rt_scheduler
// ============================================================

#include <iostream>  // For cout
#include <vector>    // For vector
using namespace std;

// ----------------------------------------------------------
// TASK STRUCT
// ----------------------------------------------------------
// Each task has:
//   id        — a name (1, 2, or 3)
//   exec_time — how many ticks it needs to run each period
//   period    — how often it repeats (also its deadline)
//   time_left — how many ticks it still needs in the current period
struct Task {
    int id;
    int exec_time;  // Ci: execution time per period
    int period;     // Ti: period length (deadline = period end)
    int time_left;  // Remaining execution time in current period
};

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main() {
    cout << "=== Real-Time Scheduler ===" << endl << endl;

    // ======================================================
    // STEP 1: Define the hardcoded task set
    // ======================================================
    // From the PDF:
    //   Task 1: Exec=2, Period=5   (runs every 5 ticks, needs 2 ticks)
    //   Task 2: Exec=4, Period=10  (runs every 10 ticks, needs 4 ticks)
    //   Task 3: Exec=1, Period=20  (runs every 20 ticks, needs 1 tick)
    //
    // Total Utilization = 2/5 + 4/10 + 1/20 = 0.4 + 0.4 + 0.05 = 0.85
    // This means the CPU is 85% busy. EDF works if utilization <= 1.0 (it does!).
    // RM might also work — let's simulate and find out!
    vector<Task> tasks = {
        {1, 2, 5,  0},   // Task 1
        {2, 4, 10, 0},   // Task 2
        {3, 1, 20, 0},   // Task 3
    };

    // Print the task info
    cout << "Task Set:" << endl;
    cout << "  Task  Exec(Ci)  Period(Ti)  Utilization" << endl;
    cout << "  ----  --------  ----------  -----------" << endl;
    for (auto &t : tasks) {
        cout << "    " << t.id << "       " << t.exec_time
             << "         " << t.period
             << "         " << (double)t.exec_time / t.period << endl;
    }
    cout << "  Total Utilization = 0.85" << endl << endl;

    // ======================================================
    // Hardcoded Hyperperiod = 20
    // ======================================================
    // The Hyperperiod is the LCM (Least Common Multiple) of all periods.
    // LCM(5, 10, 20) = 20. After 20 ticks, the entire pattern repeats.
    // So we only need to simulate 20 ticks to prove correctness.
    int hyperperiod = 20;
    cout << "Hyperperiod = " << hyperperiod << " ticks" << endl << endl;

    // ======================================================
    // SIMULATION 1: Rate Monotonic (RM)
    // ======================================================
    // RM RULE: The task with the SHORTEST PERIOD always has the
    // HIGHEST priority. This priority NEVER changes.
    //   Task 1 (period 5)  → Highest priority (checked first)
    //   Task 2 (period 10) → Medium priority
    //   Task 3 (period 20) → Lowest priority
    cout << string(50, '=') << endl;
    cout << "=== Rate Monotonic (RM) Simulation ===" << endl;
    cout << string(50, '=') << endl;

    // Reset all tasks
    tasks[0].time_left = 0;
    tasks[1].time_left = 0;
    tasks[2].time_left = 0;

    int rm_misses = 0;  // Count deadline misses

    for (int tick = 0; tick < hyperperiod; tick++) {
        // --- ACTIVATE tasks at the start of their new period ---
        // If the current tick is a multiple of the period,
        // a new instance of the task arrives and needs CPU time.
        if (tick % 5 == 0) {
            // Check if previous instance missed its deadline
            if (tasks[0].time_left > 0) {
                cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task 1 ***" << endl;
                rm_misses++;
            }
            tasks[0].time_left = tasks[0].exec_time;  // Reset to 2
        }
        if (tick % 10 == 0) {
            if (tasks[1].time_left > 0) {
                cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task 2 ***" << endl;
                rm_misses++;
            }
            tasks[1].time_left = tasks[1].exec_time;  // Reset to 4
        }
        if (tick % 20 == 0) {
            if (tasks[2].time_left > 0) {
                cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task 3 ***" << endl;
                rm_misses++;
            }
            tasks[2].time_left = tasks[2].exec_time;  // Reset to 1
        }

        // --- PICK the highest-priority task that still needs CPU ---
        // Because RM uses FIXED priorities, we just check Task 1 first,
        // then Task 2, then Task 3. Simple if-else chain!
        if (tasks[0].time_left > 0) {
            tasks[0].time_left--;
            cout << "  [Tick " << tick << "] Running Task 1" << endl;
        }
        else if (tasks[1].time_left > 0) {
            tasks[1].time_left--;
            cout << "  [Tick " << tick << "] Running Task 2" << endl;
        }
        else if (tasks[2].time_left > 0) {
            tasks[2].time_left--;
            cout << "  [Tick " << tick << "] Running Task 3" << endl;
        }
        else {
            cout << "  [Tick " << tick << "] IDLE" << endl;
        }
    }

    cout << "\nRM Result: " << rm_misses << " deadline miss(es)" << endl;
    if (rm_misses == 0) cout << "All deadlines met!" << endl;
    cout << endl;

    // ======================================================
    // SIMULATION 2: Earliest Deadline First (EDF)
    // ======================================================
    // EDF RULE: The task whose ABSOLUTE DEADLINE is closest
    // (soonest) gets the CPU. Priority changes DYNAMICALLY every tick.
    cout << string(50, '=') << endl;
    cout << "=== Earliest Deadline First (EDF) Simulation ===" << endl;
    cout << string(50, '=') << endl;

    // Reset all tasks
    tasks[0].time_left = 0;
    tasks[1].time_left = 0;
    tasks[2].time_left = 0;

    int edf_misses = 0;

    for (int tick = 0; tick < hyperperiod; tick++) {
        // --- ACTIVATE tasks at the start of their new period ---
        if (tick % 5 == 0) {
            if (tasks[0].time_left > 0) {
                cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task 1 ***" << endl;
                edf_misses++;
            }
            tasks[0].time_left = tasks[0].exec_time;
        }
        if (tick % 10 == 0) {
            if (tasks[1].time_left > 0) {
                cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task 2 ***" << endl;
                edf_misses++;
            }
            tasks[1].time_left = tasks[1].exec_time;
        }
        if (tick % 20 == 0) {
            if (tasks[2].time_left > 0) {
                cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task 3 ***" << endl;
                edf_misses++;
            }
            tasks[2].time_left = tasks[2].exec_time;
        }

        // --- CALCULATE each task's absolute deadline ---
        // The deadline for each task instance is the END of its current period.
        // For Task 1 at tick 3: current period is [0,5), so deadline = 5.
        // Formula: ((tick / period) + 1) * period
        int deadline1 = ((tick / 5) + 1) * 5;
        int deadline2 = ((tick / 10) + 1) * 10;
        int deadline3 = ((tick / 20) + 1) * 20;

        // --- PICK the task with the EARLIEST (smallest) deadline ---
        int best = -1;             // Index of the task to run (-1 = idle)
        int earliest = 999999;     // The smallest deadline found so far

        if (tasks[0].time_left > 0 && deadline1 < earliest) {
            best = 0; earliest = deadline1;
        }
        if (tasks[1].time_left > 0 && deadline2 < earliest) {
            best = 1; earliest = deadline2;
        }
        if (tasks[2].time_left > 0 && deadline3 < earliest) {
            best = 2; earliest = deadline3;
        }

        // --- EXECUTE the chosen task for 1 tick ---
        if (best != -1) {
            tasks[best].time_left--;
            cout << "  [Tick " << tick << "] Running Task " << tasks[best].id
                 << "  (deadline=" << earliest << ")" << endl;
        } else {
            cout << "  [Tick " << tick << "] IDLE" << endl;
        }
    }

    cout << "\nEDF Result: " << edf_misses << " deadline miss(es)" << endl;
    if (edf_misses == 0) cout << "All deadlines met!" << endl;

    return 0;  // Program finished successfully
}
