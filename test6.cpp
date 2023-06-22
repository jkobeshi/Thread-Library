#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex lock1;
mutex lock2;
cv x;

void tA(void *a){
    lock1.lock();
    cout << "in thread A\n";
    lock1.unlock();
}

void parent(void *a){
    cout << "thread A1 constr\n";
    thread thA1((thread_startfunc_t)tA, (void*)0);
    cout << "first join call\n";
    thA1.join();
    cout << "second join\n";
    thA1.join();
    cout << "After Join\n";
    /*
    cout << "second join call\n"; 
    thA1.join();
    cout << "finish\n";
    */
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}