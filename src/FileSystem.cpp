#include "FileSystem.h"

bool FileSystem::begin()
{
#ifdef WOKWI_EMU
    Serial.println("[FS] Modo Simulação (Wokwi): Usando Arrays embutidos.");
    return true;
#else
    Serial.println("[FS] Modo Hardware Real: Montando LittleFS...");
    if (!LittleFS.begin(true))
    { // true = formata se falhar
        Serial.println("[FS] Erro ao montar LittleFS");
        return false;
    }
    Serial.println("[FS] LittleFS montado com sucesso.");
    return true;
#endif
}