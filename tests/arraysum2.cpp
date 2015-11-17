#include <iostream>
#define HAS_CXX11_VARIADIC_TEMPLATES 1
#include <ff/parallel_for.hpp>
using namespace ff;
const size_t SIZE= 1<<20;
int main(int argc, char * argv[]) {    
    assert(argc > 1);
    int  nworkers  = atoi(argv[1]);
    // creates the array
    double *A = new double[SIZE];

    ParallelForPipeReduce<double*> pfpipe(nworkers,true);
    // init 
    pfpipe.parallel_for_idx(0,SIZE,1,0,[&A](const long start, const long stop,
					    const int thid,ff_buffernode &) { 
				for(long j=start;j<stop;++j) A[j] = j*1.0;
			    });
    double sum = 0.0;
    auto bodyFpipe = [&A](const long start, const long stop, const int thid, ff_buffernode &node) {
	// needed to avoid sending spurious lsum values to the reduce stage
	if (start==stop) return; 
    	double *lsum=new double(0.0);
    	for(long i=start;i<stop;++i) *lsum += A[i];
    	node.put(lsum);
    };
    auto reduceFpipe= [&sum](double *lsum) { 
    	sum += *lsum; delete lsum;
    };
    
    std::cout << "Computing sum with " << nworkers
    	      << " workers, using the ParallelForPipeReduce and default scheduling\n";
    pfpipe.parallel_reduce_idx(0, SIZE, 1, 0,
    			       bodyFpipe, reduceFpipe, nworkers);
    std::cout << "Sum = " << sum << "\n\n";
    delete [] A;
    return 0;
}
