#ifndef PROJECT_PATH_HEADER
#define PROJECT_PATH_HEADER

#include <filesystem>
#include <string>

std::string getFileName(const std::string& path) {
    return filesystem::path(path).filename().string();
}

std::string projectPath(const std::string& path) {
    return (projectDir / filesystem::path(path)).string();
}

#endif // PROJECT_PATH_HEADER