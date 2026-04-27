/*
 * ELI5: Time-Sliced Multilevel Queue (The Fair Teacher)
 * ------------------------------------------------------
 * Imagine a teacher with three groups of students.
 * Group 1: Urgent questions (50% of time) - Round Robin (Teacher cycles through them quickly)
 * Group 2: Medium questions (30% of time) - Shortest Job First (Quickest questions go first)
 * Group 3: Slow projects    (20% of time) - First Come, First Served (Answer in order)
 * 
 * The teacher splits their time so everyone gets attention without anyone waiting forever!
 */

#include <algorithm> // For basic data operations
#include <deque>     // A double-ended line (queue) of people
#include <iomanip>   // For making our printed tables look neat
#include <iostream>  // For printing to the screen
#include <string>    // For handling text
#include <vector>    // An expandable list

// [OS CONCEPT: Process Control Block (PCB)]
// In an OS, the PCB stores all information about a specific process. Our "Student" struct
// acts exactly like a PCB, holding their ID, arrival time, and execution state.
// A "Student" represents a computer Process waiting for the CPU (the Teacher)
struct Student {
    int id;               // Student's ID number
    int arrival_time;     // The exact minute they walked in
    int total_time_needed;// Total minutes they need with the teacher
    int time_left;        // Minutes they STILL need (starts equal to total_time_needed)
    int priority_score;   // Decides their group (0-10 = Group 1, 11-20 = Group 2, etc.)
    int first_helped_time;// The minute the teacher first talked to them (-1 means not yet)
    int finished_time;    // The minute their question was completely answered (-1 means not yet)
    bool is_in_line;      // Are they currently standing in one of the 3 lines?
};

// [OS CONCEPT: Priority-Based Queue Assignment]
// The OS checks the priority score to decide which queue (Group) the process belongs to.
// Helper to decide which group a student belongs to based on their priority score
int get_group_number(int priority_score) {
    if (priority_score <= 10) return 1; // High priority = Group 1
    if (priority_score <= 20) return 2; // Medium priority = Group 2
    return 3;                           // Low priority = Group 3
}

// [OS CONCEPT: Job Scheduler / Process Admission]
// The OS constantly checks if new processes have arrived and adds them to the ready queues.
// Check the classroom door to see if any new students just arrived
void check_for_new_arrivals(
    std::vector<Student> &classroom, 
    int current_minute, 
    std::deque<int> &group1, 
    std::vector<int> &group2, 
    std::deque<int> &group3) 
{
    // Look at every student in the classroom
    for (size_t i = 0; i < classroom.size(); i++) {
        // If they aren't in line yet, still need help, AND they have arrived...
        if (!classroom[i].is_in_line && classroom[i].time_left > 0 && classroom[i].arrival_time <= current_minute) {
            
            // Figure out which group they belong to
            int group = get_group_number(classroom[i].priority_score);
            
            // Put them in the correct line
            if (group == 1) group1.push_back((int)i);
            else if (group == 2) group2.push_back((int)i);
            else group3.push_back((int)i);
            
            // Mark them as officially standing in line
            classroom[i].is_in_line = true;
        }
    }
}

// [OS CONCEPT: CPU Execution & Time Slicing]
// The CPU executes a process for a unit of time (1 tick/minute).
// The teacher spends exactly 1 minute helping a specific student
void help_student_for_one_minute(std::vector<Student> &classroom, int student_index, int &current_minute, std::vector<int> &timeline_record) {
    // If this is the VERY FIRST time the teacher is talking to them, record the time
    if (classroom[student_index].first_helped_time == -1) {
        classroom[student_index].first_helped_time = current_minute;
    }
    
    // They need 1 minute less of help now
    classroom[student_index].time_left--;
    
    // Write down in our logbook that we helped this student this minute
    timeline_record.push_back(classroom[student_index].id);
    
    // Time moves forward by 1 minute
    current_minute++;
}

