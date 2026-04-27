/*
 * ELI5: Time-Sliced Multilevel Queue (The Fair Teacher)
 * ------------------------------------------------------
 * Imagine a teacher with three groups of students.
 * Group 1: Urgent questions (50% of time) - Round Robin (Teacher cycles through them quickly)
 * Group 2: Medium questions (30% of time) - Shortest Job First (Quickest questions go first)
 * Group 3: Slow projects    (20% of time) - First Come, First Served (Answer in order)
 * 
 * The teacher splits their time so everyone gets attention without anyone waiting forever!
 *
 * cd /home/farhan-sadeque/Downloads/Osproject/TaskD_MLQScheduler
 * g++ -std=c++11 -o mlq_scheduler mlq_scheduler.cpp
 * ./mlq_scheduler
 */

#include <algorithm> // Provides min(), sort() — basic operations on data
#include <deque>     // Double-ended queue — can add/remove from both front and back
#include <iomanip>   // setw(), setprecision() — makes printed tables look neat and aligned
#include <iostream>  // std::cout — lets us print text to the terminal
#include <string>    // std::string — for handling text like "G1", "G2"
#include <vector>    // std::vector — a resizable array (like a list that can grow)

// [OS CONCEPT: Process Control Block (PCB)]
// In a real OS, the PCB stores everything about a process: its state, ID, CPU time used, etc.
// Our Student struct is exactly a PCB — it holds all info the scheduler needs about each process.
struct Student {
    int id;               // Unique student number (like a Process ID / PID in the OS)
    int arrival_time;     // The clock tick when this student first shows up (process enters the system)
    int total_time_needed;// Total CPU burst time — how many minutes of help they need in total
    int time_left;        // Remaining burst time — how many minutes they STILL need (counts down to 0)
    int priority_score;   // Priority number — determines which group/queue they go into
    int first_helped_time;// The tick when the teacher first spoke to them (-1 = hasn't happened yet)
    int finished_time;    // The tick when they were completely done (-1 = still working)
    bool is_in_line;      // Whether they've already been placed into one of the 3 queues
};

// [OS CONCEPT: Priority-Based Queue Assignment]
// The OS kernel checks each process's priority to decide which ready queue it belongs to.
// This function maps a priority number to a group (queue) number.
int get_group_number(int priority_score) {
    if (priority_score <= 10) return 1; // Priority 0–10  → Group 1 (highest priority, system-level)
    if (priority_score <= 20) return 2; // Priority 11–20 → Group 2 (medium priority, interactive)
    return 3;                           // Priority 21–30 → Group 3 (lowest priority, batch jobs)
}

// [OS CONCEPT: Job Scheduler / Process Admission]
// When a new process arrives (its arrival_time <= current clock), the OS moves it into the
// appropriate ready queue. This function does exactly that — it scans all students and
// places newly arrived ones into their group's line.
void check_for_new_arrivals(
    std::vector<Student> &classroom,  // The full list of all students (all processes)
    int current_minute,               // The current clock tick (simulation time)
    std::deque<int> &group1,          // Queue for Group 1 (Round Robin)
    std::vector<int> &group2,         // Queue for Group 2 (SJF) — vector so we can scan & pick shortest
    std::deque<int> &group3)          // Queue for Group 3 (FCFS)
{
    for (size_t i = 0; i < classroom.size(); i++) {          // Loop through every student
        if (!classroom[i].is_in_line &&                       // Not already in a queue
            classroom[i].time_left > 0 &&                     // Still has work remaining
            classroom[i].arrival_time <= current_minute) {    // Has arrived by now

            int group = get_group_number(classroom[i].priority_score); // Determine their group

            if (group == 1) group1.push_back((int)i);         // Add their INDEX to Group 1's queue
            else if (group == 2) group2.push_back((int)i);    // Add their INDEX to Group 2's queue
            else group3.push_back((int)i);                    // Add their INDEX to Group 3's queue

            classroom[i].is_in_line = true;                   // Mark them so we don't add them twice
        }
    }
}

