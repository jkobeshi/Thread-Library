#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex lock1;
mutex lock2;
cv x;

int count = 0;

void tA(void *a){
    lock1.lock();
    cout << (char*)a << "Aquired Lock 1\n";
    lock2.lock();
    cout << (char*)a << "Aquired Lock 2\n";
    while(count < 2){
        x.wait(lock2);
    }
    lock2.unlock();
    cout << (char*)a << "Done Lock 2\n";
    lock1.unlock();
    cout << (char*)a << "Dont Lock 1\n";
}

void tB(void *a){
    lock1.lock();
    cout << (char*)a << "Aquired Lock 1\n";
    lock2.lock();
    cout << (char*)a << "Aquired Lock 2\n";
    count = 2;
    x.signal();
    lock2.unlock();
    cout << (char*)a << "Done Lock 2\n";
    lock1.unlock();
    cout << (char*)a << "Dont Lock 1\n";
}

void parent(void *a){
    cout << "thread A constr\n";
    thread thA(tA, (void*)"A");
    cout << "thread B constr\n";
    thread thB(tB, (void*)"B"); 
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}