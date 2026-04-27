// ============================================================
// Task E — Real-Time Task Scheduler & Schedulability Analyzer
// ============================================================
//
// THE PROBLEM:
//   In real-time systems (medical devices, flight controllers),
//   tasks must complete BY THEIR DEADLINE — being late is as bad
//   as being wrong. We need to simulate two scheduling algorithms
//   and check BEFORE running whether the task set is even schedulable.
//
// KEY CONCEPT — Periodic Tasks:
//   In real-time systems, tasks repeat on a fixed schedule:
//     Period (Ti):         How often the task recurs (e.g., every 5 ticks)
//     Execution Time (Ci): CPU time needed each period (e.g., 2 ticks)
//     Deadline (Di):       Must finish within Di ticks of activation
//                          For this assignment: Deadline = Period (D = P)
//     Utilization (Ui):    Ui = Ci / Ti  (fraction of CPU this task needs)
//     Total Utilization U: Sum of all Ui. If U > 1.0, CPU is overloaded.
//
// THE TWO ALGORITHMS (from the PDF):
//
//   Rate Monotonic (RM) — STATIC priorities:
//     Rule: Shorter period = higher fixed priority. Assigned ONCE at start.
//     Task 1 (period=5)  → highest priority (runs first if ready)
//     Task 2 (period=10) → medium priority
//     Task 3 (period=20) → lowest priority
//     Simple but sometimes misses deadlines when utilization is high.
//
//   Earliest Deadline First (EDF) — DYNAMIC priorities:
//     Rule: The task whose absolute deadline is CLOSEST (soonest) runs next.
//     Priority changes every tick as deadlines approach.
//     Absolute Deadline = time task was activated + its period.
//     More complex but provably optimal: if ANY algorithm can meet
//     deadlines, EDF can too (as long as total utilization <= 1.0).
//
// SCHEDULABILITY TESTING (run BEFORE simulation):
//   RM Bound:  U <= n * (2^(1/n) - 1)  → If satisfied, RM is GUARANTEED.
//              For n=3: bound ≈ 0.779. If U > bound, INCONCLUSIVE (simulate).
//   EDF Bound: U <= 1.0                → EDF is GUARANTEED if this holds.
//
// EXAMPLE TASK SET (directly from the PDF, Table on Page 8):
//   Task 1: Ci=2, Ti=5  → Ui=0.40
//   Task 2: Ci=4, Ti=10 → Ui=0.40
//   Task 3: Ci=1, Ti=20 → Ui=0.05
//   Total U = 0.85 (85%)
//   Expected: RM inconclusive (0.85 > 0.779), EDF guaranteed (0.85 < 1.0)
//
// Compile: g++ -o rt_scheduler rt_scheduler.cpp
// Run:     ./rt_scheduler
// ============================================================

#include <iostream>  // cout, endl — for printing output
#include <vector>    // vector<> — resizable array for the task list
#include <cmath>     // pow() — computes 2^(1/n) for the RM bound formula
#include <iomanip>   // setprecision() — controls decimal places in output
using namespace std;

// ----------------------------------------------------------
// TASK STRUCT — represents one periodic real-time task
// ----------------------------------------------------------
// The PDF specifies: period, execution time, deadline (=period).
// We also track remaining_time per tick and the task's ID.
// ----------------------------------------------------------
struct Task {
    int id;          // Task number (1, 2, 3) — just for display
    int exec_time;   // Ci: CPU units needed per period (e.g., 2 ticks)
    int period;      // Ti: how often the task repeats (e.g., every 5 ticks)
    int time_left;   // Remaining execution time in the CURRENT period
                     // Resets to exec_time at the start of each period
                     // Counts DOWN to 0 as the task gets CPU time
};

// ----------------------------------------------------------
// HELPER: gcd — Greatest Common Divisor (Euclidean algorithm)
// ----------------------------------------------------------
// Used to compute LCM. The GCD of a and b is the largest number
// that divides both evenly.
// Example: gcd(10, 20) = 10
// ----------------------------------------------------------
int gcd(int a, int b) {
    while (b != 0) {  // Keep going until remainder is 0
        int temp = b;  // Save b
        b = a % b;     // New b = remainder of a divided by b
        a = temp;      // New a = old b
    }
    return a; // When b=0, a holds the GCD
}

