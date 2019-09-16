#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <thread>
#include <future>
#include <atomic>
#include "utils.h"
#include "graph.h"

std::atomic <long> triangle_count;
std::vector <int> thread_ids;
std::vector <long> triangleCountEachThread;
std::vector <double> time_taken;

uintV countTriangles(uintV *array1, uintE len1, uintV *array2, uintE len2, uintV u, uintV v)
{

  uintE i = 0, j = 0; // indexes for array1 and array2
  uintV count = 0;
  while ((i < len1) && (j < len2))
  {
      if (array1[i] == array2[j])
      {
          if ((array1[i] != u) && (array1[j] != v))
          {
              count++;
          }
          i++;
          j++;
      }
      else if (array1[i] < array2[j])
      {
          i++;
      }
      else
      {
          j++;
      }
  }
  return count;
}
void processEdge(Graph *g, uintV start, uintV end, uint tid){

  std::cout << "thread"<< tid << "start:" << start << "\n";
  std::cout << "thread"<< tid <<  "end:" << end << "\n";
  timer thread_timer;
  thread_timer.start();
  long local_triangle_count = 0;
  for (uintV u = start; u < end; u++)
  {
      // For each outNeighbor v, find the intersection of inNeighbor(u) and outNeighbor(v)
      uintE out_degree = g->vertices_[u].getOutDegree();
      for (uintE i = 0; i < out_degree; i++)
      {
          uintV v = g->vertices_[u].getOutNeighbor(i);
          local_triangle_count += countTriangles(g->vertices_[u].getInNeighbors(),
                                           g->vertices_[u].getInDegree(),
                                           g->vertices_[v].getOutNeighbors(),
                                           g->vertices_[v].getOutDegree(),
                                           u,
                                           v);
      }
  }
  thread_ids[tid] = tid;
  triangleCountEachThread[tid] = local_triangle_count;
  triangle_count.fetch_add(local_triangle_count);
  time_taken[tid] = thread_timer.stop();

}
void triangleCountParallel(Graph &g, uint n_workers)
{
  uintV n = g.n_;
  double total_time_taken = 0.0;
  timer t1;
  uintV start;
  uintV end;
  //Graph tempGraph = g;

  // The outNghs and inNghs for a given vertex are already sorted

  // Create threads and distribute the work across T threads
  // -------------------------------------------------------------------
  t1.start();
  std::thread t[n_workers];
  // Process each edge <u,v>
  for(uint i = 0; i < n_workers - 1; i++){
    start = (n/n_workers)*i;
    end = ((i+1)*(n/n_workers));
    t[i] = std::thread(processEdge, &g, start, end, i); // i is tid
  }
  start = n/n_workers*(n_workers-1);
  end = n;
  t[n_workers -1] = std::thread(processEdge, &g, start, end, n_workers -1);
  // -------------------------------------------------------------------
  // Here, you can just print the number of non-unique triangles counted by each thread
  // std::cout << "thread_id, triangle_count, time_taken\n";
  // Print the above statistics for each thread
  // Example output for 2 threads:
  // thread_id, triangle_count, time_taken
  // 1, 102, 0.12
  // 0, 100, 0.12
  std::cout << "thread_id, triangle_count, time_taken\n";
  for(uint i = 0; i < n_workers; i++){
    t[i].join();
    std::cout <<thread_ids[i]<< ", " << triangleCountEachThread[i] << ", "
    << std::setprecision(TIME_PRECISION) << time_taken[i]<<"\n";
  }
  total_time_taken = t1.stop();

  // Print the overall statistics
  std::cout << "Number of triangles : " << triangle_count << "\n";
  std::cout << "Number of unique triangles : " << triangle_count / 3 << "\n";
  std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION) << total_time_taken << "\n";
}

int main(int argc, char *argv[])
{
  cxxopts::Options options("triangle_counting_serial", "Count the number of triangles using serial and parallel execution");
  options.add_options("custom", {
                                    {"nWorkers", "Number of workers", cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
                                    {"inputFile", "Input graph file path", cxxopts::value<std::string>()->default_value("/scratch/assignment1/input_graphs/roadNet-CA")},
                                });

  auto cl_options = options.parse(argc, argv);
  uint n_workers = cl_options["nWorkers"].as<uint>();
  std::string input_file_path = cl_options["inputFile"].as<std::string>();
  std::cout << std::fixed;
  std::cout << "Number of workers : " << n_workers << "\n";

  Graph g;
  std::cout << "Reading graph\n";
  g.read_graph_from_binary<int>(input_file_path);
  std::cout << "Created graph\n";

  thread_ids.reserve(n_workers);
  triangleCountEachThread.reserve(n_workers);
  time_taken.reserve(n_workers);

  triangleCountParallel(g, n_workers);

  return 1;
}
