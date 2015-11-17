#include <iostream>
#include <vector>
#include <ff/farm.hpp>
using namespace ff;
struct Worker: ff_node_t<long> {
  long *svc(long *task) {
      *task = pow(*task,3);
      return task;
  }
};
int main(int argc, char *argv[]) {
  assert(argc>2);
  int nworkers = atoi(argv[1]);
  int streamlen= atoi(argv[2]);

  std::vector<std::unique_ptr<ff_node> > Workers;
  for(int i=0;i<nworkers;++i) 
      Workers.push_back(make_unique<Worker>());
  
  ff_Farm<long> farm(std::move(Workers), 
		     true /* accelerator mode turned on*/);
  // Now run the accelator asynchronusly
  if (farm.run_then_freeze()<0) // farm.run() can also be used here
      error("running farm");
  long *result = nullptr;
  for (long i=0;i<streamlen;i++) {
      long *task = new long(i);
      // Here offloading computation onto the farm
      farm.offload(task); 
      
      // do something smart here...
      for(volatile long k=0; k<10000; ++k);
            
      // try to get results, if there are any
      if (farm.load_result_nb(result)) {
          std::cerr << "[inside for loop] result= " << *result << "\n";
          delete result;
      }
  }
  farm.offload(EOS); // sending End-Of-Stream
#if 1
  // get all remaining results syncronously. 
  while(farm.load_result(result)) {
      std::cerr << "[outside for loop] result= " << *result << "\n";
      delete result;
  }
#else
    // asynchronously waiting for results
    do {
        if (farm.load_result_nb(result)) {
            if (result==EOS) break;
            std::cerr << "[outside for loop] result= " << *result << "\n";
            delete result;
        } 
    } while(1);
#endif
    farm.wait();  // wait for termination
    return 0;
}
