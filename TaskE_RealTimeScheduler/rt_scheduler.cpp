// Task E — Real-Time Scheduler: Rate Monotonic (RM) vs Earliest Deadline First (EDF)
// Simulates periodic tasks with deadlines. Tests if RM and EDF can schedule them.
//
// Compile: g++ -o rt_scheduler rt_scheduler.cpp
// Run:     ./rt_scheduler

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct Task {
    int    id;
    double exec_time;    // how long it runs each period (Ci)
    double period;       // how often it repeats (Ti)
    double deadline;     // must finish within this time (Di = Ti)
    double utilization;  // Ci / Ti

    // Simulation state (changes during simulation)
    double remaining;           // time left in current period
    double absolute_deadline;   // when current instance must finish
    bool   ready;               // is this instance active?
    int    static_priority;     // for RM: lower = higher priority

    // Stats
    int completed;
    int missed;
};

// Compute GCD of two numbers
long long gcd(long long a, long long b) {
    while (b) { long long temp = b; b = a % b; a = temp; }
    return a;
}

// Compute LCM of two numbers
long long lcm(long long a, long long b) {
    return (a / gcd(a, b)) * b;
}

// Compute hyperperiod = LCM of all periods
int compute_hyperperiod(vector<Task> &tasks) {
    long long hp = 1;
    for (auto &t : tasks)
        hp = lcm(hp, (long long)t.period);
    return (int)hp;
}

// RM utilization bound: n * (2^(1/n) - 1)
double rm_bound(int n) {
    return (double)n * (pow(2.0, 1.0 / (double)n) - 1.0);
}

// Check if the task set is schedulable
void check_schedulability(vector<Task> &tasks) {
    int n = tasks.size();
    double total_U = 0;
    for (auto &t : tasks) total_U += t.utilization;

    double rm_limit  = rm_bound(n);
    double edf_limit = 1.0;

    cout << "=== Schedulability Analysis ===" << endl;
    cout << "Task Set:" << endl;
    for (auto &t : tasks) {
        cout << "  Task " << t.id << ": Ci=" << t.exec_time
             << ", Ti=" << t.period << ", Ui=" << t.utilization << endl;
    }
    cout << "  Total Utilization U = " << total_U << endl << endl;

    cout << "RM  bound (n=" << n << "): " << rm_limit << endl;
    cout << "EDF bound:         " << edf_limit << endl << endl;

    cout << "RM  schedulable?  " << (total_U <= rm_limit ? "YES" : "NO (U > bound)") << endl;
    cout << "EDF schedulable?  " << (total_U <= edf_limit ? "YES" : "NO (U > 1.0)") << endl;
    cout << endl;
}

// Activate tasks at the start of each new period
void activate_tasks(vector<Task> &tasks, int tick) {
    for (auto &t : tasks) {
        if (tick % (int)t.period == 0) {
            t.ready             = true;
            t.remaining         = t.exec_time;
            t.absolute_deadline = tick + t.period;
        }
    }
}

// RM: pick the ready task with the highest static priority (lowest number)
Task* pick_rm(vector<Task> &tasks) {
    Task *best = nullptr;
    for (auto &t : tasks) {
        if (!t.ready || t.remaining <= 0) continue;
        if (!best || t.static_priority < best->static_priority)
            best = &t;
    }
    return best;
}

// EDF: pick the ready task with the earliest absolute deadline
Task* pick_edf(vector<Task> &tasks) {
    Task *best = nullptr;
    for (auto &t : tasks) {
        if (!t.ready || t.remaining <= 0) continue;
        if (!best || t.absolute_deadline < best->absolute_deadline)
            best = &t;
    }
    return best;
}

// Run one simulation (either RM or EDF)
void simulate(vector<Task> tasks, string mode, int hyperperiod) {
    cout << "=== " << mode << " Simulation (0 to " << hyperperiod << " ticks) ===" << endl;

    // For RM: assign static priorities (shorter period = higher priority = lower number)
    if (mode == "RM") {
        // Sort by period, assign priority 1, 2, 3...
        vector<Task*> sorted;
        for (auto &t : tasks) sorted.push_back(&t);
        sort(sorted.begin(), sorted.end(),
             [](Task *a, Task *b) { return a->period < b->period; });
        for (int i = 0; i < (int)sorted.size(); i++)
            sorted[i]->static_priority = i + 1;
    }

    int total_misses = 0;

    for (int tick = 0; tick < hyperperiod; tick++) {
        // Step 1: Activate tasks whose new period starts now
        activate_tasks(tasks, tick);

        // Step 2: Check for deadline misses
        for (auto &t : tasks) {
            if (t.ready && t.remaining > 0 && (double)tick >= t.absolute_deadline) {
                cout << "  [t=" << tick << "] DEADLINE MISSED: Task " << t.id << endl;
                t.missed++;
                total_misses++;
                t.ready = false;
                t.remaining = 0;
            }
        }

        // Step 3: Pick which task to run
        Task *running = (mode == "EDF") ? pick_edf(tasks) : pick_rm(tasks);

        // Step 4: Execute one tick
        if (running) {
            cout << "  [t=" << tick << "] Task " << running->id << endl;
            running->remaining -= 1.0;
            if (running->remaining <= 0) {
                running->ready = false;
                running->completed++;
            }
        } else {
            cout << "  [t=" << tick << "] IDLE" << endl;
        }
    }

    // Print summary
    cout << "\nSummary:" << endl;
    cout << "  Task  Completed  Missed" << endl;
    for (auto &t : tasks) {
        cout << "    " << t.id << "       " << t.completed << "         " << t.missed << endl;
    }
    cout << "  Total misses: " << total_misses << endl;
    if (total_misses == 0)
        cout << "  All deadlines met!" << endl;
    else
        cout << "  Some deadlines were missed." << endl;
    cout << endl;
}

int main() {
    cout << "=== Real-Time Scheduler ===" << endl << endl;

    // Task set from the assignment PDF
    vector<Task> tasks = {
        //  id  exec  period  deadline  util      rem  abs_dl ready prio  comp miss
        {   1,  2.0,   5.0,    5.0,   2.0/5.0,   0,   0,    false,  0,   0,   0},
        {   2,  4.0,  10.0,   10.0,   4.0/10.0,  0,   0,    false,  0,   0,   0},
        {   3,  1.0,  20.0,   20.0,   1.0/20.0,  0,   0,    false,  0,   0,   0},
    };

    // Check schedulability before simulating
    check_schedulability(tasks);

    // Compute hyperperiod (LCM of all periods)
    int hp = compute_hyperperiod(tasks);
    cout << "Hyperperiod = " << hp << " ticks" << endl << endl;

    // Run RM simulation
    cout << string(50, '=') << endl;
    simulate(tasks, "RM", hp);

    // Run EDF simulation
    cout << string(50, '=') << endl;
    simulate(tasks, "EDF", hp);

    return 0;
}
