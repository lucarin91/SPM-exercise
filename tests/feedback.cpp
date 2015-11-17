#include <iostream>
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
using namespace ff;
const long streamlength=20;

long *F(long *in,ff_node*const) {
    *in *= *in;
    return in;
}
struct Sched: ff_node_t<long> {

    Sched(ff_loadbalancer *const lb):lb(lb) {}

    long* svc(long *task) {
    	int channel=lb->get_channel_id();
    	if (channel == -1) {
    	    std::cout << "Task " << *task << " coming from Stage0\n";
    	    return task;
    	}
    	std::cout << "Task " << *task << " coming from Worker" << channel << "\n";
    	delete task;
    	return GO_ON;
    }
    void eosnotify(ssize_t) {
	// received EOS from Stage0, broadcast EOS to all workers
        lb->broadcast_task(EOS);
    }

    ff_loadbalancer *lb;
};
long *Stage0(long*, ff_node*const node) {
    for(long i=0;i<streamlength;++i)
	node->ff_send_out(new long(i));
    return (long*)EOS;
}
int main() {
    ff_Farm<long>  farm(F, 3);
    farm.remove_collector(); // removes the default collector
    // the scheduler gets in input the internal load-balancer
    Sched S(farm.getlb());
    farm.add_emitter(S);
    // adds feedback channels between each worker and the scheduler
    farm.wrap_around();
    // creates a node from a function
    ff_node_F<long> stage(Stage0);
    // creates the pipeline
    ff_Pipe<> pipe(stage, farm);
    if (pipe.run_and_wait_end()<0) error("running pipe");
    return 0;
}
