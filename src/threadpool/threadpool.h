#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(int numThreads);
    ~ThreadPool();
    
    void SubmitWork(std::function<void()> work);
    void WaitForCompletion();

private:
    void WorkerThread(int threadID);

    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_WorkQueue;
    
    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    
    // For WaitForCompletion
    std::condition_variable m_WaitCondition; 
    std::atomic<int> m_ActiveJobs; 
    std::atomic<bool> m_Shutdown;
};
