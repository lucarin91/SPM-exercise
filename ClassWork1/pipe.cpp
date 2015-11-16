#include <iostream>
#include "ff/pipeline.hpp"
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
   *task = (*task) * (*task);
   std::cout << "res " << *task << std::endl;
   return task; 

}};
struct myNode3: ff_node_t<myTask> {
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
  myNode2 _2;
  myNode3 _3;
  ff_Pipe<> pipe(_1,_2,_3);
  
  if (pipe.run_and_wait_end()<0) error("running pipe");
  return 0; 
}
