#pragma once
#include <fstream>
#include <string>
#include <result.h>
#include <serdepp/include/serdepp/serializer.hpp>
#include "../config.h"


namespace KUMA::UTILS {

    struct Error {
        enum class Type {
            FILE_NOT_EXIST,
        };

        Error() = delete;
        Error(Type type, std::string msg): type(type), msg(msg) {};

        Type type;
        std::string msg;
    };

    static std::string getRealPath(const std::string& p_path) {
        std::string result;
        if (std::filesystem::exists(Config::ROOT + Config::ENGINE_ASSETS_PATH + p_path)) {
            result = Config::ROOT + Config::ENGINE_ASSETS_PATH + p_path;
        }
        else {
            result = Config::ROOT + Config::USER_ASSETS_PATH + p_path;
        }
        return result;
    }

	template<class T>
    Result<T, Error> loadConfigFile(const std::string& path) {
        auto realPath = getRealPath(path);
        std::ifstream ifile(realPath);
        if (!ifile.is_open()) {
            return Err(Error(Error::Type::FILE_NOT_EXIST, "loadConfigFile: " + realPath + " file not exist"));
        }
        std::stringstream buffer;
        buffer << ifile.rdbuf();
        return Ok<T>(serde::deserialize<T>(buffer));
	}

}
