// ============================================================
// Task E — Real-Time Scheduler: Rate Monotonic vs EDF
// ============================================================
// PROBLEM (from PDF): Simulate two real-time scheduling algorithms
// (Rate Monotonic and Earliest Deadline First) on a set of periodic
// tasks. Show the execution timeline, check if deadlines are met,
// and perform schedulability analysis BEFORE running simulations.
//
// FEATURES:
//   1. Schedulability Testing (RM bound + EDF bound)
//   2. Rate Monotonic simulation with fixed priorities
//   3. EDF simulation with dynamic priorities
//   4. Deadline miss detection
//   5. Hyperperiod calculation
//
// Compile: g++ -o rt_scheduler rt_scheduler.cpp
// Run:     ./rt_scheduler
// ============================================================

#include <iostream>
#include <vector>
#include <cmath>     // For pow() — needed for RM bound calculation
#include <iomanip>   // For setprecision()
using namespace std;

// ----------------------------------------------------------
// TASK STRUCT
// ----------------------------------------------------------
struct Task {
    int id;
    int exec_time;  // Ci: execution time per period
    int period;     // Ti: period length (deadline = period end)
    int time_left;  // Remaining execution time in current period
};

// ----------------------------------------------------------
// HELPER: Compute GCD (for LCM calculation)
// ----------------------------------------------------------
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// ----------------------------------------------------------
// HELPER: Compute LCM (for hyperperiod)
// ----------------------------------------------------------
int lcm(int a, int b) {
    return a / gcd(a, b) * b;
}

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main() {
    cout << "=== Real-Time Scheduler ===" << endl << endl;

    // ======================================================
    // STEP 1: Define the task set
    // ======================================================
    vector<Task> tasks = {
        {1, 2, 5,  0},   // Task 1: Exec=2, Period=5
        {2, 4, 10, 0},   // Task 2: Exec=4, Period=10
        {3, 1, 20, 0},   // Task 3: Exec=1, Period=20
    };

    int n = tasks.size();

    // Print task info
    cout << "Task Set:" << endl;
    cout << "  Task  Exec(Ci)  Period(Ti)  Utilization" << endl;
    cout << "  ----  --------  ----------  -----------" << endl;
    double total_util = 0.0;
    for (auto &t : tasks) {
        double u = (double)t.exec_time / t.period;
        total_util += u;
        cout << "    " << t.id << "       " << t.exec_time
             << "         " << t.period
             << "         " << fixed << setprecision(2) << u << endl;
    }
    cout << "  Total Utilization = " << fixed << setprecision(2) << total_util << endl << endl;

    // ======================================================
    // STEP 2: Compute Hyperperiod (LCM of all periods)
    // ======================================================
    int hyperperiod = tasks[0].period;
    for (int i = 1; i < n; i++) {
        hyperperiod = lcm(hyperperiod, tasks[i].period);
    }
    cout << "Hyperperiod = LCM(" << tasks[0].period;
    for (int i = 1; i < n; i++) cout << ", " << tasks[i].period;
    cout << ") = " << hyperperiod << " ticks" << endl << endl;

    // ======================================================
    // STEP 3: SCHEDULABILITY TESTING (before simulation)
    // ======================================================
    // This checks whether the task set CAN be scheduled
    // BEFORE actually running the simulation.
    cout << string(50, '=') << endl;
    cout << "=== Schedulability Analysis ===" << endl;
    cout << string(50, '=') << endl;

    // --- RM Bound Test ---
    // Formula: n * (2^(1/n) - 1)
    // For n=3: 3 * (2^(1/3) - 1) ≈ 0.779
    // If U <= RM bound → RM is GUARANTEED to work
    // If U > RM bound  → RM MIGHT still work (inconclusive)
    double rm_bound = n * (pow(2.0, 1.0 / n) - 1);
    cout << endl;
    cout << "  RM Bound for n=" << n << ": "
         << fixed << setprecision(3) << rm_bound << endl;
    cout << "  Total Utilization: " << fixed << setprecision(3) << total_util << endl;

    if (total_util <= rm_bound) {
        cout << "  RM Result: U <= " << fixed << setprecision(3) << rm_bound
             << " => GUARANTEED schedulable by RM" << endl;
    } else if (total_util <= 1.0) {
        cout << "  RM Result: U > " << fixed << setprecision(3) << rm_bound
             << " => INCONCLUSIVE (must simulate to verify)" << endl;
    } else {
        cout << "  RM Result: U > 1.0 => NOT schedulable by ANY algorithm" << endl;
    }

    // --- EDF Bound Test ---
    // EDF is guaranteed to work if U <= 1.0
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
    // RM RULE: Shortest period = highest FIXED priority.
    //   Task 1 (period 5)  → Highest
    //   Task 2 (period 10) → Medium
    //   Task 3 (period 20) → Lowest
    cout << string(50, '=') << endl;
    cout << "=== Rate Monotonic (RM) Simulation ===" << endl;
    cout << string(50, '=') << endl;

    // Reset all tasks
    for (auto &t : tasks) t.time_left = 0;

    int rm_misses = 0;

    for (int tick = 0; tick < hyperperiod; tick++) {
        // Activate tasks at the start of their new period
        for (int i = 0; i < n; i++) {
            if (tick % tasks[i].period == 0) {
                if (tasks[i].time_left > 0) {
                    cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task "
                         << tasks[i].id << " ***" << endl;
                    rm_misses++;
                }
                tasks[i].time_left = tasks[i].exec_time;
            }
        }

        // Pick highest-priority ready task (shortest period first)
        // Tasks are already sorted by period (5, 10, 20)
        int chosen = -1;
        for (int i = 0; i < n; i++) {
            if (tasks[i].time_left > 0) {
                chosen = i;
                break;  // First ready task = highest priority
            }
        }

        if (chosen != -1) {
            tasks[chosen].time_left--;
            cout << "  [Tick " << tick << "] Running Task " << tasks[chosen].id << endl;
        } else {
            cout << "  [Tick " << tick << "] IDLE" << endl;
        }
    }

    cout << "\nRM Result: " << rm_misses << " deadline miss(es)" << endl;
    if (rm_misses == 0) cout << "All deadlines met!" << endl;
    cout << endl;

    // ======================================================
    // STEP 5: Earliest Deadline First (EDF) Simulation
    // ======================================================
    // EDF RULE: The task whose ABSOLUTE DEADLINE is closest
    // (soonest) gets the CPU. Priority changes DYNAMICALLY.
    cout << string(50, '=') << endl;
    cout << "=== Earliest Deadline First (EDF) Simulation ===" << endl;
    cout << string(50, '=') << endl;

    // Reset all tasks
    for (auto &t : tasks) t.time_left = 0;

    // Track absolute deadlines for each task
    vector<int> abs_deadline(n, 0);

    int edf_misses = 0;

    for (int tick = 0; tick < hyperperiod; tick++) {
        // Activate tasks at the start of their new period
        for (int i = 0; i < n; i++) {
            if (tick % tasks[i].period == 0) {
                if (tasks[i].time_left > 0) {
                    cout << "  [Tick " << tick << "] *** DEADLINE MISS: Task "
                         << tasks[i].id << " ***" << endl;
                    edf_misses++;
                }
                tasks[i].time_left = tasks[i].exec_time;
                // Set absolute deadline = arrival + period
                abs_deadline[i] = tick + tasks[i].period;
            }
        }

        // Pick the task with the EARLIEST (smallest) absolute deadline
        int best = -1;
        int earliest = 999999;

        for (int i = 0; i < n; i++) {
            if (tasks[i].time_left > 0 && abs_deadline[i] < earliest) {
                best = i;
                earliest = abs_deadline[i];
            }
        }

        if (best != -1) {
            tasks[best].time_left--;
            cout << "  [Tick " << tick << "] Running Task " << tasks[best].id
                 << "  (deadline=" << abs_deadline[best] << ")" << endl;
        } else {
            cout << "  [Tick " << tick << "] IDLE" << endl;
        }
    }

    cout << "\nEDF Result: " << edf_misses << " deadline miss(es)" << endl;
    if (edf_misses == 0) cout << "All deadlines met!" << endl;

    return 0;
}
