#pragma once

#include "logger.h"
#include "resourceModule/serviceManager.h"

#define LOG_INFO IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::UTILS::LOGG::Logger>().info()
#define LOG_WARNING IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::UTILS::LOGG::Logger>().warning()
#define LOG_ERROR IKIGAI::RESOURCES::ServiceManager::Get<IKIGAI::UTILS::LOGG::Logger>().error()
