#include "threadpool.h"
// i do not get a lot of parts

ThreadPool::ThreadPool(int numThreads) : m_ActiveJobs(0), m_Shutdown(false) {
    for (int i = 0; i < numThreads; i++) {
        m_Workers.emplace_back(&ThreadPool::WorkerThread, this, i);
    }
}

ThreadPool::~ThreadPool() {
    m_Shutdown = true;
    m_Condition.notify_all();
    
    for (auto& thread : m_Workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadPool::WorkerThread(int threadID) {
    while (true) {
        std::function<void()> job;
        
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Condition.wait(lock, [this] { 
                return !m_WorkQueue.empty() || m_Shutdown; 
            });
            
            if (m_Shutdown && m_WorkQueue.empty()) {
                return;
            }
            
            if (!m_WorkQueue.empty()) {
                job = std::move(m_WorkQueue.front());
                m_WorkQueue.pop();
            }
        }
        
        if (job) {
            try {
                job();
            } catch (...) {

            }

            int jobsLeft = --m_ActiveJobs; 
            
            if (jobsLeft == 0) {
                m_WaitCondition.notify_all();
            }
        }
    }
}

void ThreadPool::SubmitWork(std::function<void()> work) {
    m_ActiveJobs++; 
    
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_WorkQueue.push(std::move(work)); 
    }
    m_Condition.notify_one();
}

void ThreadPool::WaitForCompletion() {
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_WaitCondition.wait(lock, [this]() {
        return m_ActiveJobs == 0 && m_WorkQueue.empty();
    });
}
