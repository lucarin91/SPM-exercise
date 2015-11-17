#include <iostream>
#include <ff/parallel_for.hpp>
using namespace ff;
const size_t SIZE= 1<<20;
int main(int argc, char * argv[]) {    
  assert(argc > 1);
  int  nworkers  = atoi(argv[1]);
  // creates the array
  double *A = new double[SIZE];

  ParallelForReduce<double> pfr(nworkers);
  // fill out the array A using the parallel-for
  pfr.parallel_for(0,SIZE,1, 0, [&](const long j) { A[j]=j*1.0;});
  
  auto reduceF = [](double& sum, const double elem) { sum += elem; };
  auto bodyF   = [&A](const long j, double& sum) { sum += A[j]; };
  {
    double sum = 0.0;
    std::cout << "\nComputing sum with " << std::max(1,nworkers/2)
      << " workers, default static scheduling\n";	
    pfr.parallel_reduce(sum, 0.0, 0L, SIZE,
                        bodyF, reduceF, std::max(1,nworkers/2));
    std::cout << "Sum = " << sum << "\n\n";
  }
  {
    double sum = 0.0;
    std::cout << "Computing sum with " << nworkers
      << " workers, static scheduling with interleaving 1000\n";
    pfr.parallel_reduce_static(sum, 0.0, 0, SIZE, 1, 1000,
                               bodyF, reduceF, nworkers);
    std::cout << "Sum = " << sum << "\n\n";
  }
  {
    double sum = 0.0;
    std::cout << "Computing sum with " << nworkers-1
      << " workers,  dynamic scheduling chunk=1000\n";
    pfr.parallel_reduce(sum, 0.0, 0, SIZE, 1, 1000,
                        bodyF, reduceF, nworkers);
    std::cout << "Sum = " << sum << "\n\n";
  }
  delete [] A;
  return 0;
}

