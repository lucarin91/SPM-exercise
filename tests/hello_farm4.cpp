#include <iostream>
#include <vector>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
using namespace ff;
struct Worker: ff_node_t<long> {
  int svc_init() {
    std::cout << "Hello I'm the worker " << get_my_id() << "\n";
    return 0;
  }
  long *svc(long *t) {  return t; } // it does nothing, just sends out tasks
};
struct firstStage: ff_node_t<long> {
  long size=10;
  long *svc(long *) { 
    for(long i=0; i < size; ++i)
      ff_send_out(new long(i)); // sends the task into the output channel
    return EOS;
  }
} Emitter;
struct lastStage: ff_minode_t<long> { // NOTE: multi-input node
  long *svc(long *t) { 
      const long &task=*t;
      std::cout << "Last stage, received " << task 
                << " from " << get_channel_id() << "\n";
      delete t;
      return GO_ON;
  }
} LastStage;
int main(int argc, char *argv[]) {
  assert(argc>1);
  int nworkers = atoi(argv[1]);
  std::vector<std::unique_ptr<ff_node> > Workers;
  for(int i=0;i<nworkers;++i) 
      Workers.push_back(make_unique<Worker>());
  ff_Farm<long> farm(std::move(Workers), Emitter);
  farm.remove_collector(); // this removes the default collector
  ff_Pipe<> pipe(farm, LastStage);
  if (pipe.run_and_wait_end()<0) error("running pipe"); 
  return 0;
}

