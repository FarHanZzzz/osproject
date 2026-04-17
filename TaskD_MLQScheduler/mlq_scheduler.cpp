/*
 * ============================================================
 * Task D — Time-Sliced Multilevel Queue (MLQ) Scheduler
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 * ============================================================
 *
 * Configuration:
 *   Queue 1 (System,      priority 0–10)  : Round Robin, quantum = 2
 *   Queue 2 (Interactive, priority 11–20) : Shortest Job First (non-preemptive)
 *   Queue 3 (Batch,       priority 21–30) : FCFS
 *
 *   Time-slice cycle (per 10 ticks):
 *     Q1 → 5 ticks | Q2 → 3 ticks | Q3 → 2 ticks
 *
 * Build:
 *   g++ -Wall -Wextra -std=c++17 -o mlq_scheduler mlq_scheduler.cpp
 *
 * OS Concepts demonstrated:
 *   - MLQ vs MLFQ (no demotion/promotion in MLQ)
 *   - Inter-queue time slicing (prevents starvation of lower queues)
 *   - RR, SJF, FCFS algorithms within each queue
 *   - Waiting Time, Turnaround Time, Response Time metrics
 * ============================================================
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>
#include <deque>
#include <climits>

/* ================================================================
 * Process structure
 * ================================================================ */
struct Process {
    int  pid;
    int  arrival_time;
    int  burst_time;
    int  remaining_time;
    int  priority;          // 0–10 = Q1, 11–20 = Q2, 21–30 = Q3

    /* Metrics — filled in during simulation */
    int  start_time      = -1;   // first time process got CPU
    int  completion_time =  0;
    int  waiting_time    =  0;
    int  turnaround_time =  0;
    int  response_time   =  0;

    std::string queue_name;
};

/* ================================================================
 * Gantt chart entry
 * ================================================================ */
struct GanttSlot {
    int         pid;        // -1 = idle
    int         tick;
    std::string queue;
};

/* ================================================================
 * Global simulation state
 * ================================================================ */
static std::vector<Process>   all_processes;
static std::vector<GanttSlot> gantt;
static int current_time = 0;

/* ----------------------------------------------------------------
 * classify_queue — returns queue index (1/2/3) and sets queue_name
 * ---------------------------------------------------------------- */
static int classify_queue(Process &p)
{
    if (p.priority <= 10) { p.queue_name = "Q1-System";      return 1; }
    if (p.priority <= 20) { p.queue_name = "Q2-Interactive"; return 2; }
                            p.queue_name = "Q3-Batch";        return 3;
}

/* ----------------------------------------------------------------
 * arrive — collect processes that have arrived by current_time
 * ---------------------------------------------------------------- */
static void collect_arrived(std::deque<Process *> &q, int queue_id)
{
    for (auto &p : all_processes) {
        if (p.arrival_time <= current_time &&
            p.remaining_time > 0 &&
            classify_queue(p) == queue_id) {
            /* Only add if not already in deque */
            bool found = false;
            for (auto *ptr : q) if (ptr->pid == p.pid) { found = true; break; }
            if (!found) q.push_back(&p);
        }
    }
}

/* ----------------------------------------------------------------
 * run_tick — execute one tick of a process
 * ---------------------------------------------------------------- */
static void run_tick(Process *p, const std::string &qname)
{
    if (p->start_time == -1) p->start_time = current_time;
    p->remaining_time--;
    gantt.push_back({p->pid, current_time, qname});
    current_time++;

    if (p->remaining_time == 0) {
        p->completion_time  = current_time;
        p->turnaround_time  = p->completion_time - p->arrival_time;
        p->waiting_time     = p->turnaround_time - p->burst_time;
        p->response_time    = p->start_time      - p->arrival_time;
    }
}

static void run_idle()
{
    gantt.push_back({-1, current_time, "idle"});
    current_time++;
}

/* ================================================================
 * run_Q1_RR — Round Robin, quantum = 2
 * ================================================================ */
