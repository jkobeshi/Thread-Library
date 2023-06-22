#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include "thread.h"
#include <queue>

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::pair;
using std::string;
using std::make_pair;

int maxRequests; // stores arg maximum requests
vector<string> files; // stores file names
std::vector<pair<int,int>> requestQueue; // queue for requests
std::vector<int> numRequestsPerFile;
std::vector<int> numRequestsFufilled;
int totalRequests = 0;
int currentRequests = 0;
int currentTrack = 0;

vector<vector<int>> input {
    {785, 53},
    {350, 914},
    {827, 567},
    {302, 230},
    {631, 11}
};
mutex requestQueueMutex;
//mutex broadcast;
cv isFullcv;
//cv canBroadcastcv;
int numThreadsLive = 0;
int isFull = 0;		// global variable; shared between the two threads; condition
class QueueLock {
    public:
    QueueLock() {
        requestQueueMutex.lock();
    }

    ~QueueLock() {
        requestQueueMutex.unlock();
    }
};

bool hasPriorRequest(int id) {
    bool result = false;
    for (int i = 0; i < int(requestQueue.size()); ++i) {
        if (requestQueue[i].first == id) {
            result = true;
        }
    }
    return result;
}
int indexOfMin() {
    int indexOfSmallest = 0;
    int minDistanceDiff = abs(requestQueue[0].second - currentTrack);
    for(int i = 0; i < int(requestQueue.size()); ++i) {
        int distanceDiff = abs(requestQueue[i].second - currentTrack);
        if(distanceDiff < minDistanceDiff) {
            indexOfSmallest = i;
            minDistanceDiff = distanceDiff;
        }
    }
    return indexOfSmallest;
}
void servicer(void *a)
{   
    while(currentRequests < totalRequests) {
        QueueLock lock;
        while(!isFull){
            isFullcv.wait(requestQueueMutex);
        }
        if(!requestQueue.empty()){
            if (numThreadsLive < maxRequests) {
                while(int(requestQueue.size()) >= numThreadsLive && !requestQueue.empty()) {
                    int indexOfSmallest = indexOfMin();
                    //print_service(requestQueue[indexOfSmallest].first, requestQueue[indexOfSmallest].second);
                    cout << "-" << requestQueue[indexOfSmallest].first << requestQueue[indexOfSmallest].second << '\n';
                    currentTrack = requestQueue[indexOfSmallest].second;
                    ++numRequestsFufilled[requestQueue[indexOfSmallest].first];
                    if(numRequestsFufilled[requestQueue[indexOfSmallest].first] >= numRequestsPerFile[requestQueue[indexOfSmallest].first]) {
                        --numThreadsLive;
                    }
                    requestQueue.erase(requestQueue.begin() + indexOfSmallest);
                }
            }
            else {
                int indexOfSmallest = indexOfMin();
                //print_service(requestQueue[indexOfSmallest].first, requestQueue[indexOfSmallest].second);
                cout << "-" << requestQueue[indexOfSmallest].first << requestQueue[indexOfSmallest].second << '\n';
                currentTrack = requestQueue[indexOfSmallest].second;
                ++numRequestsFufilled[requestQueue[indexOfSmallest].first];
                if(numRequestsFufilled[requestQueue[indexOfSmallest].first] >= numRequestsPerFile[requestQueue[indexOfSmallest].first]) {
                    --numThreadsLive;
                }
                requestQueue.erase(requestQueue.begin() + indexOfSmallest);
            }
            int curMaxRequests = std::min(numThreadsLive, maxRequests);
            if(int(requestQueue.size()) < curMaxRequests) {
                isFull = 0;
                isFullcv.broadcast();
            }
        }
    }
    while(!requestQueue.empty()) {
        int indexOfSmallest = indexOfMin();
        //print_service(requestQueue[indexOfSmallest].first, requestQueue[indexOfSmallest].second);
        cout << "-" << requestQueue[indexOfSmallest].first << requestQueue[indexOfSmallest].second << '\n';
        currentTrack = requestQueue[indexOfSmallest].second;
        ++numRequestsFufilled[requestQueue[indexOfSmallest].first];
        if(numRequestsFufilled[requestQueue[indexOfSmallest].first] >= numRequestsPerFile[requestQueue[indexOfSmallest].first]) {
            --numThreadsLive;
        }
        requestQueue.erase(requestQueue.begin() + indexOfSmallest);
    }
}

void requester(void *id) {
    int idOfRequester =  (int)(intptr_t) id;
    //std::ifstream infile(files[idOfRequester]);

    int temp;
    //while (infile >> temp) {
    for (int i = 0; i < int(input[idOfRequester].size()); ++i) {
        temp = input[idOfRequester][i];
        QueueLock lock;
        while(isFull || hasPriorRequest(idOfRequester)){
            isFullcv.wait(requestQueueMutex);
        }
        requestQueue.push_back(make_pair(idOfRequester, temp));
        ++currentRequests;
        //print_request(idOfRequester, temp);
        cout << "+" << idOfRequester << temp << '\n';
        if (int(requestQueue.size()) >= maxRequests || int(requestQueue.size()) >= numThreadsLive) {
            isFull = 1;
            isFullcv.broadcast();
        }
    }
    //cout << "req: " << idOfRequester << "died\n";
}

void manageThreads(void* a){
    numThreadsLive = 5; //intptr_t(files.size());
    //for(intptr_t i = 0; i < intptr_t(files.size()); ++i){
    for(intptr_t i = 0; i < intptr_t(5); ++i){
        thread((thread_startfunc_t)requester, (void*)(intptr_t) i);
    }
    thread((thread_startfunc_t)servicer, 0);
}

int main(int argc, char *argv[])
{      
    //maxRequests = atoi(argv[1]);
    maxRequests = 3;
    for (int i = 0; i < int(input.size()); ++i) {
        int counterRequests = 0;
        for (int j = 0; j < int(input[0].size()); ++j) {
            ++totalRequests;
            ++counterRequests;
        }
        numRequestsPerFile.push_back(counterRequests);
        numRequestsFufilled.push_back(0);
    }
    /*for (int i = 2; i < argc; ++i) {
        files.push_back(string(argv[i]));
        std::ifstream infile(files[i-2]);
        int temp;
        int counterRequests = 0;
        while (infile >> temp) {
            ++totalRequests;
            ++counterRequests;
        }
        numRequestsPerFile.push_back(counterRequests);
        numRequestsFufilled.push_back(0);
    }*/
    cpu::boot(1, (thread_startfunc_t) manageThreads, 0, false, false, 0);
}