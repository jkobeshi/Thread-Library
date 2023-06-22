#include "impl.h"

//Calls impl constructor to initialize and allocate per-thread: TCB and stack.
//Error checking: Bad_Alloc
thread::thread(thread_startfunc_t func, void* arg) {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    try {
        this->impl_ptr = new impl(func, arg);
    }
    catch (const std::bad_alloc& ba){
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    cpu::interrupt_enable();
}
//allocate per-thread state: TCB and stack
//Pushes TCB onto the ready queue
thread::impl::impl(thread_startfunc_t func, void* arg) {
    TCB* temp = new TCB();
    ucontext_t* ucontext_ptr = new ucontext_t();
    this->tcb.first = temp;
    this->tcb.second = pid_count;
    ++pid_count;
    char *stack = new char [STACK_SIZE];
    this->tcb.first->stack_ptr = stack;
    ucontext_ptr->uc_stack.ss_sp = stack;
    ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
    ucontext_ptr->uc_stack.ss_flags = 0;
    ucontext_ptr->uc_link = nullptr;
    makecontext(ucontext_ptr, (void (*)()) stub, 2, func, arg);
    this->tcb.first->UContext = ucontext_ptr;
    ready_queue.push_back(this->tcb);
}

//deallocate memory from impl->ptr
thread::~thread() { delete this->impl_ptr; }
thread::impl::~impl() { }

//Pushes the currently runningTCB back onto join queue
//that current runningTCB will not run until "this" finishes
//Do nothing if current "this" is terminated"
void thread::join(){ this->impl_ptr->join(); }
void thread::impl::join() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    if(this->tcb.second != -1){
        this->tcb.first->join_queue.push_back(runningTCB);
        next();
    }
    cpu::interrupt_enable();
}

//Switches runningTCB with the front of queue by putting runningTCB 
//in the back of the then switching with front.
//If ready queue is empty we do nothing
//else we push runningTCB to the back of the queue.
//we then let manager handle the rest.
void thread::yield(){ thread::impl::yield(); }
void thread::impl::yield() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    if(!ready_queue.empty()){
        ready_queue.push_back(runningTCB);
        next();
    }
    cpu::interrupt_enable();
}

//Function Stub
//after function finishes, we do everything we need after the thread exits.
//we pop everything out of its join queue if there is any and push back to ready queue
//then we set the current TCB to terminated.
//this is where to enable before switching to user code - cannot violate switch invariant by enabling before swapcontext
void stub(thread_startfunc_t func, void* arg) {
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    (*func)(arg);
    cpu::interrupt_disable();
    if(toDelete.second != -1){
        delete []toDelete.first->stack_ptr;
        delete toDelete.first->UContext;
        delete toDelete.first;
    }
    while(!runningTCB.first->join_queue.empty()){
        ready_queue.push_back(runningTCB.first->join_queue.front());
        runningTCB.first->join_queue.pop_front();
    }
    runningTCB.second = -1;
    toDelete = runningTCB;
    if(!ready_queue.empty()){
        runningTCB = ready_queue.front();
        ready_queue.pop_front();
        setcontext(runningTCB.first->UContext);
    }
    else {
        cpu::interrupt_enable_suspend();
    }    
}


