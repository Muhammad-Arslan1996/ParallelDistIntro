#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include "utils.h"

#define sqr(x) ((x) * (x))
#define DEFAULT_NUMBER_OF_POINTS "12345678"
#define DEFAULT_NUMBER_OF_WORKERS "4"

struct arg_struct{
  uint numberOfPoints;
  uint random_seed;
  uint circleCountForEachThread;
  double timeTakenForEachThread;
};
uint c_const = (uint)RAND_MAX + (uint)1;
inline double get_random_coordinate(uint *random_seed)
{
    return ((double)rand_r(random_seed)) / c_const;
}

void *get_points_in_circle(void *args)
{
    timer serial_timer;
    double time_taken = 0.0;
    serial_timer.start();
    struct arg_struct *argument_struct = (struct arg_struct *) args;
    uint n = argument_struct->numberOfPoints;
    uint circle_count = 0;
    double x_coord, y_coord;
    for (uint i = 0; i < n; i++)
    {
        x_coord = (2.0 * get_random_coordinate(&argument_struct->random_seed)) - 1.0;
        y_coord = (2.0 * get_random_coordinate(&argument_struct->random_seed)) - 1.0;
        if ((sqr(x_coord) + sqr(y_coord)) <= 1.0)
            circle_count++;
    }
    argument_struct->circleCountForEachThread = circle_count;
    time_taken = serial_timer.stop();
    argument_struct->timeTakenForEachThread = time_taken;
}

void piCalculation(uint n, uint totalThreads)
{
    uint pointsForEachThread = n/totalThreads;
    struct arg_struct args [totalThreads];
    uint circle_points = 0;
    timer serial_timer;
    double total_time_taken = 0;
    serial_timer.start();
    pthread_t thread_ids [totalThreads];
    for(uint i = 0; i < totalThreads - 1; i++){
      args[i].numberOfPoints = pointsForEachThread;
      args[i].random_seed = 1;
      pthread_create(&thread_ids[i], NULL, get_points_in_circle, &args[i]);
    }
    uint remainingPoints = n%totalThreads;
    //for last thread if n is not exactly divisible by totalThreads
    args[totalThreads - 1].numberOfPoints = pointsForEachThread + remainingPoints;
    args[totalThreads - 1].random_seed = 1;
    pthread_create(&thread_ids[totalThreads - 1], NULL, get_points_in_circle, &args[totalThreads - 1]);

    std::cout << "thread_id, points_generated, circle_points, time_taken\n";
    for(uint i = 0; i < totalThreads; i++){
      pthread_join(thread_ids[i], NULL);
      circle_points += args[i].circleCountForEachThread;
      std::cout << thread_ids[i] << ", " << args[i].numberOfPoints << ", "
      << args[i].circleCountForEachThread << ", "
      << std::setprecision(TIME_PRECISION) << args[i].timeTakenForEachThread <<"!\n";
    }
    double pi_value = 4.0 * (double)circle_points / (double)n;
    total_time_taken = serial_timer.stop();

    // Print the above statistics for each thread
    // Example output for 2 threads:
    // thread_id, points_generated, circle_points, time_taken
    // 1, 100, 90, 0.12
    // 0, 100, 89, 0.12

    // Print the overall statistics
    std::cout << "Total points generated : " << n << "\n";
    std::cout << "Total points in circle : " << circle_points << "\n";
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

    piCalculation(n_points, n_workers);

    return 0;
}
