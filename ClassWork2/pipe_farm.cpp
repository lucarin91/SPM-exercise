#include <iostream>
#include <vector>
#include "ff/pipeline.hpp"
#include "ff/farm.hpp"

using namespace std;
using namespace ff;
typedef long myTask;

struct myNode1: ff_node_t<myTask> {
 int n;
 myNode1(int ntask) : n(ntask) {}
 myTask *svc(myTask *) {
 for(long i=n-1;i>=0;--i)
   ff_send_out(new myTask(i));
 return EOS;
}};
struct myNode2: ff_node_t<myTask> {
 myTask *svc(myTask *task) {
   *task = (*task)*(*task);
   std::cout << "res " << *task << std::endl;
   return task; 
}};
struct myNode3: ff_minode_t<myTask> {
  int sum = 0;
  myTask *svc(myTask* task) {
    sum+=*task;
    delete task;
    return GO_ON;
  }
  void svc_end(){
    std::cout <<"tot "<<sum<<std::endl;
  }
}; 

int main(int argc, char* argv[]){
  myNode1 _1(6);
  myNode3 _3;
  myNode2 _2;
  
  std::vector<std::unique_ptr<ff_node>> W;
  W.push_back(make_unique<myNode2>());
  W.push_back(make_unique<myNode2>());
  W.push_back(make_unique<myNode2>());
  W.push_back(make_unique<myNode2>());

  ff_Farm<myTask> myFarm(W,_1,_3);
  //myFarm.remove_collector();

  //ff_Pipe<> pipe(_1,myFarm,_3);
  
  if (myFarm.run_and_wait_end()<0) error("running pipe");
  return 0; 
}
