#include "impl.h"

//Points impl_ptr to new memory
//Check for bad_alloc
cv::cv() {
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
cv::impl::impl(){}

//Deallocate impl_ptr
cv::~cv() { delete this->impl_ptr; }
cv::impl::~impl(){}

//Wait(mut) pushes back the current thread onto this cv's wait queue.
void cv::wait(mutex& mut) { this->impl_ptr->wait(mut); }
void cv::impl::wait(mutex& mut) {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    //illegal to call wait on unlocked mutex
    if(mut.impl_ptr->HoldingLock == -1) {
        cpu::interrupt_enable();
        throw std::runtime_error("");
    }
    //illegal to call wait on lock you don't own
    else if(mut.impl_ptr->HoldingLock != runningTCB.second){
        cpu::interrupt_enable();
        throw std::runtime_error("");
    }
    wait_queue.push_back(runningTCB);
    unlock_wrapper(mut);
    next();
    lock_wrapper(mut);
    cpu::interrupt_enable();
}

//Signal() releases the first thing in cv's wait_queue if it is not empty
//while maintaining FIFO order.
void cv::signal() { impl_ptr->signal(); }
void cv::impl::signal() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    if(!wait_queue.empty()){
        ready_queue.push_back(wait_queue.front());
        wait_queue.pop_front();
    }
    cpu::interrupt_enable();
}

//Broadcast() releases everything in cv's wait_queue if it is not empty
//while maintaing FIFO order.
void cv::broadcast() { impl_ptr->broadcast(); }
void cv::impl::broadcast() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while(!wait_queue.empty()) {
        ready_queue.push_back(wait_queue.front());
        wait_queue.pop_front();
    }
    cpu::interrupt_enable();
}

//Unlock and Lock wrapper calls lock or unlock on the given mutex, mut, and
//is wrapped around interrupt enable and disable to make sure interrupts is
//used correctly, given this order...
//called from os code, however unlock expects to come from user code - wrapper purpose is to avoid nesting interrupts
//call constructor as normal
//return to os code
void cv::impl::unlock_wrapper(mutex& mut) {
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    mut.unlock();
    cpu::interrupt_disable();
}
void cv::impl::lock_wrapper(mutex& mut) {
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    mut.lock();
    cpu::interrupt_disable();
}