// [OS CONCEPT: Round Robin (RR) Scheduling]
// Processes take turns. Each gets a strict maximum time slice (quantum) before going to the back of the queue.
// Group 1: Round Robin (The teacher cycles through students quickly)
bool teach_group1(
    std::vector<Student> &classroom, std::deque<int> &group1, std::vector<int> &timeline, 
    int &time, int max_time_allowed, int cycle_limit, int &students_finished, 
    std::vector<int> &group2, std::deque<int> &group3) 
{
    int time_spent = 0;
    bool helped_someone = false;

    // Keep going until we run out of allowed time OR the line is empty
    while (time_spent < max_time_allowed && !group1.empty()) {
        int current_student = group1.front(); // Get the first person in line
        group1.pop_front();                   // Remove them from the front

        // Figure out how long to talk to them (either cycle limit, time left, or max allowed time)
        int minutes_to_talk = cycle_limit;
        if (minutes_to_talk > max_time_allowed - time_spent) minutes_to_talk = max_time_allowed - time_spent;
        if (minutes_to_talk > classroom[current_student].time_left) minutes_to_talk = classroom[current_student].time_left;

        // Help them for the calculated minutes
        for (int i = 0; i < minutes_to_talk; i++) {
            help_student_for_one_minute(classroom, current_student, time, timeline);
            time_spent++;
            helped_someone = true;
            // Check if anyone new walked in while we were talking
            check_for_new_arrivals(classroom, time, group1, group2, group3);
        }

        // Are they completely done?
        if (classroom[current_student].time_left == 0) {
            classroom[current_student].finished_time = time;
            classroom[current_student].is_in_line = false;
            students_finished++;
        } else {
            // Not done yet? Go to the back of the line!
            group1.push_back(current_student);
        }
    }
    return helped_someone;
}

// [OS CONCEPT: Shortest Job First (SJF) Scheduling]
// The OS scans the queue and executes the process that requires the least remaining time.
// Group 2: Shortest Job First (The teacher picks the quickest question to answer first)
bool teach_group2(
    std::vector<Student> &classroom, std::vector<int> &group2, std::vector<int> &timeline, 
    int &time, int max_time_allowed, int &students_finished, 
    std::deque<int> &group1, std::deque<int> &group3) 
{
    int time_spent = 0;
    bool helped_someone = false;

    while (time_spent < max_time_allowed && !group2.empty()) {
        // Look through the whole line to find the person with the shortest question
        int best_person_index = 0;
        for (size_t i = 1; i < group2.size(); i++) {
            if (classroom[group2[i]].time_left < classroom[group2[best_person_index]].time_left) {
                best_person_index = i;
            }
        }

        int current_student = group2[best_person_index];

        // Figure out how long to talk to them
        int minutes_to_talk = classroom[current_student].time_left;
        if (minutes_to_talk > max_time_allowed - time_spent) minutes_to_talk = max_time_allowed - time_spent;

        // Help them!
        for (int i = 0; i < minutes_to_talk; i++) {
            help_student_for_one_minute(classroom, current_student, time, timeline);
            time_spent++;
            helped_someone = true;
            check_for_new_arrivals(classroom, time, group1, group2, group3);
        }

        // Are they completely done?
        if (classroom[current_student].time_left == 0) {
            classroom[current_student].finished_time = time;
            classroom[current_student].is_in_line = false;
            students_finished++;
            // Remove them from the line
            group2.erase(group2.begin() + best_person_index);
        }
    }
    return helped_someone;
}

// [OS CONCEPT: First Come, First Served (FCFS) Scheduling]
// The simplest scheduling: processes are executed exactly in the order they arrived.
// Group 3: First Come, First Served (The teacher just answers in the order they arrived)
bool teach_group3(
    std::vector<Student> &classroom, std::deque<int> &group3, std::vector<int> &timeline, 
    int &time, int max_time_allowed, int &students_finished, 
    std::deque<int> &group1, std::vector<int> &group2) 
{
    int time_spent = 0;
    bool helped_someone = false;

    while (time_spent < max_time_allowed && !group3.empty()) {
        int current_student = group3.front(); // Just take the first person in line

        // Figure out how long to talk to them
        int minutes_to_talk = classroom[current_student].time_left;
        if (minutes_to_talk > max_time_allowed - time_spent) minutes_to_talk = max_time_allowed - time_spent;

        // Help them!
        for (int i = 0; i < minutes_to_talk; i++) {
            help_student_for_one_minute(classroom, current_student, time, timeline);
            time_spent++;
            helped_someone = true;
            check_for_new_arrivals(classroom, time, group1, group2, group3);
        }

        // Are they completely done?
        if (classroom[current_student].time_left == 0) {
            classroom[current_student].finished_time = time;
            classroom[current_student].is_in_line = false;
            students_finished++;
            group3.pop_front(); // Remove from the front of the line
        }
    }
    return helped_someone;
}