// [OS CONCEPT: CPU Execution & Time Slicing]
// The CPU runs a process for exactly 1 clock tick. This function simulates that:
// it records the first time the process got the CPU (response time), decrements
// remaining time, logs the action, and advances the clock.
void help_student_for_one_minute(
    std::vector<Student> &classroom,  // Full student list (to update their state)
    int student_index,                // Which student (process) is being helped right now
    int &current_minute,              // Reference to the clock — we advance it by 1
    std::vector<int> &timeline_record)// Gantt chart log — records which PID ran at each tick
{
    if (classroom[student_index].first_helped_time == -1) {   // Is this the FIRST time they got the CPU?
        classroom[student_index].first_helped_time = current_minute; // Record it (used for Response Time)
    }

    classroom[student_index].time_left--;                     // One tick of work done — decrement remaining burst
    timeline_record.push_back(classroom[student_index].id);   // Log this student's ID in the Gantt chart
    current_minute++;                                         // Advance the simulation clock by 1 tick
}

// ============================================================
// [OS CONCEPT: Round Robin (RR) Scheduling]
// Each process gets a fixed maximum time slice (quantum = 2 ticks).
// If it doesn't finish within its quantum, it goes to the BACK of the queue
// and the next process gets a turn. This ensures fairness — no process hogs the CPU.
// ============================================================
bool teach_group1(
    std::vector<Student> &classroom,   // All students (processes)
    std::deque<int> &group1,           // Group 1's ready queue (double-ended for front/back ops)
    std::vector<int> &timeline,        // Gantt chart log
    int &time,                         // Current clock tick (passed by reference, gets updated)
    int max_time_allowed,              // Budget: how many ticks G1 is allowed this cycle (5)
    int cycle_limit,                   // Quantum: max ticks per student before rotation (2)
    int &students_finished,            // Counter of completed processes (passed by ref)
    std::vector<int> &group2,          // G2 queue — passed so we can check for new arrivals into it
    std::deque<int> &group3)           // G3 queue — same reason
{
    int time_spent = 0;                // Tracks how much of G1's budget has been used so far
    bool helped_someone = false;       // Did G1 do any work this cycle? (for idle detection)

    while (time_spent < max_time_allowed && !group1.empty()) { // Keep going while budget remains AND queue isn't empty
        int current_student = group1.front(); // Take the student at the FRONT of the queue (FIFO order)
        group1.pop_front();                   // Remove them from the front — they're being served now

        // Calculate how many minutes to help them:
        // It's the MINIMUM of: quantum (2), remaining budget, and their remaining work
        int minutes_to_talk = cycle_limit;                                                    // Start with quantum (2)
        if (minutes_to_talk > max_time_allowed - time_spent) minutes_to_talk = max_time_allowed - time_spent; // Don't exceed budget
        if (minutes_to_talk > classroom[current_student].time_left) minutes_to_talk = classroom[current_student].time_left; // Don't exceed their remaining work

        for (int i = 0; i < minutes_to_talk; i++) {                              // Run for the calculated number of ticks
            help_student_for_one_minute(classroom, current_student, time, timeline); // Execute 1 tick on this process
            time_spent++;                                                          // Deduct 1 from G1's budget
            helped_someone = true;                                                 // G1 was productive this cycle
            check_for_new_arrivals(classroom, time, group1, group2, group3);       // After each tick, check if anyone new arrived
        }

        if (classroom[current_student].time_left == 0) {       // Did this student finish all their work?
            classroom[current_student].finished_time = time;    // Record their completion time
            classroom[current_student].is_in_line = false;      // They're done — no longer in any queue
            students_finished++;                                // Increment the global finished counter
        } else {
            group1.push_back(current_student);                  // NOT done → send to the BACK of G1 (Round Robin!)
        }
    }
    return helped_someone;                                      // Tell the caller whether G1 did any work
}

