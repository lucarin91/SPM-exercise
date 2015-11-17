#include <iostream>
#include <ff/pipeline.hpp> // defines ff_pipeline and ff_Pipe
using namespace ff;
typedef long myTask;  // this is my input/output type
struct firstStage: ff_node_t<myTask> {  // 1st stage
    myTask *svc(myTask*) {
	for(long i=0;i<10;++i) {
	    // sends the task to the next stage
	    ff_send_out(new myTask(i));
	}
	return EOS; // End-Of-Stream
    }
};
struct secondStage: ff_node_t<myTask> { // 2nd stage
    myTask *svc(myTask *t) { 
	std::cout << "Hello I'm stage 2, I've received a task\n";
	return t;
    }
};
struct thirdStage: ff_node_t<myTask> {  // 3rd stage
    myTask *svc(myTask *task) {
	std::cout << "Hello I'm stage 3, I've received " << *task << "\n";
	delete task;
	return task; //GO_ON; 
    }
}; 
int main() {
#if 1 // simplest option
  firstStage  _1;
  secondStage _2;
  thirdStage  _3;
#else
  std::unique_ptr<firstStage>  _1(new firstStage);  
  std::unique_ptr<secondStage> _2(new secondStage);
  std::unique_ptr<thirdStage>  _3(new thirdStage);
#endif
  // builds the pipeline
  ff_Pipe<> pipe(_1,_2,_3);

  /* yet another option
    ff_Pipe<> pipe(make_unique<firstStage>(), 
		 make_unique<secondStage>(),
		 make_unique<thirdStage>());
  */

  // starts the pipeline
  if (pipe.run_and_wait_end()<0) error("running pipe"); 
  return 0;
}
