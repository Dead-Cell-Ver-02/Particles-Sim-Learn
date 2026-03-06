// config/ConfigReader.cpp
#include "config/ConfigReader.h"
#include <raylib.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// ============================================================================
// PATH MANAGEMENT
// ============================================================================

std::filesystem::path ConfigReader::GetExecutableDirectory()
{
    return fs::current_path();
}

std::filesystem::path ConfigReader::GetConfigDirectory()
{
    return GetExecutableDirectory() / "assets" / "presets";
}

std::string ConfigReader::GetConfigFilePath()
{
    fs::path configPath = GetConfigDirectory() / "default.cfg";
    std::string pathStr = configPath.string();

    // Replace backslashes with forward slashes for consistency
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

    return pathStr;
}

bool ConfigReader::EnsureConfigDirectoryExists()
{
    fs::path configDir = GetConfigDirectory();

    TraceLog(LOG_INFO, "[ConfigReader] Checking directory: %s", configDir.string().c_str());

    if (fs::exists(configDir))
    {
        TraceLog(LOG_INFO, "[ConfigReader] Directory already exists");
        return true;
    }

    try
    {
        TraceLog(LOG_INFO, "[ConfigReader] Creating directory structure...");
        fs::create_directories(configDir);
        TraceLog(LOG_INFO, "[ConfigReader] Directory created successfully");
        return true;
    }
    catch (const fs::filesystem_error &e)
    {
        TraceLog(LOG_ERROR, "[ConfigReader] Failed to create directory: %s", e.what());
        return false;
    }
}

// ============================================================================
// PARSING UTILITIES
// ============================================================================

void ConfigReader::TrimString(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch)
                           { return !std::isspace(ch); })
                  .base(),
              str.end());
}

bool ConfigReader::ParseFloatArray(const std::string &value, std::vector<float> &outArray, int expectedSize)
{
    outArray.clear();
    std::stringstream ss(value);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        TrimString(token);
        if (token.empty())
            continue;

        try
        {
            outArray.push_back(std::stof(token));
        }
        catch (const std::exception &e)
        {
            TraceLog(LOG_ERROR, "[ConfigReader] ParseFloatArray failed on token '%s': %s",
                     token.c_str(), e.what());
            return false;
        }
    }

    if ((int)outArray.size() != expectedSize)
    {
        TraceLog(LOG_ERROR, "[ConfigReader] ParseFloatArray: Expected %d values, got %zu",
                 expectedSize, outArray.size());
        return false;
    }

    return true;
}

bool ConfigReader::ParseIntArray(const std::string &value, std::vector<int> &outArray, int expectedSize)
{
    outArray.clear();
    std::stringstream ss(value);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        TrimString(token);
        if (token.empty())
            continue;

        try
        {
            outArray.push_back(std::stoi(token));
        }
        catch (const std::exception &e)
        {
            TraceLog(LOG_ERROR, "[ConfigReader] ParseIntArray failed on token '%s': %s",
                     token.c_str(), e.what());
            return false;
        }
    }

    if ((int)outArray.size() != expectedSize)
    {
        TraceLog(LOG_ERROR, "[ConfigReader] ParseIntArray: Expected %d values, got %zu",
                 expectedSize, outArray.size());
        return false;
    }

    return true;
}

std::string ConfigReader::FloatArrayToString(const float *array, int size)
{
    std::stringstream ss;
    for (int i = 0; i < size; i++)
    {
        ss << array[i];
        if (i < size - 1)
            ss << ", ";
    }
    return ss.str();
}

std::string ConfigReader::IntArrayToString(const int *array, int size)
{
    std::stringstream ss;
    for (int i = 0; i < size; i++)
    {
        ss << array[i];
        if (i < size - 1)
            ss << ", ";
    }
    return ss.str();
}

// ============================================================================
// LOADING
// ============================================================================

