#include <psre/log.hpp>
#include <psre/serial.h>

namespace psre::log {
    void write(const char* message) {
        serial_write("[PSRE] ");
        serial_write(message);
        serial_write("\n");
    }
}

extern "C" void psre_subsystem_m001_init(void) {
    psre::log::write("C++ subsystem initialized (freestanding mode)");
}
