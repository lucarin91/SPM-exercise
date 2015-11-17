/*  
 *         
 *    Stage0 -----> Stage1 -----> Stage2 -----> Stage3
 *    ^  ^ ^             |          |              | 
 *     \  \ \------------           |              |
 *      \  \-------------------------              |
 *       \-----------------------------------------                                              
 *
 */
#define HAS_CXX11_VARIADIC_TEMPLATES 1  // needed to use ff_pipe
#include <ff/pipeline.hpp> // defines ff_pipeline and ff_pipe
#include <ff/farm.hpp> // defines ff_minode and ff_monode
using namespace ff;
const long NUMTASKS=20;
struct Stage0: ff_minode_t<long> {
    int svc_init() { counter=0; return 0;}
    long *svc(long *task) {
        if (task==nullptr) {
            for(long i=1;i<=NUMTASKS;++i)
                ff_send_out((void*)i);
            return GO_ON;
        }
	printf("Stage0 has got task %ld\n", (long)task);
        ++counter;
        if (counter == NUMTASKS) return EOS;
        return GO_ON;
    }
    long counter;
};
struct Stage1: ff_monode_t<long> {
    long *svc(long *task) {
        if ((long)task & 0x1) // sends odd tasks back
            ff_send_out_to(task, 0); 
        else ff_send_out_to(task, 1);
        return GO_ON;
    }
};
struct Stage2: ff_monode_t<long> {
    long *svc(long *task) {
        // sends back even tasks less than ...
        if ((long)task <= (NUMTASKS/2)) 
            ff_send_out_to(task, 0); 
        else ff_send_out_to(task, 1);
        return GO_ON;
    }
};
struct Stage3: ff_node_t<long> {
    long *svc(long *task) { 
        assert(((long)task & ~0x1) && (long)task>(NUMTASKS/2));
        return task; 
    }
};
int main() {
    Stage0 s0; Stage1 s1; Stage2 s2; Stage3 s3;

    ff_Pipe<long> pipe1(s0, s1);
    pipe1.wrap_around();

    ff_Pipe<long> pipe2(pipe1, s2);
    pipe2.wrap_around();
    
    ff_Pipe<long> pipe(pipe2, s3);
    pipe.wrap_around();

    if (pipe.run_and_wait_end()<0) error("running pipe");
    return 0;
}
