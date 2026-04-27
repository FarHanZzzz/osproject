/*
 * ELI5: Parallel Matrix Adder (The Coloring Book)
 * -----------------------------------------------
 * Imagine you have a massive coloring book (Matrices). 
 * If you color it alone, it takes a long time. 
 * But if you invite friends (threads) and give each a section, you finish faster!
 *
 * Usage:  ./matrix_add [number_of_threads]
 *         (defaults to 4 if omitted)
 *
 * cd /home/farhan-sadeque/Downloads/Osproject/TaskC_MatrixAdder
 * g++ -x c++ -std=c++11 -o matrix_add matrix_adder.c -pthread
 * ./matrix_add 4
 */

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
// [OS CONCEPT: Multithreading / OS Threads]
// std::thread is a C++ wrapper around OS-level threads (like POSIX pthreads or Windows Threads).
#include <thread>
#include <vector>

// [OS CONCEPT: Heap Allocation / Shared Memory]
// std::vector allocates memory on the heap. Since all threads are in the same process,
// they can all access this shared heap memory via pointers.
// A Matrix is just a grid of numbers (like a spreadsheet)
using Matrix = std::vector<std::vector<int>>;

// The instructions for each friend (thread) to know what to color
struct WorkerArgs
{
    const Matrix *a;      // pointer to first  input matrix (read-only)
    const Matrix *b;      // pointer to second input matrix (read-only)
    Matrix *result;       // pointer to output matrix (each thread writes different rows)
    int start_row;        // first row this thread handles  (inclusive)
    int end_row;          // last  row this thread handles  (exclusive)
};

// [OS CONCEPT: Thread Execution Context]
// Each thread runs this function independently, with its own Stack and Program Counter (PC).
// The function each thread runs – adds rows [start_row, end_row)
void add_rows(const WorkerArgs args)
{
    for (int r = args.start_row; r < args.end_row; r++)
    {
        for (size_t c = 0; c < (*args.a)[r].size(); c++)
        {
            // [OS CONCEPT: Lock-Free Synchronization / Data Partitioning]
            // We avoid Race Conditions without using Mutexes (locks) because each thread writes
            // to entirely different rows (different memory addresses).
            (*args.result)[r][c] = (*args.a)[r][c] + (*args.b)[r][c];
        }
    }
}

// Runs the addition with `thread_count` threads and returns elapsed ms
double run_addition(const Matrix &a, const Matrix &b, Matrix &result, int thread_count)
{
    int rows = (int)a.size();
    int base_chunk = rows / thread_count;
    int remainder  = rows % thread_count;

    std::vector<std::thread> workers;
    workers.reserve((size_t)thread_count);

    auto start = std::chrono::high_resolution_clock::now();

    int current = 0;
    for (int t = 0; t < thread_count; t++)
    {
        // Spread remainder rows evenly: first `remainder` threads each get 1 extra row
        int chunk = base_chunk + (t < remainder ? 1 : 0);
        int end = current + chunk;

        WorkerArgs args{&a, &b, &result, current, end};
        // [OS CONCEPT: Thread Creation & OS Scheduler]
        // This asks the OS to spawn a new thread and add it to the scheduler's ready queue.
        workers.emplace_back(add_rows, args);
        current = end;
    }

    for (auto &w : workers)
    {
        // [OS CONCEPT: Thread Synchronization & Blocking]
        // main thread blocks (sleeps) here until the worker thread finishes its task.
        w.join();
    }

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = stop - start;
    return elapsed.count();
}

// Simple checksum to verify both results match
long long checksum(const Matrix &m)
{
    long long sum = 0;
    for (const auto &row : m)
    {
        for (int v : row)
        {
            sum += v;
        }
    }
    return sum;
}

int main(int argc, char *argv[])
{
    // ---- Parse thread count from command line ----
    int thread_count = 4;   // default
    if (argc >= 2)
    {
        thread_count = std::atoi(argv[1]);
    }

    if (thread_count <= 0)
    {
        std::cerr << "Error: thread count must be >= 1\n";
        std::cerr << "Usage: " << argv[0] << " [threads]   (default 4)\n";
        return 1;
    }

    // ---- Build two 800x800 matrices ----
    const int ROWS = 800;
    const int COLS = 800;

    Matrix a(ROWS, std::vector<int>(COLS, 1));   // every element = 1
    Matrix b(ROWS, std::vector<int>(COLS, 2));   // every element = 2
    Matrix result_one(ROWS, std::vector<int>(COLS, 0));
    Matrix result_many(ROWS, std::vector<int>(COLS, 0));

    // ---- Run with 1 thread (baseline) ----
    double one_thread_ms = run_addition(a, b, result_one, 1);

    // ---- Run with N threads ----
    if (thread_count > ROWS)
    {
        std::cout << "Note: clamping threads to " << ROWS
                  << " (can't have more threads than rows)\n";
        thread_count = ROWS;
    }
    double many_thread_ms = run_addition(a, b, result_many, thread_count);

    // ---- Verify correctness ----
    long long check1 = checksum(result_one);
    long long check2 = checksum(result_many);

    // ---- Print results ----
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nMatrix size       : " << ROWS << " x " << COLS << "\n";
    std::cout << "1-thread time     : " << one_thread_ms << " ms\n";
    std::cout << thread_count << "-thread time     : " << many_thread_ms << " ms\n";

    if (one_thread_ms > 0 && many_thread_ms > 0)
    {
        double speedup = one_thread_ms / many_thread_ms;
        std::cout << "Speed-up          : " << speedup << "x\n";
    }

    std::cout << "Checksum (1-thr)  : " << check1 << "\n";
    std::cout << "Checksum (" << thread_count << "-thr)  : " << check2 << "\n";

    if (check1 != check2)
    {
        std::cout << "ERROR: results do not match!\n";
        return 1;
    }

    std::cout << "Result check      : OK  (both checksums match)\n";
    return 0;
}
