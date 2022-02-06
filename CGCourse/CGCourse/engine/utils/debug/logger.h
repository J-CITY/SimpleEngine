#pragma once
#include <../../SomeLogger.h>

#define LOG_ERROR(s) SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::ERR) << s;
#define LOG_INFO(s) SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::INFO) << s;
#define LOG_WARNING(s) SomeLogger::Logger::Instance().log(SomeLogger::LoggerLevel::WARNING) << s;

