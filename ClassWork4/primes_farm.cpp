#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "ff/farm.hpp"

using namespace std;
using namespace ff;

using ull = unsigned long long;

// see http://en.wikipedia.org/wiki/Primality_test
static bool is_prime(ull n) {
    if (n <= 3)  return n > 1; // 1 is not prime !

    if (n % 2 == 0 || n % 3 == 0) return false;

    for (ull i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

// typedef ull ull;

struct Emitter : ff_node_t<ull>{
  ff_loadbalancer *lb;
  ull start;
  ull end;
  bool print;
  std::vector<ull> results;

  Emitter(ull start, ull end, bool print, ff_loadbalancer *lb) : start(start), end(end), print(print), lb(lb){
    results.reserve((size_t)(end-start)/log(start));
  }

  ull* svc(ull *task) {
    int channel = lb->get_channel_id();
    // cout << "emetter channel in:" << channel<<endl;
  	if (channel < 0){

      for (ull i = this->start; i<this->end; ++i) {
        // cout << "send Task " << i << endl;
        ff_send_out((void*)i);
      }
      lb->broadcast_task(EOS);
      return GO_ON;

    }else{

      this->results.push_back((long)task);
      //delete task;
      return GO_ON;
    }
  }

    void svc_end(){
      const size_t n = this->results.size();
      std::cout << "Found " << n << " primes\n";
      if (this->print) {
        for (const int &i : this->results)
          cout << i << " " << endl << endl;
      }
    }
};

ull *F(ull *in,ff_node*const) {
    if (is_prime((long)in)){
      return in;
    }else{
      return (ull*)GO_ON;
    }
}

int main(int argc, char *argv[]) {
    if (argc<3) {
        std::cerr << "use: " << argv[0]  << " number1 number2 [print=off|on]\n";
        return -1;
    }
    ull n1          = std::stoll(argv[1]);
    ull n2          = std::stoll(argv[2]);
    bool print_primes = false;
    if (argc >= 4)  print_primes = (std::string(argv[3]) == "on");

    ff_Farm<ull>  farm(F, 4);
    farm.remove_collector(); // removes the default collector
    // the scheduler gets in input the internal load-balancer
    Emitter S(n1, n2, print_primes, farm.getlb());
    farm.add_emitter(S);
    // adds feedback channels between each worker and the scheduler
    farm.wrap_around();

    if (farm.run_and_wait_end()<0) error("running farm");
    return 0;
}
