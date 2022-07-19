export;
#pragma once
#include <../../SomeLogger.h>
export module logger;

export void LOG_ERROR(const std::string& s) { SomeLogger::Logger::Instance().printEndl(true).log(SomeLogger::LoggerLevel::ERR) << s; }
export void LOG_INFO(const std::string& s) { SomeLogger::Logger::Instance().printEndl(true).log(SomeLogger::LoggerLevel::INFO) << s; }
export void LOG_WARNING(const std::string& s) { SomeLogger::Logger::Instance().printEndl(true).log(SomeLogger::LoggerLevel::WARNING) << s; }
