#include "impl.h"

//Points impl_ptr to new memory
//Check for bad_alloc
mutex::mutex() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    try {
        this->impl_ptr = new impl();
    }
    catch (const std::bad_alloc& ba) {
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    cpu::interrupt_enable();
}
mutex::impl::impl(){}

//Deallocate impl_ptr
mutex::~mutex() { delete this->mutex::impl_ptr; }
mutex::impl::~impl(){}

//Lock() gives lock to TCB that is Holding the lock by letting it run
//and Locks the other threads up if they are using the same lock
//These locked up threads will wait for their turn in FIFO order to
//obtain the lock
void mutex::lock() { this->impl_ptr->lock(); }
void mutex::impl::lock() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();    
    if (!free) {
        wait_queue.push_back(runningTCB);
        next();
    }
    else {
        free = false;
        HoldingLock = runningTCB.second;
    }
    cpu::interrupt_enable();
}

//Unlock() gives the lock to the next TCB on mutex's wait queue if it is not empty.
void mutex::unlock() { this->impl_ptr->unlock(); }
void mutex::impl::unlock() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    //free lock being attempted unlock - throw error
    if(HoldingLock == -1) {
        cpu::interrupt_enable();
        throw std::runtime_error("");
    }
    //lock being unlocked by thread that does not own lock - throw error
    else if((HoldingLock != runningTCB.second) && (HoldingLock != -1)) {
        cpu::interrupt_enable();
        throw std::runtime_error("");
    }
    free = true;
    HoldingLock = -1;
    if (!wait_queue.empty()) {
        HoldingLock = wait_queue.front().second;
        ready_queue.push_back(this->wait_queue.front());
        wait_queue.pop_front();
        free = false;
    }
    cpu::interrupt_enable();
}