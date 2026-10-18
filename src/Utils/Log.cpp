#include "Utils/Log.h"
#include <cstdint>
#include <vector>

std::vector<const char*> log_level_name = {"DEBUG",   "INFO",  "CRITICAL",
                                           "WARNING", "ERROR", " "};

Logger* Logger::GetInstance() {
    static Logger inst;
    return &inst;
}
void Logger::LogInit() {
    char* log_levle = getenv("DB_LOG_LEVEL");
    if (log_levle != nullptr) {
        m_levle = atoi(log_levle);
    }
    m_ftream.open("/dev/dull");
}
std::ostream& Logger::Out(LogLevel LEVEL, const std::string& filepath,
                          int line) {
    std::ostream& out = GetStream(LEVEL);
    if (LEVEL < m_levle) {
        return out;
    }
    std::string filename = filepath.substr(filepath.find_last_of('/') + 1);
    out << "[" << log_level_name[static_cast<uint8_t>(LEVEL)] << "] ["
        << filename << ":" << line << "] ";
    return out;
}

std::ostream& Logger::GetStream(LogLevel level) {
    if (level < m_levle) {
        return m_ftream;
    }
    return std::cout;
}