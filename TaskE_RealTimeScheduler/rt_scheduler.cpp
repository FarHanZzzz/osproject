/*
 * ============================================================
 * Task E — Real-Time Task Scheduler & Schedulability Analyzer
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 * ============================================================
 *
 * Supports two scheduling modes, selectable at runtime:
 *   RM  — Rate Monotonic       (static priority, shorter period → higher priority)
 *   EDF — Earliest Deadline First (dynamic priority, closest deadline first)
 *
 * Schedulability checks performed BEFORE simulation:
 *   RM  : U = Σ(Ci/Ti) ≤ n × (2^(1/n) − 1)   (≈ 0.693 for large n)
 *   EDF : U ≤ 1.0
 *
 * Built-in test case (from the assignment):
 *   τ1: Ci=2, Ti=5  → U=0.40
 *   τ2: Ci=4, Ti=10 → U=0.40
 *   τ3: Ci=1, Ti=20 → U=0.05
 *   Total U = 0.85  → NOT schedulable by RM (bound≈0.779), IS by EDF.
 *
 * Build:
 *   g++ -Wall -Wextra -std=c++17 -o rt_scheduler rt_scheduler.cpp
 *
 * OS Concepts demonstrated:
 *   - Static vs dynamic priority scheduling
 *   - Schedulability analysis (utilisation bounds)
 *   - Hyperperiod (LCM of all periods) — simulation length
 *   - Deadline miss detection
 *   - Preemption in EDF
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>
#include <numeric>   // std::lcm (C++17)
#include <cmath>
#include <climits>
#include <sstream>

/* ================================================================
 * Task — periodic real-time task
 * ================================================================ */
struct Task {
    int    id;
    double exec_time;    // Ci — worst-case execution time per period
    double period;       // Ti — recurrence interval
    double deadline;     // Di — relative deadline (= Ti in this project)
    double utilization;  // Ci / Ti

    /* Per-instance simulation state (reset each period) */
    double remaining_time    = 0.0;
    double absolute_deadline = 0.0;
    bool   ready             = false;
    int    static_priority   = 0;   // RM: lower number = higher priority

    /* Statistics */
    int instances_completed = 0;
    int deadline_misses     = 0;
};

/* ================================================================
 * GanttEntry — one time-unit of execution log
 * ================================================================ */
struct GanttEntry {
    int tick;
    int task_id;   // -1 = idle
    bool deadline_miss;
};

/* ================================================================
 * Utility: greatest common divisor / least common multiple
 * ================================================================ */
static long long gcd_ll(long long a, long long b)
{
    while (b) { a %= b; std::swap(a, b); }
    return a;
}
static long long lcm_ll(long long a, long long b)
{
    return (a / gcd_ll(a, b)) * b;
}

/* ================================================================
 * compute_hyper_period
 *   Returns LCM of all task periods (integer arithmetic).
 * ================================================================ */
static int compute_hyper_period(const std::vector<Task> &tasks)
{
    long long hp = 1;
    for (auto &t : tasks)
        hp = lcm_ll(hp, (long long)t.period);
    return (int)hp;
}

/* ================================================================
 * rm_utilisation_bound
 *   n × (2^(1/n) − 1)
 * ================================================================ */
static double rm_utilisation_bound(int n)
{
    return (double)n * (std::pow(2.0, 1.0 / (double)n) - 1.0);
}

/* ================================================================
 * schedulability_check
 * ================================================================ */
