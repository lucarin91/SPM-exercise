#include <iostream>
#include <vector>
#include <ff/farm.hpp>
using namespace ff;
struct Worker: ff_node_t<long> {
    long *svc(long *task) { 
	std::cout << "Worker " << get_my_id() 
		  << " has got the task " << *task << "\n";
	delete task;
	return GO_ON;
    }
};
struct Emitter: ff_node_t<long> {
    Emitter(ff_loadbalancer *const lb):lb(lb) {}
    long *svc(long *) { 
	for(long i=0; i <= size; ++i) {
	    if (i==0 || i == (size-1))
		lb->ff_send_out_to(new long(i), 0);
	    else
		ff_send_out(new long(i));          
	}
	return EOS;
    }

    ff_loadbalancer * lb;
    const long size=10;  
};
int main(int argc, char *argv[]) {
  assert(argc>1);
  int nworkers = atoi(argv[1]);
  std::vector<std::unique_ptr<ff_node> > Workers;
  for(int i=0;i<nworkers;++i) 
      Workers.push_back(make_unique<Worker>());
  ff_Farm<long> farm(std::move(Workers));
  Emitter E(farm.getlb());
  farm.add_emitter(E);     // adds the specialized emitter
  farm.remove_collector(); // this removes the default collector
  if (farm.run_and_wait_end()<0) error("running farm"); 
  return 0;
}
