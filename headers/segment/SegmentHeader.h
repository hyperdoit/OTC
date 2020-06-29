#pragma once

#include "../../framework.h"

class SegmentHeader {

public:

    //Simple DllMain function. (Used for call OEP)
    //Docs: https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain
    typedef int (__stdcall* DLLMAIN_CALLBACK) (HMODULE hModule, DWORD callReason, LPVOID lpReserved);
    
    //Just data
    enum Datacase {
        //Original entry point. (https://en.wikipedia.org/wiki/Entry_point | https://www.aldeid.com/wiki/OEP-Original-Entry-Point)
        OEP = 0x4738C,
        //Previosly runtime value for fix relocations. (https://en.wikipedia.org/wiki/Relocation_(computing))
        RUNTIME = 0x3D600000,
        //Alloc size. (Param for new memory alloc size. Used for VirtualAlloc)
        ALLOCATION = 0x968940,
        //Segment size. (Param for copy segment to alloc mem. Used for memcpy)
        SIZE = 0x186A00
    };

    enum ImportType {

        /**
              What is intrnal ? Let's look at a simple function.

                 sub01:

                     int myImportSuperValue = (0xFFFFFFFF) (mySuperValue);

              This function receives the value from the import (some dll) and writes it to a variable. This is the Internal.
         **/

        INTERNAL,

        //What is public? This is a global variable that points to function import. I don’t even know what to add. That's all :d
        PUBLIC
    };

    struct ImportInfo {
        //Import function name.
        const char* function;
        //Location in segment where need patch. (RVA only)
        std::multimap <ImportType, DWORD> offsetsMap;
    };
    
    SegmentHeader () {
        FillImports ();
        FillRelocations ();
    }

    std::map <const char*, std::vector<ImportInfo>> GetImports() {
        return m_imports;
    }

    std::vector <DWORD> GetRelocations() {
        return m_relocations;
    }

private:

    //Need for segment can call the necessary functions from libraries. Storage model: Module -> ImportInfo [Function, offsets ^ n].
    std::map <const char*, std::vector<ImportInfo>> m_imports;
    //Need for reconstruct own variables/methods call.
    std::vector <DWORD> m_relocations;

    //Simple (no), functions for fill data-vectors.

    void FillImports ();
    void FillRelocations ();

};