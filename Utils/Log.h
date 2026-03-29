#ifndef UTILS_LOG_H_
#define UTILS_LOG_H_

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
enum LogLevel : uint8_t {
    DEBUG,
    INFO,
    CRITICAL,
    WARNING,
    ERROR,
    UNKNOWN,
};

class Logger {
public:
    static Logger*  GetInstance();
    void            LogInit();
    inline LogLevel GetLogLevel() { return static_cast<LogLevel>(m_levle); }
    std::ostream&   Out(LogLevel LEVEL, const std::string& filepath, int line);

private:
    std::ostream& GetStream(LogLevel level);

private:
    friend std::stringstream;
    uint8_t       m_levle = static_cast<uint8_t>(LogLevel::ERROR);
    std::ofstream m_ftream;
};

#define DB_LOG(LEVEL) Logger::GetInstance()->Out(LEVEL, __FILE__, __LINE__)

#endif