// ============================================================
// [OS CONCEPT: Shortest Job First (SJF) Scheduling]
// The scheduler scans ALL processes in the queue and picks the one with the
// SMALLEST remaining time. This minimizes average waiting time.
// It's non-preemptive within the budget — once chosen, the process runs
// until it finishes or the budget runs out.
// ============================================================
bool teach_group2(
    std::vector<Student> &classroom,   // All students (processes)
    std::vector<int> &group2,          // Group 2's queue — a vector so we can scan & erase by index
    std::vector<int> &timeline,        // Gantt chart log
    int &time,                         // Current clock tick
    int max_time_allowed,              // Budget: how many ticks G2 is allowed this cycle (3)
    int &students_finished,            // Counter of completed processes
    std::deque<int> &group1,           // G1 queue — for checking new arrivals
    std::deque<int> &group3)           // G3 queue — for checking new arrivals
{
    int time_spent = 0;                // How much of G2's budget has been consumed
    bool helped_someone = false;       // Did G2 do any work this cycle?

    while (time_spent < max_time_allowed && !group2.empty()) { // While budget remains AND queue has students

        // --- SCAN for the shortest job ---
        int best_person_index = 0;                             // Assume the first student is the shortest
        for (size_t i = 1; i < group2.size(); i++) {           // Compare against every other student in G2
            if (classroom[group2[i]].time_left < classroom[group2[best_person_index]].time_left) { // Found someone shorter?
                best_person_index = i;                         // Update our pick to this student
            }
        }

        int current_student = group2[best_person_index];       // Get the actual student index from the queue

        // Calculate how long to help: minimum of their remaining work and remaining budget
        int minutes_to_talk = classroom[current_student].time_left;                           // Start with their full remaining time
        if (minutes_to_talk > max_time_allowed - time_spent) minutes_to_talk = max_time_allowed - time_spent; // Cap at remaining budget

        for (int i = 0; i < minutes_to_talk; i++) {                              // Run for the calculated ticks
            help_student_for_one_minute(classroom, current_student, time, timeline); // Execute 1 tick
            time_spent++;                                                          // Deduct from budget
            helped_someone = true;                                                 // G2 was productive
            check_for_new_arrivals(classroom, time, group1, group2, group3);       // Check for new arrivals after each tick
        }

        if (classroom[current_student].time_left == 0) {       // Did this student finish?
            classroom[current_student].finished_time = time;    // Record completion time
            classroom[current_student].is_in_line = false;      // Remove from queue tracking
            students_finished++;                                // Increment finished counter
            group2.erase(group2.begin() + best_person_index);   // Remove them from G2's vector
        }
        // If NOT done, they stay in group2 — SJF will re-scan and may pick them or someone shorter next
    }
    return helped_someone;                                      // Report whether G2 did any work
}

// ============================================================
// [OS CONCEPT: First Come, First Served (FCFS) Scheduling]
// The simplest possible algorithm: processes run in the exact order they arrived.
// No reordering, no preemption within the budget. The first process in line
// runs until it finishes or the budget is exhausted.
// ============================================================
bool teach_group3(
    std::vector<Student> &classroom,   // All students (processes)
    std::deque<int> &group3,           // Group 3's queue — deque for efficient front operations
    std::vector<int> &timeline,        // Gantt chart log
    int &time,                         // Current clock tick
    int max_time_allowed,              // Budget: how many ticks G3 is allowed this cycle (2)
    int &students_finished,            // Counter of completed processes
    std::deque<int> &group1,           // G1 queue — for checking new arrivals
    std::vector<int> &group2)          // G2 queue — for checking new arrivals
{
    int time_spent = 0;                // How much of G3's budget has been consumed
    bool helped_someone = false;       // Did G3 do any work this cycle?

    while (time_spent < max_time_allowed && !group3.empty()) { // While budget remains AND queue has students
        int current_student = group3.front();                  // Always take the FIRST person in line (arrival order)

        // Calculate how long to help: minimum of remaining work and remaining budget
        int minutes_to_talk = classroom[current_student].time_left;                           // Their full remaining time
        if (minutes_to_talk > max_time_allowed - time_spent) minutes_to_talk = max_time_allowed - time_spent; // Cap at budget

        for (int i = 0; i < minutes_to_talk; i++) {                              // Run for calculated ticks
            help_student_for_one_minute(classroom, current_student, time, timeline); // Execute 1 tick
            time_spent++;                                                          // Deduct from budget
            helped_someone = true;                                                 // G3 was productive
            check_for_new_arrivals(classroom, time, group1, group2, group3);       // Check for new arrivals
        }

        if (classroom[current_student].time_left == 0) {       // Did this student finish?
            classroom[current_student].finished_time = time;    // Record completion time
            classroom[current_student].is_in_line = false;      // Remove from queue tracking
            students_finished++;                                // Increment finished counter
            group3.pop_front();                                 // Remove from front of G3 (they're done)
        }
        // If NOT done, they STAY at the front — FCFS means same person continues next cycle
    }
    return helped_someone;                                      // Report whether G3 did any work
}