static void run_Q1_RR(int time_budget)
{
    const int QUANTUM = 2;
    std::deque<Process *> ready;

    int spent = 0;
    while (spent < time_budget) {
        collect_arrived(ready, 1);

        if (ready.empty()) {
            run_idle();
            spent++;
            continue;
        }

        Process *p = ready.front();
        ready.pop_front();

        int run_for = std::min({QUANTUM, p->remaining_time, time_budget - spent});
        for (int i = 0; i < run_for; i++) {
            run_tick(p, "Q1");
            spent++;
            /* collect newly arrived processes after each tick */
            collect_arrived(ready, 1);
        }

        /* If not finished, push back to end of queue (Round Robin) */
        if (p->remaining_time > 0)
            ready.push_back(p);
    }
}

/* ================================================================
 * run_Q2_SJF — Shortest Job First (non-preemptive)
 * ================================================================ */
static void run_Q2_SJF(int time_budget)
{
    int spent = 0;
    while (spent < time_budget) {
        /* Collect all arrived, then pick shortest */
        std::vector<Process *> ready;
        for (auto &p : all_processes) {
            if (p.arrival_time <= current_time &&
                p.remaining_time > 0 &&
                classify_queue(p) == 2) {
                ready.push_back(&p);
            }
        }

        if (ready.empty()) {
            run_idle();
            spent++;
            continue;
        }

        /* Sort by remaining_time (SJF picks shortest) */
        std::sort(ready.begin(), ready.end(),
                  [](Process *a, Process *b) {
                      return a->remaining_time < b->remaining_time;
                  });

        Process *p = ready[0];
        /* Run to completion (non-preemptive) but don't exceed budget */
        int run_for = std::min(p->remaining_time, time_budget - spent);
        for (int i = 0; i < run_for; i++) {
            run_tick(p, "Q2");
            spent++;
        }
    }
}

/* ================================================================
 * run_Q3_FCFS — First Come First Served
 * ================================================================ */
static void run_Q3_FCFS(int time_budget)
{
    int spent = 0;
    while (spent < time_budget) {
        /* Find earliest-arrived unfinished Q3 process */
        Process *p = nullptr;
        for (auto &proc : all_processes) {
            if (proc.arrival_time <= current_time &&
                proc.remaining_time > 0 &&
                classify_queue(proc) == 3) {
                if (!p || proc.arrival_time < p->arrival_time)
                    p = &proc;
            }
        }

        if (!p) {
            run_idle();
            spent++;
            continue;
        }

        int run_for = std::min(p->remaining_time, time_budget - spent);
        for (int i = 0; i < run_for; i++) {
            run_tick(p, "Q3");
            spent++;
        }
    }
}

/* ================================================================
 * all_done — true when every process has finished
 * ================================================================ */
static bool all_done()
{
    for (auto &p : all_processes)
        if (p.remaining_time > 0) return false;
    return true;
}

/* ================================================================
 * print_gantt
 * ================================================================ */
static void print_gantt()
{
    std::cout << "\n=== Gantt Chart ===\n";

    /* Print process row */
    for (auto &slot : gantt) {
        if (slot.pid == -1)
            std::cout << "|idle";
        else
            std::cout << "| P" << slot.pid;
    }
    std::cout << "|\n";

    /* Print time marks every 5 ticks */
    int t = 0;
    for (size_t i = 0; i < gantt.size(); i++, t++) {
        if (i % 5 == 0)
            std::cout << std::setw(5) << t;
    }
    std::cout << std::setw(5) << t << "\n";

    /* Print queue assignments */
    std::cout << "\nQueue assignments:\n";
    for (auto &slot : gantt) {
        if (slot.pid == -1)
            std::cout << "[t=" << std::setw(3) << slot.tick << "] IDLE\n";
        else
            std::cout << "[t=" << std::setw(3) << slot.tick
                      << "] P" << slot.pid
                      << " (" << slot.queue << ")\n";
    }
}

/* ================================================================
 * print_metrics
 * ================================================================ */