int main() {
    // ---- Define our classroom of students ----
    // id, arrival_time, total_time_needed, time_left, priority_score, first_helped, finished, in_line
    std::vector<Student> classroom = {
        {1,  0,  7,  7,   5,  -1, -1, false},   // High Priority (Group 1)
        {2,  1,  4,  4,  12,  -1, -1, false},   // Med Priority (Group 2)
        {3,  2,  8,  8,  25,  -1, -1, false},   // Low Priority (Group 3)
        {4,  3,  5,  5,   8,  -1, -1, false},   // High Priority (Group 1)
        {5,  5,  3,  3,  18,  -1, -1, false},   // Med Priority (Group 2)
        {6,  6,  6,  6,  27,  -1, -1, false}    // Low Priority (Group 3)
    };

    // The teacher's rules for splitting a 10-minute period
    const int group1_time_allowed  = 5;   // Group 1 gets 5 minutes (50%)
    const int group2_time_allowed  = 3;   // Group 2 gets 3 minutes (30%)
    const int group3_time_allowed  = 2;   // Group 3 gets 2 minutes (20%)
    const int group1_cycle_limit   = 2;   // Group 1 students only get 2 mins before going to back of line

    int clock_time = 0;
    int students_finished = 0;
    std::vector<int> timeline; // A log of who the teacher was helping every minute

    std::deque<int>  group1_line;
    std::vector<int> group2_line;
    std::deque<int>  group3_line;

    // [OS CONCEPT: Multilevel Queue (MLQ) Scheduler]
    // The main loop represents the OS short-term scheduler continuously choosing which queue 
    // to service based on their allotted time budgets.
    // ---- The School Day Loop ----
    // Keep running until every single student is finished
    while (students_finished < (int)classroom.size()) {
        
        // See who is waiting at the door right now
        check_for_new_arrivals(classroom, clock_time, group1_line, group2_line, group3_line);

        bool teacher_was_busy = false;
        
        // The teacher spends their allotted time on each group in order
        teacher_was_busy = teach_group1(classroom, group1_line, timeline, clock_time, group1_time_allowed, group1_cycle_limit, students_finished, group2_line, group3_line) || teacher_was_busy;
        teacher_was_busy = teach_group2(classroom, group2_line, timeline, clock_time, group2_time_allowed, students_finished, group1_line, group3_line) || teacher_was_busy;
        teacher_was_busy = teach_group3(classroom, group3_line, timeline, clock_time, group3_time_allowed, students_finished, group1_line, group2_line) || teacher_was_busy;

        // If all the lines were completely empty, the teacher just waits 1 minute
        if (!teacher_was_busy) {
            timeline.push_back(-1); // -1 means teacher was drinking coffee (idle)
            clock_time++;
        }
    }

    // ---- Print the Timeline (Gantt Chart) ----
    std::cout << "Timeline of who the teacher helped every minute ('-' = drinking coffee):\n";
    for (size_t t = 0; t < timeline.size(); t++) {
        if (timeline[t] == -1) std::cout << "- ";
        else std::cout << "S" << timeline[t] << " ";
    }
    std::cout << "\n\n";

    // ---- Print the final Report Card ----
    std::cout << std::left
              << std::setw(6)  << "ID"
              << std::setw(7)  << "Group"
              << std::setw(8)  << "Arrived"
              << std::setw(8)  << "Needed"
              << std::setw(9)  << "Finished"
              << std::setw(9)  << "Wait"
              << std::setw(12) << "Turnaround"
              << std::setw(10) << "Response"
              << "\n";

    double total_wait = 0, total_turnaround = 0, total_response = 0;

    for (const auto &student : classroom) {
        // Math to calculate how good/bad the waiting was
        int turnaround = student.finished_time - student.arrival_time; // Total time spent in classroom
        int waiting    = turnaround - student.total_time_needed;       // Time spent just standing in line
        int response   = student.first_helped_time - student.arrival_time; // Time before teacher FIRST said hello

        std::string group_name = "G" + std::to_string(get_group_number(student.priority_score));

        std::cout << std::left
                  << std::setw(6)  << student.id
                  << std::setw(7)  << group_name
                  << std::setw(8)  << student.arrival_time
                  << std::setw(8)  << student.total_time_needed
                  << std::setw(9)  << student.finished_time
                  << std::setw(9)  << waiting
                  << std::setw(12) << turnaround
                  << std::setw(10) << response
                  << "\n";

        total_wait += waiting;
        total_turnaround += turnaround;
        total_response += response;
    }

    // Print class averages
    int total_students = (int)classroom.size();
    std::cout << "\nClass Averages:\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Avg Waiting Time    : " << total_wait / total_students << " minutes\n";
    std::cout << "  Avg Turnaround Time : " << total_turnaround / total_students << " minutes\n";
    std::cout << "  Avg Response Time   : " << total_response / total_students << " minutes\n";

    return 0; // School day is over!
}
