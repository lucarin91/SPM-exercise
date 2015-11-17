#include <iostream>
#include <vector>
#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
using namespace ff;
typedef std::pair<long,long> fftask_t;

struct Start: ff_node_t<fftask_t> {
    Start(long streamlen):streamlen(streamlen) {}
    fftask_t *svc(fftask_t*) {    
        for (long j=0;j<streamlen;j++) {
            ff_send_out(new std::pair<long,long>(random() % 20000, j));
        }
        return EOS;
    }
    long streamlen;
};
struct Worker: ff_node_t<fftask_t> {
    fftask_t *svc(fftask_t *task) {
        for(volatile long j=task->first; j>0;--j);
        return task;
    }
};
struct Stop: ff_node_t<fftask_t> {
    int svc_init() { expected = 0; return 0;}
    fftask_t *svc(fftask_t *task) {    
        if (task->second != expected) 
            std::cerr << "ERROR: tasks received out of order, received " 
                      << task->second << " expected " << expected << "\n";        
        expected++;
        delete task;
        return GO_ON;
    }
    long expected;
};

int main() {
    long nworkers  = 2, streamlen = 1000;
    srandom(1);

    Start start(streamlen);       
    Stop stop;
    std::vector<std::unique_ptr<ff_node> > W;
    for(int i=0;i<nworkers;++i) 
       W.push_back(make_unique<Worker>());
#if defined(NOT_ORDERED)
    ff_Farm<>  ofarm(std::move(W));
#else
    ff_OFarm<> ofarm(std::move(W));
#endif
    ff_Pipe<> pipe(start, ofarm, stop);
    if (pipe.run_and_wait_end()<0)
	error("running pipe\n");
    return 0;
}
