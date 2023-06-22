#include <iostream>
#include <cstdlib>
#include "thread.h"
#include "cpu.h"
#include "mutex.h"

using std::cout;
using std::endl;

mutex m;

//mutex 12
void funcA(void *a) {
    assert_interrupts_enabled();
    try
    {
        m.unlock();
    }
    catch(const std::runtime_error& e)
    {
        std::cout << "caught";
    }
    std::cout << "done\n";
    assert_interrupts_enabled();

}
void boot_func(void *) {
    
    thread tA(funcA, (void*)1);
    
}
int main()
{
    //throw std::bad_alloc();
    cout << "T1\n";
    cpu::boot(1, (thread_startfunc_t) boot_func, (void *) 100, false, false, 0);
}
