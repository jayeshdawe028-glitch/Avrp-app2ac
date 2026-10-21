#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class CFPSFix
{
private:
    void Routine();

    std::mutex m_Mutex;
    std::vector<pid_t> m_Threads;
    std::atomic<bool> m_Running{false};
    std::thread m_Worker;

public:
    CFPSFix();
    ~CFPSFix();

    void PushThread(pid_t tid);
};