bool ConfigReader::LoadFromFile(const std::string &filepath, _simulation_config &config)
{
    TraceLog(LOG_INFO, "");
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "[ConfigReader] LOAD CONFIG");
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "[ConfigReader] File path: %s", filepath.c_str());

    // Check if file exists
    if (!fs::exists(filepath))
    {
        TraceLog(LOG_ERROR, "[ConfigReader] File does not exist!");
        return false;
    }

    // Get file size
    std::uintmax_t fileSize = fs::file_size(filepath);
    TraceLog(LOG_INFO, "[ConfigReader] File size: %llu bytes", fileSize);

    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open())
    {
        TraceLog(LOG_ERROR, "[ConfigReader] Failed to open file for reading!");
        return false;
    }

    std::string line;
    std::string currentSection;
    int lineNumber = 0;
    int valuesLoaded = 0;

    TraceLog(LOG_INFO, "[ConfigReader] Beginning parse...");
    TraceLog(LOG_INFO, "");

    while (std::getline(file, line))
    {
        lineNumber++;
        TrimString(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        // Section headers [SECTION_NAME]
        if (line[0] == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.length() - 2);
            TrimString(currentSection);
            TraceLog(LOG_INFO, "[Line %d] Section: [%s]", lineNumber, currentSection.c_str());
            continue;
        }

        // Key-value pairs: KEY = VALUE
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos)
        {
            TraceLog(LOG_WARNING, "[Line %d] Skipping (no '=' found): %s", lineNumber, line.c_str());
            continue;
        }

        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);
        TrimString(key);
        TrimString(value);

        // Parse based on section
        try
        {
            if (currentSection == "PHYSICS")
            {
                if (key == "FRICTION")
                {
                    config._PHYSICS._FRICTION = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] FRICTION = %.3f", config._PHYSICS._FRICTION);
                    valuesLoaded++;
                }
                else if (key == "INTERACTION_RADIUS")
                {
                    config._PHYSICS._INTERACTION_RADIUS = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] INTERACTION_RADIUS = %.1f", config._PHYSICS._INTERACTION_RADIUS);
                    valuesLoaded++;
                }
                else if (key == "BETA")
                {
                    config._PHYSICS._BETA = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] BETA = %.3f", config._PHYSICS._BETA);
                    valuesLoaded++;
                }
                else if (key == "FORCE_SCALER")
                {
                    config._PHYSICS._FORCE_SCALER = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] FORCE_SCALER = %.1f", config._PHYSICS._FORCE_SCALER);
                    valuesLoaded++;
                }
                else if (key == "MIN_DISTANCE_SQUARED")
                {
                    config._PHYSICS._MIN_DISTANCE_SQUARED = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] MIN_DISTANCE_SQUARED = %.6f", config._PHYSICS._MIN_DISTANCE_SQUARED);
                    valuesLoaded++;
                }
            }
            else if (currentSection == "RENDER")
            {
                if (key == "PARTICLE_SIZE")
                {
                    config._RENDER._PARTCILE_SIZE = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] PARTICLE_SIZE = %.2f", config._RENDER._PARTCILE_SIZE);
                    valuesLoaded++;
                }
                else if (key == "PARTICLE_ALPHA")
                {
                    config._RENDER._PARTICLE_ALPHA = std::stof(value);
                    TraceLog(LOG_INFO, "  [✓] PARTICLE_ALPHA = %.2f", config._RENDER._PARTICLE_ALPHA);
                    valuesLoaded++;
                }
                else if (key == "BG_COLOR")
                {
                    std::vector<int> colors;
                    if (ParseIntArray(value, colors, 4))
                    {
                        config._RENDER._BG_COLOR.r = (unsigned char)colors[0];
                        config._RENDER._BG_COLOR.g = (unsigned char)colors[1];
                        config._RENDER._BG_COLOR.b = (unsigned char)colors[2];
                        config._RENDER._BG_COLOR.a = (unsigned char)colors[3];
                        TraceLog(LOG_INFO, "  [✓] BG_COLOR = (%d, %d, %d, %d)",
                                 colors[0], colors[1], colors[2], colors[3]);
                        valuesLoaded++;
                    }
                }
            }
            else if (currentSection == "ARENA")
            {
                if (key == "WIDTH")
                {
                    config._ARENA._WIDTH = std::stoi(value);
                    TraceLog(LOG_INFO, "  [✓] WIDTH = %d", config._ARENA._WIDTH);
                    valuesLoaded++;
                }
                else if (key == "HEIGHT")
                {
                    config._ARENA._HEIGHT = std::stoi(value);
                    TraceLog(LOG_INFO, "  [✓] HEIGHT = %d", config._ARENA._HEIGHT);
                    valuesLoaded++;
                }
                else if (key == "INITIAL_PARTICLE_COUNT")
                {
                    config._ARENA._INITIAL_PARTICLE_COUNT = std::stoi(value);
                    TraceLog(LOG_INFO, "  [✓] INITIAL_PARTICLE_COUNT = %d <<<< THIS IS THE KEY VALUE",
                             config._ARENA._INITIAL_PARTICLE_COUNT);
                    valuesLoaded++;
                }
                else if (key == "SPAWN_PER_CLICK")
                {
                    config._ARENA._SPAWN_PER_CLICK = std::stoi(value);
                    TraceLog(LOG_INFO, "  [✓] SPAWN_PER_CLICK = %d", config._ARENA._SPAWN_PER_CLICK);
                    valuesLoaded++;
                }
            }
            else if (currentSection == "INTERACTION_RULES")
            {
                if (key == "RULES")
                {
                    std::vector<float> rules;
                    int expectedCount = _NUM_PARTICLES_TYPES * _NUM_PARTICLES_TYPES;
                    if (ParseFloatArray(value, rules, expectedCount))
                    {
                        for (int i = 0; i < expectedCount; i++)
                        {
                            config._INTERACTION_RULES[i] = rules[i];
                        }
                        TraceLog(LOG_INFO, "  [✓] INTERACTION_RULES = %d values", expectedCount);
                        valuesLoaded++;
                    }
                }
            }
            else if (currentSection == "PARTICLE_COLORS")
            {
                if (key.substr(0, 6) == "COLOR_")
                {
                    int colorIndex = std::stoi(key.substr(6));
                    if (colorIndex >= 0 && colorIndex < _NUM_PARTICLES_TYPES)
                    {
                        std::vector<int> colors;
                        if (ParseIntArray(value, colors, 4))
                        {
                            config._RENDER._PARTICLE_COLORS[colorIndex].r = (unsigned char)colors[0];
                            config._RENDER._PARTICLE_COLORS[colorIndex].g = (unsigned char)colors[1];
                            config._RENDER._PARTICLE_COLORS[colorIndex].b = (unsigned char)colors[2];
                            config._RENDER._PARTICLE_COLORS[colorIndex].a = (unsigned char)colors[3];
                            TraceLog(LOG_INFO, "  [✓] COLOR_%d = (%d, %d, %d, %d)",
                                     colorIndex, colors[0], colors[1], colors[2], colors[3]);
                            valuesLoaded++;
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            TraceLog(LOG_ERROR, "[Line %d] Exception: %s", lineNumber, e.what());
            file.close();
            return false;
        }
    }

    file.close();

    TraceLog(LOG_INFO, "");
    TraceLog(LOG_INFO, "[ConfigReader] Parse complete: %d values loaded", valuesLoaded);
    TraceLog(LOG_INFO, "[ConfigReader] Final INITIAL_PARTICLE_COUNT = %d", config._ARENA._INITIAL_PARTICLE_COUNT);
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "");

    return valuesLoaded > 0;
}

