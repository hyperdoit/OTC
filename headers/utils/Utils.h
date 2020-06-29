#pragma once

#include "../../framework.h"
//Hotfix for GetVersionExW.
#pragma warning (disable : 4996)

class Utils {

public:

    /**
     * Get module base address from internal process.
     *
     * @param module - Module name.
     **/

    static HMODULE GetModule (const char* module);

    /**
     * Get module info from internal process.
     * 
     * @param module - Module name.
     **/

    static MODULEINFO GetModuleInfo (const char* module);

    /**
     * Get function address from module.
     *
     * @param module - Module name.
     * @param function - Function name.
     **/

    static DWORD GetFunction (const char* module, const char* function);

    /**
     * Find offsets and fill to vector.
     *
     * @param module - Module name.
     * @param signatures - Signatures vector.
     * @param vector - Fill vector.
     * @param isPanic - Call panic when offset not found.
     **/

    static void FindOffsetsToVec (const char* module, std::vector<const char*> signatures, std::vector<DWORD>& vector, bool isPanic = false);

    /**
     * Update instruction argument with char array.
     * 
     * @param base - Module base address.
     * @param compressedArray - Char array with limited length.
     * @param offsetToInstructions - Instruct offset. (Base address + offsetToInstruct = InstructionArgumentStartPos)
     * @param instructionIteratorBase - Iterator offset base. (Base address + offsetToInstruct + instructionIterator = Next instruction)
     * @param instructionCount - Next instruction count. (InstructionSize * Count = InstructionCount)
     * @param iterationCycleUpdate - Iterator sum update. (Iteration += InstructionSize)
     * @param instructionCharMoveCount - Count symbols move from array to instruction argument. (Example: For mov instruction need 0x4 bytes)
     * @param arrayCharIteratorBase - Start array pos.
     * @param arrayCharCycleUpdate - Iterator sum update.
     **/

    static void UpdateInstructionCharArgument (DWORD base, char compressedArray[], int offsetToInstructions, int instructionIteratorBase, int instructionCount, int iterationCycleUpdate, int instructionCharMoveCount, int arrayCharIteratorBase, int arrayCharCycleUpdate);

    /**
     * Simple check on OS version to meet the minimum requirements.
     **/

    static bool IsWinXPOrLater ();

    /**
     * Simple check on OS type to meet the minimum requirements.
     **/

    static bool IsWinServBuild ();

    /**
     * Simple check on installed redist to meet the minimum requirements.
     **/

    static bool IsRedistPackagePresent ();

};