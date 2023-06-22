#include <iostream>
#include <cstdlib>
#include "thread.h"
#include "cpu.h"
#include "mutex.h"

using std::cout;
using std::endl;

mutex m;
cv c;

//cv error checking
void funcA(void *a) {
    try
    {
    c.wait(m);
    }
    catch(const std::runtime_error& m)
    {
        std::cout << "caught";
    }
    
    std::cout << "done\n";
}
void boot_func(void *) {
        
    thread tA(funcA, (void*)1);

}
int main()
{
    //throw std::bad_alloc();
    cout << "T2\n";
    cpu::boot(1, (thread_startfunc_t) boot_func, (void *) 100, false, false, 0);
}
