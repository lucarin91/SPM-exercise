#include <iostream>
#define HAS_CXX11_VARIADIC_TEMPLATES 1
#include <ff/parallel_for.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include <ff/map.hpp>
using namespace ff;
const long SIZE = 100;

typedef std::pair<std::vector<long>,std::vector<long> > task_t;

struct mapWorker: ff_Map<task_t> {
    task_t *svc(task_t*) {
        task_t *task = new task_t;
	task->first.resize(SIZE);
	task->second.resize(SIZE);
	const int myid = get_my_id();
        ff_Map<task_t>::parallel_for(0,SIZE,[myid,&task](const long i) { 
                task->first.operator[](i)  = i + myid;
		task->second.operator[](i) = SIZE-i;
            },3);
        ff_send_out(task);
        return EOS;
    }
};
struct mapStage: ff_Map<task_t> {
    mapStage():ff_Map<task_t>(ff_realNumCores()) {}
    task_t *svc(task_t *task) {
        
        // this is the parallel_for provided by the ff_Map class
        ff_Map<task_t>::parallel_for(0,SIZE,[&task](const long i) { 
                task->first.operator[](i) += task->second.operator[](i);
            });
	
	for(size_t i=0;i<SIZE;++i)
	    std::cout << task->first.operator[](i) << " ";
	std::cout << "\n";
	delete task;
	return GO_ON;
    }    
};
int main() {
    std::vector<std::unique_ptr<ff_node> > W;
    W.push_back(make_unique<mapWorker>());
    W.push_back(make_unique<mapWorker>());

    ff_Farm<task_t> farm(std::move(W));
    mapStage stage;
    ff_Pipe<task_t> pipe(farm, stage);
    if (pipe.run_and_wait_end()<0)
        error("running pipe");   
    return 0;	
}
