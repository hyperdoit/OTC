#pragma once

#include "../../framework.h"

class GameFramework {

protected:

    //Legacy.

    #define GAME_VIRTUAL_CONSOLE_INDEX 108 * 4
    #define GAME_VIRTUAL_CLIENT_CMD_INDEX 114

    //More about this stuff: https://www.unknowncheats.me/forum/counterstrike-global-offensive/190552-internal-printing-console-explanation.html
    typedef PVOID (*oCreateInterfaceEngine)             (const char* interfaceName, PINT code);
    typedef BOOL (__thiscall *oConsoleInterface)        (const void* caller, const char* command);
    typedef VOID (*oPrintConsoleInterface)              (const char* message, ...);

    oPrintConsoleInterface m_PrintInterface =  (oPrintConsoleInterface) Utils::GetFunction ("tier0.dll", "Msg");
    oCreateInterfaceEngine m_CreateInterface = (oCreateInterfaceEngine) Utils::GetFunction ("engine.dll", "CreateInterface");
    PVOID m_VClientPointer = m_CreateInterface ("VEngineClient014", nullptr);
    oConsoleInterface m_ConsoleInterface =     (oConsoleInterface) * reinterpret_cast<int*> (*reinterpret_cast<int*> (m_VClientPointer) + GAME_VIRTUAL_CONSOLE_INDEX);

public:

    //Check non-null interfaces.
    BOOL m_IsInterfacesAvailable = m_PrintInterface && m_CreateInterface && m_VClientPointer && m_ConsoleInterface;

    /**
     * Simple send new line to console.
     *
     * @param newLineCount - Lines count.
     **/

    void PrintNewLineToConsole (int newLineCount = 1);

    /**
     * Print message to game console.
     **/

    oPrintConsoleInterface PrintToConsole ();
    /**
     * Execute command at console.
     **/

    oConsoleInterface AtConsoleInterface () {
        return m_ConsoleInterface;
    }

    /**
     * Legacy method for create interfaces.
     **/

    oCreateInterfaceEngine AtCreateInterfaceEngine () {
        return m_CreateInterface;
    }

};