static void schedulability_check(const std::vector<Task> &tasks)
{
    int    n = (int)tasks.size();
    double U = 0.0;
    for (auto &t : tasks) U += t.utilization;

    double rm_bound  = rm_utilisation_bound(n);
    double edf_bound = 1.0;

    std::cout << "=== Schedulability Analysis ===\n";
    std::cout << std::fixed << std::setprecision(4);

    std::cout << "\nTask Set:\n";
    std::cout << std::setw(6)  << "Task"
              << std::setw(8)  << "Ci"
              << std::setw(8)  << "Ti"
              << std::setw(10) << "Ui\n";
    std::cout << std::string(32, '-') << "\n";
    for (auto &t : tasks) {
        std::cout << std::setw(6)  << ("τ" + std::to_string(t.id))
                  << std::setw(8)  << t.exec_time
                  << std::setw(8)  << t.period
                  << std::setw(10) << t.utilization << "\n";
    }
    std::cout << std::string(32, '-') << "\n";
    std::cout << std::setw(22) << "Total U =" << std::setw(10) << U << "\n\n";

    std::cout << "RM  Utilisation Bound  (n=" << n << "): " << rm_bound  << "\n";
    std::cout << "EDF Utilisation Bound       : " << edf_bound << "\n\n";

    bool rm_ok  = (U <= rm_bound);
    bool edf_ok = (U <= edf_bound);

    std::cout << "RM  schedulable : " << (rm_ok  ? "YES ✓" : "NO  ✗  (U exceeds bound)") << "\n";
    std::cout << "EDF schedulable : " << (edf_ok ? "YES ✓" : "NO  ✗  (U > 1.0, impossible)") << "\n\n";

    if (!edf_ok) {
        std::cout << "WARNING: Task set is over-utilised — no algorithm can schedule it.\n\n";
    } else if (!rm_ok) {
        std::cout << "NOTE: RM may miss deadlines for this task set.\n"
                  << "      Simulation will show which tasks miss in RM mode.\n\n";
    }
}

/* ================================================================
 * activate_tasks
 *   At each tick, re-activate tasks whose period has elapsed.
 * ================================================================ */
static void activate_tasks(std::vector<Task> &tasks, int tick)
{
    for (auto &t : tasks) {
        if (tick % (int)t.period == 0) {
            t.ready             = true;
            t.remaining_time    = t.exec_time;
            t.absolute_deadline = tick + t.period;
        }
    }
}

/* ================================================================
 * select_rm — pick highest static priority ready task
 * ================================================================ */
static Task *select_rm(std::vector<Task> &tasks)
{
    Task *best = nullptr;
    for (auto &t : tasks) {
        if (!t.ready || t.remaining_time <= 0) continue;
        if (!best || t.static_priority < best->static_priority)
            best = &t;
    }
    return best;
}

/* ================================================================
 * select_edf — pick task with smallest absolute deadline
 * ================================================================ */
static Task *select_edf(std::vector<Task> &tasks)
{
    Task *best = nullptr;
    for (auto &t : tasks) {
        if (!t.ready || t.remaining_time <= 0) continue;
        if (!best || t.absolute_deadline < best->absolute_deadline)
            best = &t;
    }
    return best;
}

/* ================================================================
 * run_simulation
 * ================================================================ */
