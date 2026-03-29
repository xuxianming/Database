#include "Utils/RunTime.h"
#include <utility>

RunTimeMonitor::RunTimeMonitor(std::string label) : m_label(std::move(label)) {}

RunTimeWoker::RunTimeWoker(RunTimeMonitor* leader) : m_leader(leader) {
    m_start_time = std::chrono::system_clock::now();
}
RunTimeWoker::~RunTimeWoker() {
    auto end_time = std::chrono::system_clock::now();
    auto duration = end_time - m_start_time;
    ++m_leader->m_count;
    m_leader->m_runtime += duration;
}

void RunTimeManger::Print() {
    if (m_fp == nullptr) {
        m_fp = fopen("runtime.log", "w");
    }
    fprintf(m_fp, "LABEL         COUNT           TIME        \n");
    for (const auto& kv : m_all_monitors) {
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      kv.second->m_runtime.time_since_epoch())
                      .count();
        fprintf(m_fp, "%s            %d          %ld\n",
                kv.second->m_label.c_str(), kv.second->m_count, ns);
    }
}

RunTimeManger* RunTimeManger::GetInstance() {
    static RunTimeManger inst;
    return &inst;
}
RunTimeMonitor* RunTimeManger::AddRunTimeMonitor(const std::string& label) {
    if (m_all_monitors.count(label) == 0U) {
        m_all_monitors[label] = new RunTimeMonitor(label);
    }
    return m_all_monitors[label];
}