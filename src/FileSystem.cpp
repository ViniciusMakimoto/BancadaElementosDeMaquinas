#include "FileSystem.h"

bool FileSystem::begin()
{
#ifdef WOKWI_EMU
    DEBUG_PRINTLN("[FS] Modo Simulação (Wokwi): Usando Arrays embutidos.");
    return true;
#else
    DEBUG_PRINTLN("[FS] Modo Hardware Real: Montando LittleFS...");
    if (!LittleFS.begin(true))
    { // true = formata se falhar
        DEBUG_PRINTLN("[FS] Erro ao montar LittleFS");
        return false;
    }
    DEBUG_PRINTLN("[FS] LittleFS montado com sucesso.");
    return true;
#endif
}