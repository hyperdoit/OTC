#include "../../framework.h"

void PanicUtils::SetImportant (DWORD *variable, DWORD value, const char* reason) {
    if (variable && value) {
        *variable = value;
    }
    else {
        PanicUtils::Release (Layers::ROUTINE, reason);
    }
}

void PanicUtils::RequireNonNull (Layers layer, DWORD value, const char* reason) {
    if (!value) {
        PanicUtils::Release (layer, reason);
    }
}

void PanicUtils::Release (Layers layer, const char* reason) {

    // Changing to CONSOLE is strictly not recommended.
    // It is possible that the message simply does not have time to be displayed.
    Logger log(Logger::LoggerType::FILE);
    std::stringstream stream;

    stream  << "\n"
    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PANIC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" 
    << "\n" << "\n"
    << "If you see this message: OTC cannot find a way out of the situation." 
    << "\n"
    << "After information about the cause of the error, CSGO will be frozen."
    << "\n" << "\n"
    << "----------- INFO ----------" << "\n"
    << "Layer: " << layer << "." << "\n"
    << "Caused: " << reason << "." << "\n"
    << "----------------------------" << "\n"
    << "Global variables: " << "\n"
    << "-----------------------------------------------------------" << "\n"
    << "[SEGMENT] Module: " << "0x" << std::uppercase << std::hex << Segment::UnsafeLibraryPointer << "\n"
    << "[SEGMENT] Allocation: " << "0x" << std::uppercase << std::hex << Segment::UnsafeAllocatedPointer << "\n"
    << "-----------------------------------------------------------"
    << "\n" << "\n"
    << "Send this log to the developer or try connecting a debugger and find out what the problem is." << "\n"
    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PANIC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << "\n"
    ;

    log.SwitchedInfo ("_Panic");
    log.SwitchedInfo ("If you see this message: OTC cannot find a way out of the situation.");
    log.SwitchedInfo ("Log may at document folder.");
    log.Info (stream.str().c_str());

    if (!Segment::UnsafeLibraryPointer) {
        //Debug to process and watch exit code for get solution!
        ExitProcess (0xFFFA1);
    }

    if (!Segment::UnsafeAllocatedPointer) {
        FreeLibraryAndExitThread (reinterpret_cast<HMODULE> (Segment::UnsafeLibraryPointer), NULL);
    }

    VirtualFree (reinterpret_cast<LPVOID> (Segment::UnsafeAllocatedPointer), NULL, MEM_RELEASE);
    //Froze for read log.
    ExitThread (0);

}