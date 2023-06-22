#include "thread.h"
#include <deque>
#include <stdio.h>
#include <stdexcept>
#include <utility>
#include <ucontext.h>

//TCB Struct
struct TCB {
    std::deque<std::pair<TCB*, int>> join_queue;
    ucontext_t* UContext;
    char* stack_ptr;
};

//Global vars & functions
extern std::deque<std::pair<TCB*, int>> ready_queue;
extern std::pair<TCB*, int> runningTCB;
extern std::pair<TCB*, int> toDelete;
extern int pid_count;
extern void stub(thread_startfunc_t, void*);
extern void next();

//All impl classes for thread, cpu, mutex, and cv
class thread::impl {
public:
    impl(thread_startfunc_t, void*);
    ~impl();
    void join();
    static void yield();
    std::pair<TCB*, int> tcb;
};

class cpu::impl {
public:
    void init(thread_startfunc_t , void*);
    void thread_wrapper(thread_startfunc_t , void*);
};

class mutex::impl {
public:
    impl();
    ~impl();
    void lock();
    void unlock();
    std::deque<std::pair<TCB*,int>> wait_queue;
    int HoldingLock = -1;
private:
    bool free = true;
};

class cv::impl {
public:
    impl();
    ~impl();
    void wait(mutex&);
    void signal();
    void broadcast();
    void unlock();
    void unlock_wrapper(mutex&);
    void lock_wrapper(mutex&);
    std::deque<std::pair<TCB*,int>> wait_queue;
};