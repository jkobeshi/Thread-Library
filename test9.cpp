#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex lock1;
mutex lock2;
cv x;

int count = 0;

//testing FIFO of mutex

void tA(void *a){
    lock1.lock();
    cout << (char*)a << "Aquired Lock 1\n";
    thread::yield();
    cout << "return from yield\n";
    //assert_interrupts_enabled();
    lock1.unlock();
    //assert_interrupts_enabled();
}

void tB(void *a){
    thread::yield();
    lock1.lock();
    cout << "B recieved lock\n";
    lock1.unlock();
}
void tC(void *a) {
    lock1.lock();
    cout << "C recieved lock\n";
    lock1.unlock();
}

void parent(void *a){
    
    cout << "thread A constr\n";
    thread thA(tA, (void*)"A");
    
    cout << "thread B constr\n";
    thread thB(tB, (void*)"B"); 
    cout << "thread C constr\n";
    thread thC(tC, (void*)"C");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}