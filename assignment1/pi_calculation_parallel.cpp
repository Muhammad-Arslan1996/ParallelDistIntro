#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include "utils.h"

#define sqr(x) ((x) * (x))
#define DEFAULT_NUMBER_OF_POINTS "12345678"
std::atomic <int> circle_count;
std::vector <int> thread_ids;
std::vector <int> points_generated;
std::vector <int> circle_points;
std::vector <double> time_taken;
uint rand_seed;


uint c_const = (uint)RAND_MAX + (uint)1;
inline double get_random_coordinate(uint *random_seed)
{
    return ((double)rand_r(random_seed)) / c_const;
}
void get_points_in_circle(uint n, uint random_seed, int tid )//std::promise<int> & prom)
{
    timer thread_timer;
    thread_timer.start();
    uint circlePointsForEachThread = 0;
    double x_coord, y_coord;
    for (uint i = 0; i < n; i++)
    {
        x_coord = (2.0 * get_random_coordinate(&random_seed)) - 1.0;
        y_coord = (2.0 * get_random_coordinate(&random_seed)) - 1.0;
        if ((sqr(x_coord) + sqr(y_coord)) <= 1.0)
            circlePointsForEachThread++;
    }
    thread_ids[tid] = tid;
    points_generated[tid] = n;
    circle_points[tid] = circlePointsForEachThread;
    circle_count.fetch_add(circlePointsForEachThread);
}
void piCalculation(uint n, uint totalThreads)
{
    rand_seed = 1;
    uint pointsForEachThread = n/totalThreads;
    timer serial_timer;
    double total_time_taken = 0;
    serial_timer.start();
    std::thread t[totalThreads];
    for(uint i = 0; i < totalThreads - 1; i++){
      t[i] = std::thread(get_points_in_circle, pointsForEachThread, rand() % 100, i); // i is tid
    }
    uint remainingPoints = n%totalThreads;
        //for last thread if n is not exactly divisible by totalThreads
    t[totalThreads -1] = std::thread(get_points_in_circle, pointsForEachThread+remainingPoints, rand() % 1, totalThreads -1);

    std::cout << "thread_id, points_generated, circle_points, time_taken\n";
    for(uint i = 0; i < totalThreads; i++){
      t[i].join();
      std::cout << thread_ids[i]<< ", " << points_generated[i] << ", "
      << circle_points[i]<< ", "
      << std::setprecision(TIME_PRECISION) << time_taken[i]<<"\n";
    }
    double pi_value = 4.0 * (double)circle_count / (double)n;
    total_time_taken = serial_timer.stop();

    // Print the above statistics for each thread
    // Example output for 2 threads:
    // thread_id, points_generated, circle_points, time_taken
    // 1, 100, 90, 0.12
    // 0, 100, 89, 0.12

    // Print the overall statistics
    std::cout << "Total points generated : " << n << "\n";
    std::cout << "Total points in circle : " << circle_count << "\n";
    std::cout << "Result : " << std::setprecision(VAL_PRECISION) << pi_value << "\n";
    std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION) << total_time_taken << "\n";
}

int main(int argc, char *argv[])
{
    // Initialize command line arguments
    cxxopts::Options options("pi_calculation", "Calculate pi using serial and parallel execution");
    options.add_options("custom", {
                                {"nPoints", "Number of points", cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_POINTS)},
                                {"nWorkers", "Number of workers", cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
                            });

    auto cl_options = options.parse(argc, argv);
    uint n_points = cl_options["nPoints"].as<uint>();
    uint n_workers = cl_options["nWorkers"].as<uint>();
    std::cout << std::fixed;
    std::cout << "Number of points : " << n_points << "\n";
    std::cout << "Number of workers : " << n_workers << "\n";
    thread_ids.reserve(n_workers);
    points_generated.reserve(n_workers);
    circle_points.reserve(n_workers);
    time_taken.reserve(n_workers);
    piCalculation(n_points, n_workers);

    return 0;
}
