#include "../../framework.h"

//RUNTIME.

void SegmentFramework::CreateDependencyTable () {

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // We use this as a temporary storage. (The compiler will replace the vector with an array)                                                                  //
    //                                                                                                                                                           //
       std::vector <DWORD> info;                                                                                                                                 //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                                                           //  
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Some function in steam client. idk.                                                                                                                       //
    //                                                                                                                                                           //
       info.emplace_back (reinterpret_cast<DWORD> (Utils::GetModule("steamclient.dll") + Datacase::LIBRARY_RVA));                                                //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Base addresses of game modules for stable operation of the onetap.                                                                                        //
    //                                                                                                                                                           //
       for (const auto& library : m_libraries) info.emplace_back (reinterpret_cast<DWORD> (Utils::GetModule(library)));                                          //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // We scan signatures in real time, and move what we got (offsets) to the table.                                                                             //
    // Offsets are needed so that OneTap internal functions can find functions from outside. (For example, the render of the menu depends on the same offset)    //
    // p.s Index 0 - it's client dll.                                                                                                                            //
    //                                                                                                                                                           //
       Utils::FindOffsetsToVec (m_libraries.at(0), m_signatures, info, true);                                                                                    //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // And so, consider everything in order. Here we move the finished table to the allocated segment memory.                                                    //
    // Onetap has a function that automatically takes information from the allocated memory because it knows what will be there: "Libraries, Offsets."           //
    // Since the original loader loads there as we do.                                                                                                           //
    //                                                                                                                                                           //
    // What is the number 0x20 and why is it here? The function starts reading data (base addresses, offsets) starting from: Base address + offset by 0x20 bytes //
    // And since we only have data that the function reads in the table, we can safely move the data directly to the place where the function reads.             //
    //                                                                                                                                                           //
        std::memmove (reinterpret_cast<DWORD*> (Segment::GetSafeAllocationPointer () + 0x20), reinterpret_cast<PVOID> (info.data()), Datacase::TABLE_SIZE);      //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

void SegmentFramework::UpdateNetVars () {

    //Netvars are offsets to parent variables in valve sdk.
    for (const auto& netvar : m_netvars) {
        *reinterpret_cast<DWORD*> (Segment::GetSafeAllocationPointer () + netvar.rva) = netvar.new_value;
    }

}


void SegmentFramework::CreateHook () {

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // We create a hook to prevent a crash due to incorrect indexes, but first, let's look at everything in order.                                                                       //
    //                                                                                                                                                                                   //
    // How does a hook work at all? The function is called -> The call is redirected to our function -> Next, we can do whatever we want.                                                //
    //                                                                                                                                                                                   //
    // Why do we need a hook here?                                                                                                                                                       //
    //                                                                                                                                                                                   //
    // OneTap uses virtual functions for internal purposes (find out what kind of weapon the player currently has, find out which object holds the weapon)                               //                                                                                                                                                                          //
    // Virtual functions are called using an index table.                                                                                                                                //
    //                                                                                                                                                                                   //
    // Since the game is constantly being updated, the indexes are shifted, and therefore, every time we call a function on the wrong index, a crash occurs.                             //
    // In order not to patch a binary file every time, creating a hook is the simplest solution.                                                                                         //
    // (And it gets even easier with HookLib. More details: https://www.github.com/HoShiMin/HookLib)                                                                                     //                                                                                         
    //                                                                                                                                                                                   //
      SetHook (reinterpret_cast<PVOID> (Segment::GetSafeAllocationPointer() + Datacase::VIRTUAL_EXECUTOR_RVA), &CustomVirtualCaller, reinterpret_cast<PVOID*> (&OriginalVirtualCaller)); //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

UINT SegmentFramework::CustomVirtualCaller (PVOID vTable, INT index) {

    //Last index update you can found on UC. (https://www.unknowncheats.me/forum/counterstrike-global-offensive/310246-updates-megathread.html)

    if (index >= 152) {

        index += 2;

        if (index >= 256) index++;
        if (index >= 300) index += 2;
        if (index >= 300) index++;

    }

    return SegmentFramework::OriginalVirtualCaller(vTable, index);

}

//VISUALS.

void SegmentFramework::UpdateWatermark (const char* mark, const char* player) {

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create new char array for update instructions.                                                                                                            //
    //                                                                                                                                                           //
    // Why 16? Because in watermark only 4 mov instructions with 4 bytes in argument. 4 * 4 = 16.                                                                //
    //                                                                                                                                                           //
       char watermark [16];                                                                                                                                      //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Copy watermark name to compressed array with custom size.                                                                                                 //
    //                                                                                                                                                           //
       std::strncpy (watermark, mark, 16);                                                                                                                       //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // All arguments docs inside func.                                                                                                                           //
    //                                                                                                                                                           //
       Utils::UpdateInstructionCharArgument (Segment::GetSafeAllocationPointer(), watermark, Datacase::BOX_WATERMARK_RVA,  0x0, 0x1C, 0x7, 0x4, 0x0, 0x4);       //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // We move the username to the start of the segment since the function reads the username first, then the libraries, then the offsets.                       //
    //                                                                                                                                                           //
    // What is the offset 0x1 ? If the internal OneTap function reads the first byte and it turns out to be non - zero - crash.                                  //
    //                                                                                                                                                           //
    // What is the value 0x20 ? This is the maximum nickname length. 0x20 translated from hex to dec - This is 32 characters.                                    //
    //                                                                                                                                                           //
       std::memmove (reinterpret_cast<DWORD*> (Segment::GetSafeAllocationPointer() + 0x1), player, 0x20);                                                        //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
}

void SegmentFramework::UpdateMenuWatermark (const char* value) {

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create new char array for update instructions.                                                                                                            //
    //                                                                                                                                                           //
    // Why 12? Because in watermark only 3 mov instructions with 4 bytes in argument. 4 * 3 = 12.                                                                //
    //                                                                                                                                                           //
       char watermark [12];                                                                                                                                      //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Copy watermark name to compressed array with custom size.                                                                                                 //
    //                                                                                                                                                           //
       std::strncpy (watermark, value, 12);                                                                                                                      //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // All arguments info inside func.                                                                                                                           //
    //                                                                                                                                                           //
       Utils::UpdateInstructionCharArgument (Segment::GetSafeAllocationPointer(), watermark, Datacase::MENU_WATERMARK_RVA, 0x0, 0x15, 0x7, 0x4, 0x0, 0x4);       //
    //                                                                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

void SegmentFramework::SetMenuStatus (bool status) {
    //P.S You can write your own function to open the menu using your own key.
    *reinterpret_cast<DWORD*> (Segment::GetSafeAllocationPointer() + Datacase::MENU_STATUS_RVA) = status;
}