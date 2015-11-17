#include <iostream>
#include <ff/pipeline.hpp> // defines ff_pipeline and ff_Pipe
using namespace ff;
typedef long myTask;  // this is my input/output type
#if 1
myTask* F1(myTask *t,ff_node*const node) {     // 1st stage
  static long count = 10;
  std::cout << "Hello I'm stage F1, sending 1\n";
  return (--count > 0) ? new long(1) : (myTask*)EOS;
} 
#else  
// This version does not terminate. Can you say why ?
myTask* F1(myTask *t,ff_node*const node) {     // 1st stage
  std::cout << "Hello I'm stage F1, sending 1\n";
  return new long(1);
} 
#endif
struct F2: ff_node_t<myTask> {                 // 2nd stage
    myTask *svc(myTask *task) { 
	std::cout << "Hello I'm stage F2, I've received " << *task << "\n";
	return task;
    }
} F2;
myTask* F3(myTask *task,ff_node*const node) {    // 3rd stage
    std::cout << "Hello I'm stage F3, I've received " << *task << "\n";
    return task;
} 
int main() {
    // F1 and F3 are 2 functions, F2 is an ff_node
    ff_node_F<myTask> first(F1);
    ff_node_F<myTask> last(F3);

    ff_Pipe<> pipe(first, F2, last);
    if (pipe.run_and_wait_end()<0) error("running pipe"); 
    return 0;
}
