#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <limits.h>
#include <unordered_map>
#include <string>

#include "thread.h"
#include "cv.h"
#include "mutex.h"
#include "cpu.h"
#include "disk.h"


int max_requests = 0;
std::vector<std::string> in_files;
//pair<requester, track>
std::vector<std::pair<int, int>> queue;

//map to keep track of if the requester already has a request in the queue(can't request before completion)
std::unordered_map<int, bool> map;

mutex cout_mutex;
mutex num_active_requesters_mutex;
mutex queue_mutex;
mutex map_mutex;
//mutex disk_track_mutex;

cv requester_cv;
cv servicer_cv;
cv in_queue_cv;

int disk_track = 0;
int num_active_requesters = 0;

void servicer(void *a) {
    
    while(num_active_requesters > 0 || queue.size() > 0) {
    //lock shared data
    queue_mutex.lock();
    num_active_requesters_mutex.lock();

    //if the queue is full/maxed out on requesters, thats when service is needed - WRONG, CASE OF 

    while(int(queue.size()) < std::min(max_requests, num_active_requesters)) {
        num_active_requesters_mutex.unlock();
        servicer_cv.wait(queue_mutex);       
        num_active_requesters_mutex.lock(); 
    }
    //disk request logic
    int min = 1000;
    int min_index = 0;
    for(int i = 0; i < int(queue.size()); i++) {
        int temp_min = abs(disk_track - queue[i].second);
        if(temp_min < min) {
            min = temp_min;
            min_index = i;
        }
    }
    cout_mutex.lock();
    if(queue.size() != 0) {
        //print_service(queue[min_index].first, queue[min_index].second);
        std::cout << "Service requester" << queue[min_index].first << "track " << queue[min_index].second << std::endl;
        
    }
    cout_mutex.unlock();
    
    //disk track update 
    
    disk_track = queue[min_index].second;
    
    //thread is now out of queue
    map_mutex.lock();
    map[queue[min_index].first] = false;
    map_mutex.unlock();
    
    in_queue_cv.broadcast();

    //remove from queue
    if(queue.size() != 0) {
        queue.erase(queue.begin() + min_index);
    }
    /*
    cout_mutex.lock();
    std::cout << "queue: ";
    for(int i = 0; i < int(queue.size()); i++) {
        std::cout << queue[i].first << " ";
    }
    std::cout << std::endl;
    std::cout << "num_active_reqs: " << num_active_requesters << std::endl;
    cout_mutex.unlock();
    */
    
    //wake up all requester threads
    requester_cv.broadcast();
    //need to check this

    //unlock shared data
    num_active_requesters_mutex.unlock();
    queue_mutex.unlock();
    }
    

} 

void requester(int i) {
    std::ifstream infile(in_files[i]);
    int mem_location = 0;

    //while requests remaining in thread, load
    while(infile >> mem_location) {

        //lock queue
        queue_mutex.lock();
        num_active_requesters_mutex.lock();
        map_mutex.lock();
        //while queue isn't at max size cv or the track has already been requested
        while(int(queue.size()) >= std::min(max_requests, num_active_requesters) || map[i] == true) {
            map_mutex.unlock();
            num_active_requesters_mutex.unlock(); 
            requester_cv.wait(queue_mutex);
            num_active_requesters_mutex.lock();
            map_mutex.lock();
        }
        map_mutex.unlock();
        num_active_requesters_mutex.unlock();
        //push request back into queue
        queue.push_back(std::make_pair(i, mem_location));
        //thread is now in queue
        map_mutex.lock();
        map[i] = true;
        map_mutex.unlock();

        cout_mutex.lock();
        std::cout << "Requester " << i<< "on track " << mem_location << std::endl;
        //print_request(i, mem_location);
        cout_mutex.unlock();
        //wake up the servicer
        servicer_cv.signal();
        //drop lock
        queue_mutex.unlock();
    }
    
    //track number of active requester threads - PROBLEM
    map_mutex.lock();
    while(map[i] == true) {
        in_queue_cv.wait(map_mutex);
    }
    map_mutex.unlock();
    servicer_cv.signal();
    
    num_active_requesters_mutex.lock();
    num_active_requesters = num_active_requesters - 1;
    num_active_requesters_mutex.unlock();
    servicer_cv.signal();
}

void thread_manager() {
    //start all requester and the servicer threads
    for(intptr_t i = 0 ; i < intptr_t(in_files.size()); i++) {
        thread((thread_startfunc_t)requester, (void*) i);
    }
    thread((thread_startfunc_t)servicer, (void*)0);
}

int main(int argc, char **argv) {
    
    max_requests = std::stoi(argv[1]);

    num_active_requesters = argc-2;
    in_files.resize(argc - 2);
    
    for(int i = 0; i < argc - 2; i++) {
        in_files[i] = argv[i + 2];
        map[i] = false;
    }
//REMEMBER TO CHANGE THIS BACK ON _SUBMIT
    cpu::boot(1, (thread_startfunc_t) thread_manager, (void *) 100, 0, 0, 0);
}