static void print_metrics()
{
    std::cout << "\n=== Process Metrics ===\n";
    std::cout << std::left
              << std::setw(5)  << "PID"
              << std::setw(8)  << "Queue"
              << std::setw(9)  << "Arrival"
              << std::setw(7)  << "Burst"
              << std::setw(13) << "Completion"
              << std::setw(12) << "Turnaround"
              << std::setw(10) << "Waiting"
              << std::setw(10) << "Response"
              << "\n";
    std::cout << std::string(74, '-') << "\n";

    double total_tat = 0, total_wt = 0, total_rt = 0;
    int count = 0;

    for (auto &p : all_processes) {
        std::cout << std::setw(5)  << p.pid
                  << std::setw(8)  << p.queue_name.substr(0,2)
                  << std::setw(9)  << p.arrival_time
                  << std::setw(7)  << p.burst_time
                  << std::setw(13) << p.completion_time
                  << std::setw(12) << p.turnaround_time
                  << std::setw(10) << p.waiting_time
                  << std::setw(10) << p.response_time
                  << "\n";
        total_tat += p.turnaround_time;
        total_wt  += p.waiting_time;
        total_rt  += p.response_time;
        count++;
    }

    std::cout << std::string(74, '-') << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Averages → Turnaround: " << total_tat/count
              << "  Waiting: "  << total_wt/count
              << "  Response: " << total_rt/count << "\n";
}

/* ================================================================
 * main
 * ================================================================ */
int main()
{
    std::cout << "=== Time-Sliced MLQ Scheduler ===\n";
    std::cout << "  Q1 (System,      priority  0-10): Round Robin (quantum=2), 50% CPU\n";
    std::cout << "  Q2 (Interactive, priority 11-20): SJF (non-preemptive),    30% CPU\n";
    std::cout << "  Q3 (Batch,       priority 21-30): FCFS,                    20% CPU\n\n";

    /* ---- Sample process set: 9 processes across all 3 queues ---- */
    all_processes = {
        /*  pid  arrival  burst  remaining  priority  start compl wait tat  resp  queue_name */
        {  1,    0,       6,     6,         5,        -1,   0,    0,   0,   0,    "" },  // Q1
        {  2,    1,       4,     4,         3,        -1,   0,    0,   0,   0,    "" },  // Q1
        {  3,    2,       8,     8,         15,       -1,   0,    0,   0,   0,    "" },  // Q2
        {  4,    0,       5,     5,         18,       -1,   0,    0,   0,   0,    "" },  // Q2
        {  5,    3,       3,     3,         25,       -1,   0,    0,   0,   0,    "" },  // Q3
        {  6,    1,       7,     7,         28,       -1,   0,    0,   0,   0,    "" },  // Q3
        {  7,    4,       2,     2,         8,        -1,   0,    0,   0,   0,    "" },  // Q1
        {  8,    2,       6,     6,         12,       -1,   0,    0,   0,   0,    "" },  // Q2
        {  9,    5,       4,     4,         22,       -1,   0,    0,   0,   0,    "" },  // Q3
    };

    /* Classify & display input */
    std::cout << "Input Processes:\n";
    std::cout << std::setw(5) << "PID" << std::setw(9) << "Arrival"
              << std::setw(7) << "Burst" << std::setw(10) << "Priority"
              << std::setw(18) << "Queue\n";
    std::cout << std::string(49, '-') << "\n";
    for (auto &p : all_processes) {
        classify_queue(p);
        std::cout << std::setw(5) << p.pid << std::setw(9) << p.arrival_time
                  << std::setw(7) << p.burst_time << std::setw(10) << p.priority
                  << std::setw(18) << p.queue_name << "\n";
    }
    std::cout << "\n";

    /* ---- Time-sliced simulation loop ---- */
    /* Cycle: Q1=5 ticks, Q2=3 ticks, Q3=2 ticks → repeat */
    const int BUDGET_Q1 = 5;
    const int BUDGET_Q2 = 3;
    const int BUDGET_Q3 = 2;
    const int MAX_TICKS = 200; /* safety cap */

    while (!all_done() && current_time < MAX_TICKS) {
        run_Q1_RR  (BUDGET_Q1);
        if (all_done()) break;
        run_Q2_SJF (BUDGET_Q2);
        if (all_done()) break;
        run_Q3_FCFS(BUDGET_Q3);
    }

    /* ---- Output ---- */
    print_gantt();
    print_metrics();

    return 0;
}
