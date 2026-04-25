# Task D: MLQ Scheduler (Concepts First!)

Before looking at the code, let's understand what **Scheduling** means in an Operating System.

---

## 🧠 Core Concept: What is a Scheduler?
Imagine you are a Chef in a busy kitchen. You have three stoves and 20 orders coming in. You can't cook all 20 orders at once. You have to decide:
- *Which order do I cook first?*
- *How long do I spend on this order before checking the other pots?*

The **Scheduler** is the Chef of the CPU. It decides which programs (Processes) get to run and for how long.

## 🧠 Core Concept: MLQ (Multi-Level Queue)
A Multi-Level Queue means the Chef has categorized the orders into "VIP", "Regular", and "Low Priority". In our code, we have 3 Queues:

1. **Queue 1 (VIP - Round Robin):**
   - **Concept:** Imagine giving 5 kids exactly 1 minute to play a video game before passing the controller to the next kid in a circle.
   - **In OS:** Everyone gets a fair, equal slice of time.

2. **Queue 2 (Regular - Shortest Job First):**
   - **Concept:** You look at the line at a grocery store. You let the person holding only 1 item go ahead of the person with a full shopping cart.
   - **In OS:** The process that requires the least amount of time goes first.

3. **Queue 3 (Low Priority - First Come First Served):**
   - **Concept:** Standard fast-food line. Whoever gets in line first, gets served first.
   - **In OS:** No cutting the line. Processes execute in the exact order they arrived.

---

## 💻 How the Code Works

This program uses a massive infinite loop (the "Chef") to look at all 3 Queues.

### Step-by-Step Walkthrough

1. **Setting the Budgets**
   ```cpp
   int q1_budget = 5;
   int q2_budget = 3;
   int q3_budget = 2;
   ```
   To make sure the lower queues actually get some CPU time, we give Queue 1 exactly 5 "ticks" (seconds) of CPU time, then we switch to Queue 2 for 3 ticks, and Queue 3 for 2 ticks. 

2. **Queue 1 Logic (Round Robin)**
   ```cpp
   if (procs[i].queue_level == 1 && procs[i].time_left > 0 && q1_budget > 0) {
       procs[i].time_left--;
       q1_budget--;
   }
   ```
   We just loop straight through all the Queue 1 processes. Because it's inside a `for` loop, it naturally acts like a circle (Round Robin). It runs Process A for 1 tick, then Process B for 1 tick.

3. **Queue 2 Logic (Shortest Job First)**
   ```cpp
   if (procs[i].time_left < shortest_time) {
       shortest_time = procs[i].time_left;
       shortest_index = i;
   }
   ```
   For Queue 2, we must look at all the processes and find the one with the smallest `time_left`. We then run that specific process. 

4. **Queue 3 Logic (First Come First Served)**
   This acts just like Queue 1, except because of how the data is stored in the list, it naturally just starts at the top and runs until the process finishes or the budget runs out.
