#include "../../headers/runtime/RuntimeHandler.h"

void RuntimeHandler::ExtractSegment () {
    //Allocate memory for segment, and set pointer.
    PanicUtils::SetImportant (&Segment::UnsafeAllocatedPointer, reinterpret_cast<DWORD> (VirtualAlloc (NULL, SegmentHeader::Datacase::ALLOCATION, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE)));
    //Small check for not nullable pointer. (In 99% of situations this is not necessary. But who knows what is on the player side. :d)
    //This is also unlikely, but if the player is not guilty of this, it will be useful for debugging.
    PanicUtils::RequireNonNull (PanicUtils::Layers::INIT, reinterpret_cast<DWORD> (memcpy (reinterpret_cast<PVOID> (Segment::GetSafeAllocationPointer()), SegmentData, SegmentHeader::Datacase::SIZE)), "Allocated memory pointer is null");
}

void RuntimeHandler::ReconstructHotPoints () {

    //------RELOCATIONS

    //Iterate relocations in vector.
    //(Why don't use SafeAllocationPointer? Because it's slow!)
    for (const auto& relocation : m_Segment.GetHeader().GetRelocations ()) {
        //Subtract value with old base address in segment from memory.
        *reinterpret_cast<DWORD*> (Segment::UnsafeAllocatedPointer + relocation) -= SegmentHeader::Datacase::RUNTIME;
        //Add new base address in segment to relocations.
        *reinterpret_cast<DWORD*> (Segment::UnsafeAllocatedPointer + relocation) += Segment::UnsafeAllocatedPointer;
    }

    //------RELOCATIONS

    //------IMPORTS

    //Iterate value in imports map.
    for (const auto& importsMap : m_Segment.GetHeader().GetImports ()) {

        //Get info about import from value at map.
        for (const auto& importInfo : importsMap.second) {

            //Get function pointer from module. (Support only function with name)
            DWORD functionPointer = Utils::GetFunction (importsMap.first, importInfo.function);

            //Check non-nullptr at function and module. (Why don't use RequireNonNull? Because it's slow!)
            if (!functionPointer) {
                //Just error. Message, value, etc.
                std::ostringstream message;
                message << "Can`t find module or function. " << "Module: " << importsMap.first << " <|> " << "Function: " << importInfo.function;
                PanicUtils::Release (PanicUtils::Layers::INIT, message.str().c_str());
            }

            //Just iterate all offset. (Locations where function call in segment)
            for (const auto& parsedOffset : importInfo.offsetsMap) {

                //More about ImportType: SegmentHeader.h
                switch (parsedOffset.first) {

                    case SegmentHeader::ImportType::INTERNAL:
                        //Patch functions (and in them) that call import.
                        *reinterpret_cast<DWORD*> (Segment::UnsafeAllocatedPointer + parsedOffset.second) = functionPointer - (Segment::UnsafeAllocatedPointer + parsedOffset.second + 0x4);
                      break;

                    case SegmentHeader::ImportType::PUBLIC:
                        //Patch global variables.
                        *reinterpret_cast<DWORD*> (Segment::UnsafeAllocatedPointer + parsedOffset.second) = functionPointer;
                      break;

                    default:
                        //Just error. Message, value, etc.
                        std::ostringstream message;
                        message << "Can't find offset type. Offset: " << parsedOffset.second << " <-> " << "Function: " << importInfo.function;
                        PanicUtils::Release (PanicUtils::Layers::ROUTINE, message.str().c_str());
                      break;
                }

            }

        }

    }

    //------IMPORTS

}

void RuntimeHandler::InvokeOEP () {
    //Set function address for call.
    SegmentHeader::DLLMAIN_CALLBACK DllMain = reinterpret_cast <SegmentHeader::DLLMAIN_CALLBACK> (Segment::GetSafeAllocationPointer() + SegmentHeader::Datacase::OEP);
    //Check for non-null OEP address.
    PanicUtils::RequireNonNull (PanicUtils::Layers::INIT, reinterpret_cast<DWORD> (DllMain), "OEP value is null");
    //Call "OEP" func with arguments.
    DllMain (reinterpret_cast<HMODULE> (Segment::GetSafeAllocationPointer()), DLL_PROCESS_ATTACH, NULL);
}