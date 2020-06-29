#include "../../framework.h"

HMODULE Utils::GetModule (const char* module) {

   HMODULE hModule = GetModuleHandleA (module);

   if (!hModule) {
       hModule = LoadLibraryA (module);
   }

   return hModule;
}

MODULEINFO Utils::GetModuleInfo (const char* module) {

    MODULEINFO modinfo = { 0 };

    GetModuleInformation (GetCurrentProcess (), GetModule(module), &modinfo, sizeof (MODULEINFO));

    return modinfo;
}

DWORD Utils::GetFunction (const char* module, const char* function) {
   return reinterpret_cast<DWORD> (GetProcAddress(GetModule(module), function));
}

template <typename T>
size_t split (__in const T& str, __in const T& delimiter, __out std::vector<T>& tokens) {

    tokens.clear ();

    size_t start = 0, end = str.find(delimiter), delimLength = delimiter.length();

    while (end != T::npos) {
        tokens.emplace_back (str.substr (start, end - start));
        start = end + delimLength;
        end = str.find (delimiter, start);
    }

    tokens.emplace_back (str.substr (start));
    return tokens.size ();

}

bool UnpackSignature (const char* packedSignature, std::vector <unsigned char>& pattern, std::string& mask) {

    std::vector <std::string> tokens;

    size_t count = split <std::string> (std::string (packedSignature), " ", tokens);
    if (!count) return false;

    pattern.reserve (count);
    mask.reserve (count);

    for (const auto& token : tokens) {

        if (token == "?") {
            pattern.push_back (0x00);
            mask += "?";
        }
        else {
            pattern.push_back (static_cast <unsigned char> (std::stoi (token, nullptr, 16)));
            mask += ".";
        }

    }

    return true;
}

DWORD SearchSignature (DWORD baseAddress, DWORD moduleSize, const char* pattern, const char* mask) {

    DWORD patternLength = strlen (mask);

    if (patternLength > moduleSize) return NULL;
    if (baseAddress == 0) return NULL;

    for (DWORD offset = 0; offset < baseAddress - patternLength; offset++) {

        bool isFound = true;

        for (DWORD charIterator = 0; charIterator < patternLength; charIterator++) {
            isFound &= mask [charIterator] == '?' || pattern [charIterator] == * reinterpret_cast<const char*> (baseAddress + offset + charIterator);
        }

        if (isFound) {
            return baseAddress + offset;
        }
    }

    return NULL;
}

void Utils::FindOffsetsToVec (const char* module, std::vector<const char*> signatures, std::vector<DWORD>& vector, bool isPanic) {

    MODULEINFO moduleInfo = GetModuleInfo (module);

    std::vector <unsigned char> pattern;
    std::string mask;

    for (const auto& signature : signatures) {

        if (!UnpackSignature (signature, pattern, mask)) {
            if (isPanic) {
                std::stringstream stream;
                stream << "Can't unpack signature. " << "Signature: " << signature;
                PanicUtils::Release (PanicUtils::Layers::ROUTINE, stream.str().c_str());
            }
        }

        DWORD offset = SearchSignature (reinterpret_cast<DWORD> (moduleInfo.lpBaseOfDll), moduleInfo.SizeOfImage, reinterpret_cast<const char*> (std::data(pattern)), mask.c_str());

        vector.emplace_back (offset);

        pattern.clear ();
        mask.clear ();
    }

}

void Utils::UpdateInstructionCharArgument (DWORD base, char compressedArray[], int offsetToInstructions, int instructionIteratorBase, int instructionCount, int iterationCycleUpdate, int instructionCharMoveCount, int arrayCharIteratorBase, int arrayCharCycleUpdate) {

    int segmentedCharIterator = arrayCharIteratorBase;

    for (int instructionsIterator = instructionIteratorBase; instructionsIterator < instructionCount; instructionsIterator += iterationCycleUpdate) {
        std::memcpy (reinterpret_cast <DWORD*> (base + offsetToInstructions + instructionsIterator), compressedArray + segmentedCharIterator, instructionCharMoveCount);
        segmentedCharIterator += arrayCharCycleUpdate;
    }

}

bool Utils::IsWinXPOrLater () {

    //Source: https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa.

    OSVERSIONINFO osVersionInfo;
    ZeroMemory (&osVersionInfo, sizeof (OSVERSIONINFO));

    //This causes a little confused. 
    //
    //If we were unable to fill the structure, then we return true, because the function expects false, 
    //this will lead to the fact that the initialization of the loader simply stops, and through the debugger you can see what is wrong. 
    if (GetVersionEx (&osVersionInfo)) return true;

    return ((osVersionInfo.dwMajorVersion > 5) || ((osVersionInfo.dwMajorVersion == 5) && (osVersionInfo.dwMinorVersion >= 1)));

}

bool Utils::IsWinServBuild () {

    OSVERSIONINFOEX osVersionInfo;
    ZeroMemory (&osVersionInfo, sizeof(OSVERSIONINFO));

    //This causes a little confused. 
    //
    //If we were unable to fill the structure, then we return true, because the function expects false, 
    //this will lead to the fact that the initialization of the loader simply stops, and through the debugger you can see what is wrong.
    if (GetVersionEx ((LPOSVERSIONINFO) &osVersionInfo)) return true;

    return osVersionInfo.wProductType == VER_NT_WORKSTATION;

}

bool Utils::IsRedistPackagePresent () {

    //All packages keys x86.
    //Source: https://stackoverflow.com/questions/12206314/detect-if-visual-c-redistributable-for-visual-studio-2012-is-installed.

    std::map <HKEY, std::wstring> regSubKeys = {
        //Visual C++ 2010.
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\Installer\\Products\\67D6ECF5CD5FBA732B8B22BAC8DE1B4D" },
        //Visual C++ 2012.
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\Installer\\Dependencies\\{33d1fd90-4274-48a1-9bc1-97e33d9c2d6f}" },
        //Visual C++ 2013.
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\Installer\\Dependencies\\{f65db027-aff3-4070-886a-0d87064aabb1}" },
        //Visual C++ 2015.
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\Installer\\Dependencies\\{e2803110-78b3-4664-a479-3611a381656a}" },
        //Visual C++ 2017.
        { HKEY_CLASSES_ROOT, L"Installer\\Dependencies\\VC,redist.x86,x86,14.16,bundle\\Dependents\\{67f67547-9693-4937-aa13-56e296bd40f6}" },
        //Visual C++ 2019. (Pack)
        { HKEY_CLASSES_ROOT, L"Installer\\Dependencies\\VC,redist.x86,x86,14.21,bundle\\Dependents\\{49697869-be8e-427d-81a0-c334d1d14950}" },
        { HKEY_CLASSES_ROOT, L"Installer\\Dependencies\\VC,redist.x86,x86,14.22,bundle\\Dependents\\{5bfc1380-fd35-4b85-9715-7351535d077e}" },
    };
   
    for (const auto& key : regSubKeys) {
        if (RegOpenKeyExW (key.first, key.second.c_str(), 0, KEY_READ, nullptr)) {
            RegCloseKey (key.first);
            return true;
        }
    }

    return false;

}