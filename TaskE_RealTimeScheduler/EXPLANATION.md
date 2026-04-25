# Task E: Real-Time Scheduler (Concepts First!)

Before looking at the code, let's understand how a **Real-Time System** works.

---

## 🧠 Core Concept: What is a Real-Time System?
Imagine you are writing the code for a car's airbag. If the car crashes, the airbag MUST deploy within 50 milliseconds. If the CPU says, *"Hold on, I'm busy updating the radio screen, I'll deploy the airbag in 100 milliseconds,"* the passenger gets hurt. 

In a Real-Time OS, **Deadlines** are absolute. A program is considered a failure if it finishes its task even 1 millisecond too late.

## 🧠 Core Concept: The Hyperperiod
We have three tasks looping forever:
- Task 1 repeats every 5 seconds.
- Task 2 repeats every 10 seconds.
- Task 3 repeats every 20 seconds.

If we want to prove our scheduling algorithm works, we don't have to run it forever. We just have to run it until the pattern repeats perfectly! The point where all tasks restart at the exact same time is called the **Hyperperiod**.
*(Math trick: The Hyperperiod is just the Lowest Common Multiple (LCM) of the periods. The LCM of 5, 10, and 20 is exactly **20**).*

## 🧠 Core Concept: RM vs. EDF
We have two ways (algorithms) to decide who gets the CPU:

1. **Rate Monotonic (RM) - "Shortest Cycle Wins"**
   - **Concept:** The task that repeats the most often gets the highest priority forever. 
   - **In our code:** Task 1 repeats every 5 seconds. Task 3 repeats every 20 seconds. So Task 1 is VIP, and Task 3 is Low Priority. This priority *never changes*.

2. **Earliest Deadline First (EDF) - "Urgent Deadline Wins"**
   - **Concept:** The task whose deadline is approaching the fastest gets the CPU.
   - **In our code:** Priority changes dynamically! If Task 3's deadline is in 2 seconds, but Task 1's deadline is in 4 seconds, Task 3 suddenly becomes the VIP.

---

## 💻 How the Code Works

This program is extremely simple because we **hardcoded** the Hyperperiod to 20, meaning we just loop exactly 20 times.

### Step-by-Step Walkthrough

1. **The Tasks**
   ```cpp
   vector<Task> tasks = {
       {1, 2, 5,  0}, 
       {2, 4, 10, 0}, 
       {3, 1, 20, 0}
   };
   ```
   We have three tasks. Task 1 needs `2` seconds of CPU time and repeats every `5` seconds.

2. **The Big Loop (Hyperperiod)**
   ```cpp
   for (int tick = 0; tick < 20; tick++)
   ```
   We run a clock from 0 to 19.

3. **Waking Up Tasks**
   ```cpp
   if (tick % 5 == 0)  tasks[0].time_left = tasks[0].exec_time;
   ```
   If the clock hits 0, 5, 10, or 15, Task 1 "wakes up" and demands its 2 seconds of CPU time.

4. **Rate Monotonic Logic**
   ```cpp
   if (tasks[0].time_left > 0) { ... }
   else if (tasks[1].time_left > 0) { ... }
   ```
   In the RM section, we just use a simple `if-else` chain! Since Task 1 has the shortest period, we put it at the very top of the `if` chain. It will always be checked first!

5. **Earliest Deadline First Logic**
   ```cpp
   int deadline1 = ((tick / 5) + 1) * 5;
   ```
   In the EDF section, we do some quick math to figure out when the next period starts. Whichever task has the smallest deadline gets to run next!
