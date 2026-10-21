#include "CFPSFix.h"
#include "main.h"
#include <sys/syscall.h>
#include <sched.h>
#include <errno.h>

static void setThreadAffinityMask(pid_t tid, uint32_t mask)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    for (uint32_t cpu = 0; cpu < 32; ++cpu)
    {
        if (mask & (1u << cpu))
            CPU_SET(cpu, &cpuset);
    }

    (void)sched_setaffinity(tid, sizeof(cpuset), &cpuset);
}

void CFPSFix::Routine()
{
    while (m_Running.load(std::memory_order_acquire))
    {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            for (auto it = m_Threads.begin(); it != m_Threads.end(); )
            {
                const pid_t tid = *it;
                if (tid > 0)
                {
                    setThreadAffinityMask(tid, 0xff);
                    ++it;
                }
                else
                {
                    it = m_Threads.erase(it);
                }
            }
        }

        for (int i = 0; i < 10 && m_Running.load(std::memory_order_acquire); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

CFPSFix::CFPSFix()
    : m_Running(true),
      m_Worker(&CFPSFix::Routine, this)
{
}

CFPSFix::~CFPSFix()
{
    m_Running.store(false, std::memory_order_release);

    if (m_Worker.joinable())
        m_Worker.join();
}

void CFPSFix::PushThread(pid_t tid)
{
    if (tid <= 0)
        return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    for (pid_t existing : m_Threads)
    {
        if (existing == tid)
            return;
    }

    m_Threads.push_back(tid);
}
