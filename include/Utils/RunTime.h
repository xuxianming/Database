#ifndef UTILS_RUNTIME_H_
#define UTILS_RUNTIME_H_

#include <chrono>
#include <map>
#include <string>

class RunTimeMonitor {
public:
    explicit RunTimeMonitor(std::string label);
    ~RunTimeMonitor() = default;
    const std::string                                  m_label;
    std::chrono::time_point<std::chrono::system_clock> m_runtime;
    uint32_t                                           m_count;
};

class RunTimeWoker {
public:
    explicit RunTimeWoker(RunTimeMonitor* leader);
    ~RunTimeWoker();

private:
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    RunTimeMonitor*                                    m_leader;
};

class RunTimeManger {
private:
    RunTimeManger() = default;

public:
    static RunTimeManger* GetInstance();
    RunTimeMonitor*       AddRunTimeMonitor(const std::string& label);
    void                  Print();

private:
    std::map<std::string, RunTimeMonitor*> m_all_monitors;
    FILE*                                  m_fp;
};

#define RUNTIME_MONITOR_REGISTER(label) \
    RunTimeWoker worker(RunTimeManger::GetInstance()->AddRunTimeMonitor(label));

#define PRINT_RUNTIME_LOG() RunTimeManger::GetInstance()->Print();

#endif
