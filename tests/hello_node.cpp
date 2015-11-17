#include <iostream>
#include <ff/node.hpp>
using namespace ff;
struct myNode:ff_node {
    // called once at the beginning of node's life cycle
    int svc_init() {
	std::cout << "Hello, I'm (re-)starting...\n";
	counter = 0;
	return 0; // 0 means succes
    }
    // called for each input task of the stream, or until 
    // the EOS is returned if the node has no input stream
    void *svc(void *task) {       
	if (++counter > 5) return EOS;
	std::cout << "Hi! (" << counter << ")\n";
	return GO_ON; // keep calling me again
    }
    // called once at the end of node's life cycle
    void svc_end() { std::cout << "Goodbye!\n"; }
    
    
    // starts the node and waits for its termination
    int  run_and_wait_end(bool=false) { 
	if (ff_node::run() < 0) return -1;
	return ff_node::wait();
    }
    // first sets the freeze flag then starts the node
    int  run_then_freeze() { 
        if (isfrozen()) {
            thaw(true);// true means that next time threads are frozen again
            return 0;
        }
	return ff_node::freeze_and_run(); 
    }
    // waits for node pause (i.e. until the node is put to sleep) 
    int  wait_freezing()   { return ff_node::wait_freezing();}
    // waits for node termination
    int  wait()            { return ff_node::wait();}
    
    long counter;    
};
int main() {
    myNode mynode;
    if (mynode.run_and_wait_end()<0)
	error("running myNode"); 
    std::cout << "first run done\n\n";
    long i=0;
    do {
	if (mynode.run_then_freeze()<0)
	    error("running myNode");
	if (mynode.wait_freezing())
	    error("waiting myNode");
	std::cout << "run " << i << " done\n\n";
    } while(++i<3);
    if (mynode.wait())
	error("waiting myNode");

    return 0;
}
