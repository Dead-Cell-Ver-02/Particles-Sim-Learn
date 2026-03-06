#include "application/application.h"
#include "config/config.h"
#include "config/ConfigReader.h"

int main()
{
    // Enable all logging for debugging
    SetTraceLogLevel(LOG_ALL);

    TraceLog(LOG_INFO, "");
    TraceLog(LOG_INFO, "##########################################");
    TraceLog(LOG_INFO, "#   PARSIM - PARTICLE LIFE SIMULATION   #");
    TraceLog(LOG_INFO, "##########################################");
    TraceLog(LOG_INFO, "");

    // Get the config file path
    std::string configPath = ConfigReader::GetConfigFilePath();
    TraceLog(LOG_INFO, "[Main] Config file path: %s", configPath.c_str());

    // Ensure config directory exists
    if (!ConfigReader::EnsureConfigDirectoryExists())
    {
        TraceLog(LOG_ERROR, "[Main] Failed to create config directory!");
        TraceLog(LOG_ERROR, "[Main] Continuing with default config...");
    }

    // Create config with hardcoded defaults
    _simulation_config config;
    TraceLog(LOG_INFO, "[Main] Default INITIAL_PARTICLE_COUNT = %d", config._ARENA._INITIAL_PARTICLE_COUNT);

    // Try to load existing config
    if (ConfigReader::LoadFromFile(configPath, config))
    {
        TraceLog(LOG_INFO, "[Main] ✓ Config loaded successfully!");
        TraceLog(LOG_INFO, "[Main] Using INITIAL_PARTICLE_COUNT = %d", config._ARENA._INITIAL_PARTICLE_COUNT);
    }
    else
    {
        TraceLog(LOG_WARNING, "[Main] Config file not found or failed to load");
        TraceLog(LOG_INFO, "[Main] Creating default config file...");

        if (ConfigReader::SaveToFile(configPath, config))
        {
            TraceLog(LOG_INFO, "[Main] ✓ Default config file created");
        }
        else
        {
            TraceLog(LOG_ERROR, "[Main] ✗ Failed to create default config file");
        }

        TraceLog(LOG_INFO, "[Main] Using hardcoded defaults: INITIAL_PARTICLE_COUNT = %d",
                 config._ARENA._INITIAL_PARTICLE_COUNT);
    }

    TraceLog(LOG_INFO, "");
    TraceLog(LOG_INFO, "[Main] Creating Application...");
    TraceLog(LOG_INFO, "[Main] Config being passed: INITIAL_PARTICLE_COUNT = %d", config._ARENA._INITIAL_PARTICLE_COUNT);

    Application app("PARSIM - Particle Life Simulation", config);

    TraceLog(LOG_INFO, "[Main] Starting main loop...");
    TraceLog(LOG_INFO, "");

    app.Run();

    TraceLog(LOG_INFO, "[Main] Application closed normally");
    return 0;
}