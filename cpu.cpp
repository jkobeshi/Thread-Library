#include "impl.h"

//Global Vars
std::deque<std::pair<TCB*, int>> ready_queue;
std::pair<TCB*, int> runningTCB;
std::pair<TCB*, int> toDelete;
int pid_count = 0;

//Next() goes to the next context in the ready_queue
void next(){
    if(!ready_queue.empty()){
        ucontext_t* temp = runningTCB.first->UContext;
        runningTCB = ready_queue.front();
        ready_queue.pop_front();
        swapcontext(temp, runningTCB.first->UContext);
    }
    else{
        cpu::interrupt_enable_suspend();
    }
}

//Interrupt_handler handles interrupts using a Timer.
//swap back to runningTCB if empty 
    //do nothing
//else handle normally to manager
static void interrupt_handler() {
    cpu::interrupt_disable();
    if(!ready_queue.empty()) {
        ready_queue.push_back(runningTCB);
        next();
    }
    cpu::interrupt_enable();
}

//Wraps initial thread inside interrupts in order to satisfy our interrupts conditions
//called from os code, however thread initializer expects to come from user code - wrapper purpose is to avoid nesting interrupts
//call constructor as normal
//return to os code
void cpu::impl::thread_wrapper(thread_startfunc_t func, void* arg) {
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    thread InitThread((thread_startfunc_t) func, (void *)arg);
    cpu::interrupt_disable();
}

//cpu::init(func, arg) initializes cpu and at the same time initializes the inital given function
//along with arg by using thread wrapper.
void cpu::init(thread_startfunc_t func, void * arg) { this->impl_ptr->init(func, arg); }
void cpu::impl::init(thread_startfunc_t func, void* arg){
    assert_interrupts_disabled();
    //set-up interrupt vector
    cpu::self()->interrupt_vector_table[TIMER] = interrupt_handler;
    cpu::impl::thread_wrapper((thread_startfunc_t) func, (void *)arg); 
    
    runningTCB = ready_queue.front();
    ready_queue.pop_front();
    setcontext(runningTCB.first->UContext);
}