// ----------------------------------------------------------
// HELPER: lcm — Least Common Multiple
// ----------------------------------------------------------
// LCM(a, b) = a * b / gcd(a, b)
// We compute it as a/gcd*b to avoid integer overflow for large values.
// Example: lcm(5, 10) = 10, lcm(5, 20) = 20, lcm(10, 20) = 20
// ----------------------------------------------------------
int lcm(int a, int b) {
    return a / gcd(a, b) * b; // Division first prevents overflow
}

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main() {
    cout << "=== Real-Time Scheduler ===" << endl << endl;

    // ======================================================
    // STEP 1: Define the task set (exact values from PDF table)
    // ======================================================
    // {id, exec_time (Ci), period (Ti), time_left}
    // time_left starts at 0 — tasks are not "active" yet at tick 0
    // (they become active when their first period starts)
    vector<Task> tasks = {
        {1, 2, 5,  0},   // Task 1: needs 2 ticks, repeats every 5  → U=0.40
        {2, 4, 10, 0},   // Task 2: needs 4 ticks, repeats every 10 → U=0.40
        {3, 1, 20, 0},   // Task 3: needs 1 tick,  repeats every 20 → U=0.05
    };

    int n = tasks.size(); // Number of tasks (3 in this case)

    // Print the task set table matching the PDF's example layout
    cout << "Task Set:" << endl;
    cout << "  Task  Exec(Ci)  Period(Ti)  Utilization" << endl;
    cout << "  ----  --------  ----------  -----------" << endl;
    double total_util = 0.0; // Accumulator for total utilization U
    for (auto &t : tasks) {
        double u = (double)t.exec_time / t.period; // Ui = Ci / Ti
        total_util += u;                            // Add to total U
        cout << "    " << t.id
             << "       " << t.exec_time
             << "         " << t.period
             << "         " << fixed << setprecision(2) << u << endl;
    }
    cout << "  Total Utilization = " << fixed << setprecision(2) << total_util << endl << endl;

    // ======================================================
    // STEP 2: Compute Hyperperiod = LCM of all task periods
    // ======================================================
    // The hyperperiod is the SMALLEST time window where every task
    // completes at least one full cycle. We simulate for exactly
    // one hyperperiod — after that, the schedule repeats forever.
    // LCM(5, 10) = 10, then LCM(10, 20) = 20. So hyperperiod = 20.
    int hyperperiod = tasks[0].period; // Start with the first task's period
    for (int i = 1; i < n; i++) {
        hyperperiod = lcm(hyperperiod, tasks[i].period); // Extend LCM step by step
    }
    cout << "Hyperperiod = LCM(" << tasks[0].period;
    for (int i = 1; i < n; i++) cout << ", " << tasks[i].period;
    cout << ") = " << hyperperiod << " ticks" << endl << endl;

    // ======================================================
    // STEP 3: Schedulability Analysis (before any simulation)
    // ======================================================
    // We check MATHEMATICALLY whether the task set can be scheduled
    // before wasting time running the simulation.
    cout << string(50, '=') << endl;
    cout << "=== Schedulability Analysis ===" << endl;
    cout << string(50, '=') << endl;

    // --- RM Bound Test ---
    // Formula from PDF: U_bound = n * (2^(1/n) - 1)
    // For n=3: 3 * (2^(1/3) - 1) = 3 * (1.2599 - 1) = 3 * 0.2599 ≈ 0.779
    // pow(2.0, 1.0/n) = 2 raised to the power (1/n)
    double rm_bound = n * (pow(2.0, 1.0 / n) - 1);
    cout << endl;
    cout << "  RM Bound for n=" << n << ": "
         << fixed << setprecision(3) << rm_bound << endl; // Expected: 0.779
    cout << "  Total Utilization: " << fixed << setprecision(3) << total_util << endl;

    if (total_util <= rm_bound) {
        // U <= bound → RM is mathematically guaranteed to meet all deadlines
        cout << "  RM Result: U <= " << fixed << setprecision(3) << rm_bound
             << " => GUARANTEED schedulable by RM" << endl;
    } else if (total_util <= 1.0) {
        // U > bound but U <= 1.0 → RM MIGHT work, but we can't guarantee it
        // The PDF says for our task set (U=0.85 > 0.779) this is the case
        cout << "  RM Result: U > " << fixed << setprecision(3) << rm_bound
             << " => INCONCLUSIVE (must simulate to verify)" << endl;
    } else {
        // U > 1.0 → CPU is overloaded, NO algorithm can schedule this
        cout << "  RM Result: U > 1.0 => NOT schedulable by ANY algorithm" << endl;
    }

    // --- EDF Bound Test ---
    // EDF is the OPTIMAL algorithm for preemptive scheduling.
    // If U <= 1.0, EDF GUARANTEES all deadlines are met.
    // Our task set: U=0.85 < 1.0 → EDF is guaranteed.
    cout << endl;
    cout << "  EDF Bound: 1.000 (100% utilization)" << endl;
    cout << "  Total Utilization: " << fixed << setprecision(3) << total_util << endl;

    if (total_util <= 1.0) {
        cout << "  EDF Result: U <= 1.0 => GUARANTEED schedulable by EDF" << endl;
    } else {
        cout << "  EDF Result: U > 1.0 => NOT schedulable" << endl;
    }
    cout << endl;

    // ======================================================
    // STEP 4: Rate Monotonic (RM) Simulation
    // ======================================================
    // RM assigns STATIC (fixed) priorities based on period:
    //   Task 1 (period=5)  → Priority 1 (highest — checked first in loop)
    //   Task 2 (period=10) → Priority 2 (medium)
    //   Task 3 (period=20) → Priority 3 (lowest)
    // The tasks vector is already sorted by period ascending,
    // so looping from index 0 = checking highest priority first.
    cout << string(50, '=') << endl;
    cout << "=== Rate Monotonic (RM) Simulation ===" << endl;
    cout << string(50, '=') << endl;

    // Reset time_left to 0 for all tasks before starting the RM simulation
    for (auto &t : tasks) t.time_left = 0;

    int rm_misses = 0; // Counter for how many deadline misses occur

    // The simulation runs from tick 0 to tick hyperperiod-1
    for (int tick = 0; tick < hyperperiod; tick++) {

        // --- At the start of each period: activate (or re-activate) tasks ---
        // tick % task.period == 0 means "this tick is the start of a new period"
        // Example: Task 1 activates at tick 0, 5, 10, 15, 20...
        //          Task 2 activates at tick 0, 10, 20...
        //          Task 3 activates at tick 0, 20...
        for (int i = 0; i < n; i++) {
            if (tick % tasks[i].period == 0) {
                // If time_left > 0, the previous period's work wasn't done → DEADLINE MISS!
                if (tasks[i].time_left > 0) {
                    cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task "
                         << tasks[i].id << " ***" << endl;
                    rm_misses++;
                }
                // Reset: give the task its full execution budget for this new period
                tasks[i].time_left = tasks[i].exec_time;
            }
        }

        // --- Select the highest-priority READY task ---
        // "Ready" means time_left > 0 (it has work remaining in this period).
        // We iterate from index 0 (Task 1, period=5, highest priority) upward.
        // The FIRST ready task we find is the one we run (RM static priority).
        int chosen = -1; // -1 = no task ready (CPU will be idle)
        for (int i = 0; i < n; i++) {
            if (tasks[i].time_left > 0) {
                chosen = i; // This is the highest-priority ready task
                break;       // Stop scanning — we found our pick
            }
        }

        if (chosen != -1) {
            tasks[chosen].time_left--; // Use 1 tick of this task's execution budget
            cout << "  [Tick " << tick << "] Running Task " << tasks[chosen].id << endl;
        } else {
            cout << "  [Tick " << tick << "] IDLE" << endl; // No task is ready
        }
    }

    // Summary for RM
    cout << "\nRM Result: " << rm_misses << " deadline miss(es)" << endl;
    if (rm_misses == 0) cout << "All deadlines met!" << endl;
    cout << endl;

    // ======================================================
    // STEP 5: Earliest Deadline First (EDF) Simulation
    // ======================================================
    // EDF assigns DYNAMIC priorities at every tick:
    //   The task with the EARLIEST (smallest) absolute deadline runs.
    //   Absolute deadline = time the period started + the period length.
    // When a new task arrives with a closer deadline, it PREEMPTS
    // the currently running task immediately.
    cout << string(50, '=') << endl;
    cout << "=== Earliest Deadline First (EDF) Simulation ===" << endl;
    cout << string(50, '=') << endl;

    // Reset all tasks before the EDF simulation
    for (auto &t : tasks) t.time_left = 0;

    // abs_deadline[i] tracks the absolute deadline for task i's current period.
    // Example: if Task 1 (period=5) activates at tick 10, abs_deadline[0] = 10+5 = 15.
    vector<int> abs_deadline(n, 0); // Initialize all deadlines to 0

    int edf_misses = 0; // Counter for EDF deadline misses

    for (int tick = 0; tick < hyperperiod; tick++) {

        // --- Activate tasks and check for deadline misses at period start ---
        for (int i = 0; i < n; i++) {
            if (tick % tasks[i].period == 0) {
                if (tasks[i].time_left > 0) {
                    // Previous period's work wasn't finished → deadline missed!
                    cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task "
                         << tasks[i].id << " ***" << endl;
                    edf_misses++;
                }
                tasks[i].time_left = tasks[i].exec_time; // Reload work for new period
                // Set absolute deadline: this period must complete by this tick
                abs_deadline[i] = tick + tasks[i].period;
            }
        }

        // --- Select the task with the EARLIEST absolute deadline ---
        // This is the core EDF decision: re-evaluate EVERY tick.
        int best = -1;       // Index of the chosen task (-1 = none ready)
        int earliest = 999999; // Start with a huge number; any real deadline is smaller

        for (int i = 0; i < n; i++) {
            // Only consider tasks that have work remaining (time_left > 0)
            // AND whose absolute deadline is the earliest seen so far
            if (tasks[i].time_left > 0 && abs_deadline[i] < earliest) {
                best = i;                    // This task has the tightest deadline
                earliest = abs_deadline[i]; // Remember this deadline value
            }
        }

        if (best != -1) {
            tasks[best].time_left--; // Use 1 tick on the chosen task
            cout << "  [Tick " << tick << "] Running Task " << tasks[best].id
                 << "  (deadline=" << abs_deadline[best] << ")" << endl;
        } else {
            cout << "  [Tick " << tick << "] IDLE" << endl;
        }
    }

    // Summary for EDF
    cout << "\nEDF Result: " << edf_misses << " deadline miss(es)" << endl;
    if (edf_misses == 0) cout << "All deadlines met!" << endl;

    return 0;
}
