#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex lock1;
mutex lock2;
cv x;


void tA(void *a){
    cout << (intptr_t)a << " Locking\n";
    lock1.lock();
    cout << (intptr_t)a << " Yielding\n";
    thread::yield();
    cout << (intptr_t)a << " return from Yield\n";
    lock1.unlock();
    cout << (intptr_t)a << " return from Lock\n";
}

int count = 0;
void tB(void *a){
    cout << (intptr_t)a << " Locking\n";
    lock1.lock();
    while (count < 1){
        x.wait(lock1);
    }
    cout << (intptr_t)a << " Yielding\n";
    thread::yield();
    cout << (intptr_t)a << " return from Yield\n";
    lock1.unlock();
    cout << (intptr_t)a << " return from Lock\n";
}

void tC(void *a){
    lock1.lock();
    cout << (intptr_t)a << " Aquired Lock 1\n";
    count = 1;
    x.signal();
    lock1.unlock();
    cout << (intptr_t)a << " Done Lock 1\n";
}

void parent(void *a){
    cout << "thread A1 constr\n";
    thread thA1(tA, (void*)1);
    cout << "thread A2 constr\n";
    thread thA2(tA, (void*)2);
    cout << "thread A3 constr\n";
    thread thA3(tA, (void*)3);
    cout << "thread A4 constr\n";
    thread thA4(tA, (void*)4);
    cout << "thread A5 constr\n";
    thread thA5(tA, (void*)5);
    thA5.join();
    cout << "\n\n";
    cout << "thread B1 constr\n";
    thread thB1(tB, (void*)1);
    cout << "thread B2 constr\n";
    thread thB2(tB, (void*)2);
    cout << "thread B3 constr\n";
    thread thB3(tB, (void*)3);
    cout << "thread B4 constr\n";
    thread thB4(tB, (void*)4);
    cout << "thread B5 constr\n";
    thread thB5(tB, (void*)5);
    cout << "thread C0 constr\n";
    thread thC1(tC, (void*)0);
    
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}