// config/ConfigReader.h
#pragma once

#include "config.h"
#include <string>
#include <vector>
#include <filesystem>

class ConfigReader
{
public:
    // Load/Save with specific filepath
    static bool LoadFromFile(const std::string &filepath, _simulation_config &config);
    static bool SaveToFile(const std::string &filepath, const _simulation_config &config);

    // Get the absolute path to the config file
    static std::string GetConfigFilePath();

    // Ensure the config directory exists
    static bool EnsureConfigDirectoryExists();

private:
    static void TrimString(std::string &str);
    static bool ParseFloatArray(const std::string &value, std::vector<float> &outArray, int expectedSize);
    static bool ParseIntArray(const std::string &value, std::vector<int> &outArray, int expectedSize);
    static std::string FloatArrayToString(const float *array, int size);
    static std::string IntArrayToString(const int *array, int size);

    // Internal path helpers
    static std::filesystem::path GetExecutableDirectory();
    static std::filesystem::path GetConfigDirectory();
};