#include <iostream>
#include <vector>
#include <ff/farm.hpp>
using namespace ff;
struct Worker: ff_node_t<long> {
  long *svc(long *t) {
    std::cout << "Hello I'm the worker " << get_my_id() << "\n";
    return t;
  }
};
int main(int argc, char *argv[]) {
  assert(argc>1);
  int nworkers = atoi(argv[1]);

  ff_Farm<> myFarm( [nworkers]() {
	  std::vector<std::unique_ptr<ff_node> > Workers;
	  for(int i=0;i<nworkers;++i) 
	    Workers.push_back(make_unique<Worker>());
	    //Workers.push_back(std::unique_ptr<ff_node_t<long> >(new Worker));
	  return Workers;
    }() );


  if (myFarm.run_and_wait_end()<0) error("running myFarm");
  return 0;
}
