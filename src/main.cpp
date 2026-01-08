#include "application/application.h"
#include "config/config.h"

int main() {
    _simulation_config config;

    Application app("PARSIM - Particle Life Simulation", config);
    app.Run();
    
    return 0;
}
