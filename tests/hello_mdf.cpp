/* *************************** */
/* ******* hello_mdf.cpp ***** */

/*                                  ________
 *                                 /        \         
 *                                 |         \        
 * A = A + B;     // sum2    A     B     C   |     D  
 * C = C + B;     // sum2    |     |     |   |     |  
 * D = D + B;     // sum2     \   / \   /     \   /   
 * D = A + C + D; // sum3       +     +         +     
 *                              A     C         D     
 *                              |     |         |     
 *                               \     \       / 
 *                                ----- + -----        
 *                                      D                                                              
 */
#include <ff/mdf.hpp>
using namespace ff;
const size_t SIZE = 1<<8; //1<<20;
void sum2(long *X, long *Y, const long size) {
    for(long i=0;i<size;++i)
        X[i] += Y[i];
}
void sum3(long *X, long *Y, long *Z, const long size) {
    for(long i=0;i<size;++i)
        X[i] += Y[i] + Z[i];
}
template<typename T>
struct Parameters {
    long *A,*B,*C,*D;
    T* mdf;
};
void taskGen(Parameters<ff_mdf > *const P){
    long *A = P->A;  long *B = P->B;
    long *C = P->C;  long *D = P->D;    
    auto mdf = P->mdf;
    
    std::vector<param_info> Param;
    // A = A + B;
    {
	const param_info _1={(uintptr_t)A,INPUT};  // 1st param
	const param_info _2={(uintptr_t)B,INPUT};  // 2nd param
	const param_info _3={(uintptr_t)A,OUTPUT}; // 3rd param
	// pack the parameters in one single vector
	Param.push_back(_1); Param.push_back(_2); 
	Param.push_back(_3);  
	mdf->AddTask(Param, sum2, A,B,SIZE); // create on task
    }
    // C = C + B;
    {
	Param.clear();
	const param_info _1={(uintptr_t)C,INPUT};
	const param_info _2={(uintptr_t)B,INPUT};
	const param_info _3={(uintptr_t)C,OUTPUT};
	Param.push_back(_1); Param.push_back(_2); Param.push_back(_3);
	mdf->AddTask(Param, sum2, C,B,SIZE);
    }
    // D = D + B;
    {
	Param.clear();
	const param_info _1={(uintptr_t)D,INPUT};
	const param_info _2={(uintptr_t)B,INPUT};
	const param_info _3={(uintptr_t)D,OUTPUT};
	Param.push_back(_1); Param.push_back(_2); Param.push_back(_3);
	mdf->AddTask(Param, sum2, D,B,SIZE);
    }
    // D = A + C + D;
    { 
	Param.clear();
	const param_info _1={(uintptr_t)A,INPUT};
	const param_info _2={(uintptr_t)C,INPUT};
	const param_info _3={(uintptr_t)D,INPUT};
	const param_info _4={(uintptr_t)D,OUTPUT};
	Param.push_back(_1); Param.push_back(_2); 
	Param.push_back(_3); Param.push_back(_4);
	mdf->AddTask(Param, sum3, D,A,C,SIZE);
    }
}
int main() {
    long *A= new long[SIZE], *B= new long[SIZE]; 
    long *C= new long[SIZE], *D= new long[SIZE];
    for(size_t i=0;i<SIZE;++i) {
	A[i]=i; B[i]=i+1; C[i]=i+2; D[i]=i+3;
    }
    // creates the mdf object passing the task generetor function
    Parameters<ff_mdf > P;
    ff_mdf dag(taskGen, &P);
    P.A=A,P.B=B,P.C=C,P.D=D,P.mdf=&dag;
    
    if (dag.run_and_wait_end() <0) error("running dag");
    for(size_t i=0;i<SIZE;++i) printf("%ld ", D[i]); printf("\n");
    delete [] A; delete [] B; delete [] C; delete [] D;
    return 0;
}