// ============================================================
// MAIN — The simulation entry point
// ============================================================
int main() {
    // ---- Define our classroom of students (the process table) ----
    // Each entry: {id, arrival_time, total_time_needed, time_left, priority_score, first_helped, finished, in_line}
    std::vector<Student> classroom = {
        {1,  0,  7,  7,   5,  -1, -1, false},   // S1: arrives t=0, needs 7 min, priority 5  → Group 1 (RR)
        {2,  1,  4,  4,  12,  -1, -1, false},   // S2: arrives t=1, needs 4 min, priority 12 → Group 2 (SJF)
        {3,  2,  8,  8,  25,  -1, -1, false},   // S3: arrives t=2, needs 8 min, priority 25 → Group 3 (FCFS)
        {4,  3,  5,  5,   8,  -1, -1, false},   // S4: arrives t=3, needs 5 min, priority 8  → Group 1 (RR)
        {5,  5,  3,  3,  18,  -1, -1, false},   // S5: arrives t=5, needs 3 min, priority 18 → Group 2 (SJF)
        {6,  6,  6,  6,  27,  -1, -1, false}    // S6: arrives t=6, needs 6 min, priority 27 → Group 3 (FCFS)
    };

    // ---- Time-slice configuration (how the teacher divides each 10-minute cycle) ----
    const int group1_time_allowed  = 5;   // Group 1 gets 5 out of 10 ticks = 50% of CPU time
    const int group2_time_allowed  = 3;   // Group 2 gets 3 out of 10 ticks = 30% of CPU time
    const int group3_time_allowed  = 2;   // Group 3 gets 2 out of 10 ticks = 20% of CPU time
    const int group1_cycle_limit   = 2;   // RR quantum for Group 1: max 2 ticks per student before rotation

    int clock_time = 0;                   // The simulation clock — starts at tick 0
    int students_finished = 0;            // How many students have completed — stopping condition
    std::vector<int> timeline;            // Gantt chart: stores which student ID ran at each tick

    std::deque<int>  group1_line;         // Group 1 ready queue (deque for efficient front/back ops)
    std::vector<int> group2_line;         // Group 2 ready queue (vector so we can scan for shortest job)
    std::deque<int>  group3_line;         // Group 3 ready queue (deque for FCFS front access)

    // ============================================================
    // [OS CONCEPT: Multilevel Queue (MLQ) Scheduler — Main Loop]
    // The OS short-term scheduler continuously cycles through the 3 queues,
    // giving each its allotted time budget, then repeating.
    // Loop: G1(5 ticks) → G2(3 ticks) → G3(2 ticks) → G1 → G2 → G3 → ...
    // ============================================================
    while (students_finished < (int)classroom.size()) {        // Keep looping until ALL 6 students are done

        // First: check if any new students have arrived and add them to their queue
        check_for_new_arrivals(classroom, clock_time, group1_line, group2_line, group3_line);

        bool teacher_was_busy = false;                         // Track if ANY group did work this cycle

        // Serve Group 1 first (highest priority, Round Robin, 5-tick budget)
        teacher_was_busy = teach_group1(classroom, group1_line, timeline, clock_time,
                                        group1_time_allowed, group1_cycle_limit,
                                        students_finished, group2_line, group3_line)
                           || teacher_was_busy;

        // Then serve Group 2 (medium priority, Shortest Job First, 3-tick budget)
        teacher_was_busy = teach_group2(classroom, group2_line, timeline, clock_time,
                                        group2_time_allowed,
                                        students_finished, group1_line, group3_line)
                           || teacher_was_busy;

        // Finally serve Group 3 (lowest priority, FCFS, 2-tick budget)
        teacher_was_busy = teach_group3(classroom, group3_line, timeline, clock_time,
                                        group3_time_allowed,
                                        students_finished, group1_line, group2_line)
                           || teacher_was_busy;

        // If ALL three queues were empty (no one to serve), the teacher idles for 1 tick
        if (!teacher_was_busy) {
            timeline.push_back(-1);                            // -1 in Gantt chart = IDLE (no process ran)
            clock_time++;                                      // Advance the clock by 1 tick
        }
    }

    // ============================================================
    // OUTPUT SECTION: Print the Gantt Chart (timeline)
    // ============================================================
    std::cout << "Timeline of who the teacher helped every minute ('-' = drinking coffee):\n";
    for (size_t t = 0; t < timeline.size(); t++) {             // Loop through every tick in the timeline
        if (timeline[t] == -1) std::cout << "- ";              // -1 = idle tick, print a dash
        else std::cout << "S" << timeline[t] << " ";           // Otherwise print the student ID
    }
    std::cout << "\n\n";                                       // Blank line after the Gantt chart

    // ============================================================
    // OUTPUT SECTION: Print the Report Card (performance metrics table)
    // ============================================================
    std::cout << std::left                                     // Left-align all columns
              << std::setw(6)  << "ID"                         // Column: Student ID
              << std::setw(7)  << "Group"                      // Column: Which group (G1/G2/G3)
              << std::setw(8)  << "Arrived"                    // Column: Arrival time
              << std::setw(8)  << "Needed"                     // Column: Total burst time
              << std::setw(9)  << "Finished"                   // Column: Completion time
              << std::setw(9)  << "Wait"                       // Column: Waiting time
              << std::setw(12) << "Turnaround"                 // Column: Turnaround time
              << std::setw(10) << "Response"                   // Column: Response time
              << "\n";

    double total_wait = 0, total_turnaround = 0, total_response = 0; // Accumulators for computing averages

    for (const auto &student : classroom) {                    // Loop through every student to compute their metrics

        // [OS CONCEPT: Performance Metrics]
        // Turnaround = finish_time - arrival_time           (total time spent in the system)
        // Waiting    = turnaround - total_time_needed       (time spent NOT being served)
        // Response   = first_helped_time - arrival_time     (delay before first CPU access)
        int turnaround = student.finished_time - student.arrival_time;      // Total time in system
        int waiting    = turnaround - student.total_time_needed;            // Time spent just waiting
        int response   = student.first_helped_time - student.arrival_time;  // Delay before first service

        std::string group_name = "G" + std::to_string(get_group_number(student.priority_score)); // "G1", "G2", or "G3"

        std::cout << std::left                                 // Print one row of the metrics table
                  << std::setw(6)  << student.id               // Student ID
                  << std::setw(7)  << group_name               // Group name
                  << std::setw(8)  << student.arrival_time      // When they arrived
                  << std::setw(8)  << student.total_time_needed // How much help they needed
                  << std::setw(9)  << student.finished_time     // When they finished
                  << std::setw(9)  << waiting                   // How long they waited
                  << std::setw(12) << turnaround                // Total turnaround
                  << std::setw(10) << response                  // Response time
                  << "\n";

        total_wait += waiting;                                 // Add to running total for averaging
        total_turnaround += turnaround;                        // Add to running total
        total_response += response;                            // Add to running total
    }

    // ============================================================
    // OUTPUT SECTION: Print class-wide averages
    // ============================================================
    int total_students = (int)classroom.size();                // Total number of students (6)
    std::cout << "\nClass Averages:\n";
    std::cout << std::fixed << std::setprecision(2);           // Show exactly 2 decimal places
    std::cout << "  Avg Waiting Time    : " << total_wait / total_students << " minutes\n";       // Mean waiting time
    std::cout << "  Avg Turnaround Time : " << total_turnaround / total_students << " minutes\n"; // Mean turnaround time
    std::cout << "  Avg Response Time   : " << total_response / total_students << " minutes\n";   // Mean response time

    return 0; // Program finished successfully — school day is over!
}
