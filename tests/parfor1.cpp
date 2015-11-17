#include <iostream>
#include <ff/parallel_for.hpp>
using namespace ff;

int main(int argc, char *argv[]) {
  assert(argc>1);
  long N = atol(argv[1]);
  long *A = new long[N];

  ParallelFor pf( ff_realNumCores() );

  // initialize the array A
  pf.parallel_for(0,N,[&A](const long i) { A[i]=i;});

  // do something on each even element of the array A
  pf.parallel_for(0,N,2,[&A](const long i) { A[i]=i*i;});
  // print the result

  for(long i=0;i<N;++i)
    std::cout << A[i] << " ";
  std::cout << "\n";
  delete [] A;
  return 0;
}
