#include <iostream>
#include <vector>
#include <ff/farm.hpp>
using namespace ff;
struct WorkerA: ff_node_t<long> {
  long *svc(long *task) { 
      std::cout << "WorkerA has got the task " << *task << "\n";
      return task;
  }
};
struct WorkerB: ff_node_t<long> {
  long *svc(long *task) { 
      std::cout << "WorkerB has got the task " << *task << "\n";
      return task;
  }
};
struct Emitter: ff_node_t<long> {
  Emitter(ff_loadbalancer *const lb):lb(lb) {}
  ff_loadbalancer *const lb;
  const long size=10;  
  long *svc(long *) { 
    for(long i=0; i <= size; ++i) {
	lb->broadcast_task(new long(i));
    }
    return EOS;
  }
};
struct Collector: ff_node_t<long> {
  Collector(ff_gatherer *const gt):gt(gt) {}
  ff_gatherer *const gt;
  long *svc(long *task) { 
      std::cout << "received task from Worker " << gt->get_channel_id() << "\n";
      if (gt->get_channel_id() == 0) delete task;
      return GO_ON;
  }
};
int main(int argc, char *argv[]) {
  assert(argc>1);
  int nworkers = atoi(argv[1]);
  assert(nworkers>=2);
  std::vector<std::unique_ptr<ff_node> > Workers;
  for(int i=0;i<nworkers/2;++i)        Workers.push_back(make_unique<WorkerA>());
  for(int i=nworkers/2;i<nworkers;++i) Workers.push_back(make_unique<WorkerB>());
  ff_Farm<> farm(std::move(Workers)); 
  Emitter E(farm.getlb());
  Collector C(farm.getgt());
  farm.add_emitter(E);    // add the specialized emitter
  farm.add_collector(C);  
  if (farm.run_and_wait_end()<0) error("running farm"); 
  return 0;
}
