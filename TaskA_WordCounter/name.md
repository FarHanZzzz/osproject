Task A (Word Counter): It is Static in execution (runs start-to-finish), but Dynamic in resource allocation (scales forks and pipes based on input).
Task B (Elusive Cursor): It is purely Dynamic because it runs continuously and reacts to background OS timer events.
Task C (Matrix Adder): It is Static in execution (does the math and ends), but Dynamic in load balancing (adapts to however many threads the bash script gives it).
Task D (MLQ Scheduler): It is Dynamic in execution (simulating active CPU context-switching), even though the students' priority groups are static.
Task E (Real-Time Scheduler): It is a mix: The Rate Monotonic algorithm is strictly Static (fixed priorities), while the EDF algorithm is strictly Dynamic (priorities shift every tick).
Optional Task (Kernel Module): It is purely Static because it executes a fixed calculation exactly once when loaded and once when unloaded.