// ============================================================================
// SAVING
// ============================================================================

bool ConfigReader::SaveToFile(const std::string &filepath, const _simulation_config &config)
{
    TraceLog(LOG_INFO, "");
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "[ConfigReader] SAVE CONFIG");
    TraceLog(LOG_INFO, "========================================");
    TraceLog(LOG_INFO, "[ConfigReader] File path: %s", filepath.c_str());
    TraceLog(LOG_INFO, "[ConfigReader] INITIAL_PARTICLE_COUNT to save = %d", config._ARENA._INITIAL_PARTICLE_COUNT);

    // Ensure directory exists
    fs::path filePath(filepath);
    fs::path directory = filePath.parent_path();

    if (!directory.empty() && !fs::exists(directory))
    {
        TraceLog(LOG_INFO, "[ConfigReader] Creating directory: %s", directory.string().c_str());
        try
        {
            fs::create_directories(directory);
        }
        catch (const fs::filesystem_error &e)
        {
            TraceLog(LOG_ERROR, "[ConfigReader] Failed to create directory: %s", e.what());
            return false;
        }
    }

    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        TraceLog(LOG_ERROR, "[ConfigReader] Failed to open file for writing!");
        return false;
    }

    file << "# PARSIM Configuration File\n";
    file << "# Auto-generated\n\n";

    file << "[PHYSICS]\n";
    file << "FRICTION = " << config._PHYSICS._FRICTION << "\n";
    file << "INTERACTION_RADIUS = " << config._PHYSICS._INTERACTION_RADIUS << "\n";
    file << "BETA = " << config._PHYSICS._BETA << "\n";
    file << "FORCE_SCALER = " << config._PHYSICS._FORCE_SCALER << "\n";
    file << "MIN_DISTANCE_SQUARED = " << config._PHYSICS._MIN_DISTANCE_SQUARED << "\n\n";

    file << "[RENDER]\n";
    file << "PARTICLE_SIZE = " << config._RENDER._PARTCILE_SIZE << "\n";
    file << "PARTICLE_ALPHA = " << config._RENDER._PARTICLE_ALPHA << "\n";

    int bgColor[4] = {
        (int)config._RENDER._BG_COLOR.r,
        (int)config._RENDER._BG_COLOR.g,
        (int)config._RENDER._BG_COLOR.b,
        (int)config._RENDER._BG_COLOR.a};
    file << "BG_COLOR = " << IntArrayToString(bgColor, 4) << "\n\n";

    file << "[ARENA]\n";
    file << "WIDTH = " << config._ARENA._WIDTH << "\n";
    file << "HEIGHT = " << config._ARENA._HEIGHT << "\n";
    file << "INITIAL_PARTICLE_COUNT = " << config._ARENA._INITIAL_PARTICLE_COUNT << "\n";
    file << "SPAWN_PER_CLICK = " << config._ARENA._SPAWN_PER_CLICK << "\n\n";

    file << "[INTERACTION_RULES]\n";
    file << "# 6x6 interaction matrix\n";
    file << "RULES = " << FloatArrayToString(config._INTERACTION_RULES.data(), _NUM_PARTICLES_TYPES * _NUM_PARTICLES_TYPES) << "\n\n";

    file << "[PARTICLE_COLORS]\n";
    file << "# RGBA colors (0-255)\n";
    for (int i = 0; i < _NUM_PARTICLES_TYPES; i++)
    {
        int colors[4] = {
            (int)config._RENDER._PARTICLE_COLORS[i].r,
            (int)config._RENDER._PARTICLE_COLORS[i].g,
            (int)config._RENDER._PARTICLE_COLORS[i].b,
            (int)config._RENDER._PARTICLE_COLORS[i].a};
        file << "COLOR_" << i << " = " << IntArrayToString(colors, 4) << "\n";
    }

    file.close();

    // Verify the file was written
    if (fs::exists(filepath))
    {
        std::uintmax_t fileSize = fs::file_size(filepath);
        TraceLog(LOG_INFO, "[ConfigReader] File written successfully (%llu bytes)", fileSize);
        TraceLog(LOG_INFO, "========================================");
        TraceLog(LOG_INFO, "");
        return true;
    }
    else
    {
        TraceLog(LOG_ERROR, "[ConfigReader] File was not created!");
        TraceLog(LOG_INFO, "========================================");
        TraceLog(LOG_INFO, "");
        return false;
    }
}