static void run_simulation(std::vector<Task> tasks, const std::string &mode, int hyper_period)
{
    std::cout << "=== Simulation: " << mode << " mode (0 → " << hyper_period << " ticks) ===\n\n";

    /* Assign RM static priorities: shorter period → lower number → higher priority */
    if (mode == "RM") {
        std::vector<Task *> sorted;
        for (auto &t : tasks) sorted.push_back(&t);
        std::sort(sorted.begin(), sorted.end(),
                  [](Task *a, Task *b){ return a->period < b->period; });
        for (int i = 0; i < (int)sorted.size(); i++)
            sorted[i]->static_priority = i + 1;   // 1 = highest
    }

    std::vector<GanttEntry> gantt;
    int missed_total = 0;

    for (int tick = 0; tick < hyper_period; tick++) {

        /* 1. Activate tasks whose period starts now */
        activate_tasks(tasks, tick);

        /* 2. Check for deadline misses BEFORE scheduling */
        for (auto &t : tasks) {
            if (t.ready && t.remaining_time > 0 &&
                (double)tick >= t.absolute_deadline) {
                std::cout << "  [t=" << std::setw(3) << tick
                          << "] *** DEADLINE MISSED for τ"
                          << t.id << " (deadline was " << (int)t.absolute_deadline << ") ***\n";
                t.deadline_misses++;
                missed_total++;
                /* Mark instance as failed, reset */
                t.ready          = false;
                t.remaining_time = 0;
            }
        }

        /* 3. Select task */
        Task *running = (mode == "EDF") ? select_edf(tasks) : select_rm(tasks);

        /* 4. Execute one tick */
        if (running) {
            running->remaining_time -= 1.0;
            gantt.push_back({tick, running->id, false});

            if (running->remaining_time <= 0) {
                running->ready = false;
                running->instances_completed++;
            }
        } else {
            gantt.push_back({tick, -1, false});
        }
    }

    /* ---- Compact Gantt chart (group consecutive same-task runs) ---- */
    std::cout << "Gantt Chart (compact):\n";
    int i = 0;
    while (i < (int)gantt.size()) {
        int j = i;
        while (j < (int)gantt.size() && gantt[j].task_id == gantt[i].task_id)
            j++;
        if (gantt[i].task_id == -1)
            std::cout << "[t=" << std::setw(3) << gantt[i].tick
                      << "-" << std::setw(3) << j << "] IDLE\n";
        else
            std::cout << "[t=" << std::setw(3) << gantt[i].tick
                      << "-" << std::setw(3) << j << "] τ"
                      << gantt[i].task_id << "\n";
        i = j;
    }

    /* ---- Per-task summary ---- */
    std::cout << "\nTask Summary:\n";
    std::cout << std::setw(6)  << "Task"
              << std::setw(12) << "Instances"
              << std::setw(16) << "DL Misses\n";
    std::cout << std::string(34, '-') << "\n";
    for (auto &t : tasks) {
        std::cout << std::setw(6)  << ("τ" + std::to_string(t.id))
                  << std::setw(12) << t.instances_completed
                  << std::setw(16) << t.deadline_misses << "\n";
    }
    std::cout << std::string(34, '-') << "\n";
    std::cout << "Total deadline misses: " << missed_total << "\n";
    std::cout << (missed_total == 0 ? "✓ All deadlines met!\n" : "✗ Deadline misses occurred.\n");
    std::cout << "\n";
}

/* ================================================================
 * main
 * ================================================================ */
int main()
{
    std::cout << "=== Real-Time Scheduler & Schedulability Analyzer ===\n\n";

    /* ---- Built-in task set (from assignment specification) ---- */
    std::vector<Task> tasks = {
        /* id  exec  period  deadline  utilization */
        {  1,  2.0,   5.0,   5.0,   2.0/5.0  },
        {  2,  4.0,  10.0,  10.0,   4.0/10.0 },
        {  3,  1.0,  20.0,  20.0,   1.0/20.0 },
    };

    /* ---- Schedulability check ---- */
    schedulability_check(tasks);

    /* ---- Hyperperiod ---- */
    int hp = compute_hyper_period(tasks);
    std::cout << "Hyperperiod (LCM of periods): " << hp << " ticks\n\n";

    /* ---- Run both modes so the user can compare ---- */
    std::cout << std::string(60, '=') << "\n";
    run_simulation(tasks, "RM",  hp);

    std::cout << std::string(60, '=') << "\n";
    run_simulation(tasks, "EDF", hp);

    /* ---- Interactive mode selection ---- */
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Run again with custom mode? (RM/EDF/no): ";
    std::string choice;
    std::cin >> choice;

    if (choice == "RM" || choice == "rm" || choice == "EDF" || choice == "edf") {
        std::string mode = (choice == "rm") ? "RM" : choice;
        // Uppercase
        for (auto &c : mode) c = toupper(c);
        std::cout << "\n";
        run_simulation(tasks, mode, hp);
    } else {
        std::cout << "Done.\n";
    }

    return 0;
}
