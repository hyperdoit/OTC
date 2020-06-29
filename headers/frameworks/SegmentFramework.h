#pragma once

#include "../../framework.h"
#include "../runtime/Logger.h"

class SegmentFramework {

public:

   typedef UINT (__fastcall* oVirtualFunctionCaller) (PVOID vTable, INT index);

   //A variable that indicates the original function from the hook.
   static oVirtualFunctionCaller OriginalVirtualCaller;

   //RUNTIME.

   /**
    * Used to initialize internal values ​​in segment. (About values: SegmentUtils.cpp)
    *
    * @param nick - Player nickname in watermark (!32 character limit!)
   **/

   void CreateDependencyTable ();

   /**
    * Netvars updater. (More about netvars can be found on unknowncheats)
    **/

   void UpdateNetVars ();

   /**
    * Intercept legacy functions from the segment and replace it with your own. (About hook/lib: https://www.github.com/HoShiMin/HookLib)
    **/

   void CreateHook ();

   /**
    * Used to repair indexes.
    **/

   static UINT __fastcall CustomVirtualCaller(PVOID vTable, INT index);

   //VISUALS.

   /**
    * Update segment name and user name in internal render watermark.
    *
    * @param water - New watermarks name. (Limit - 16 symbols)
    * @param playerName - New player name. (Limit - 32 symbols)
    **/

   void UpdateWatermark (const char* water, const char* playerName);

   /**
    * Update segment name in internal menu.
    *
    * @param valye - New watermark name. (Limit - 12 symbols)
    **/

   void UpdateMenuWatermark (const char* value);

   /**
    * Show/hide internal segment menu.
    *
    * @param status - true = show/false = hide.
    **/

   void SetMenuStatus (bool status);

protected:

    //Netvars are offsets to parent variables in valve sdk.
    struct RelocatedNetVar {
        //Offset to variable. (Relocation - Old Base Address)
        int rva;
        //Set value.
        int new_value;
    };
    
    //Just data.
    enum Datacase {

        //Unknown offset. Used for create info table.
        LIBRARY_RVA = 0x2A6900,
        //Info-table in memory size. (Value taken from segment-side function)
        TABLE_SIZE = 0x18C,

        //Offset to hook function in memory.
        VIRTUAL_EXECUTOR_RVA = 0x3B30,

        //Offset to first argument in segment mov instruction.
        BOX_WATERMARK_RVA = 0x12D381,
        //Offset to first argument in segment mov instruction.
        MENU_WATERMARK_RVA = 0xE025F,

        //Offset to boolean for show/hide internal menu segment.
        MENU_STATUS_RVA = 0x485F67,

    };

    //Used for fix segment.
    std::vector <RelocatedNetVar> m_netvars = {
        //m_bIsScoped. (Actual value: https://github.com/frk1/hazedumper/blob/master/csgo.cs#L32)
        RelocatedNetVar { 0x95A250, 0x3928 }
    };

    //Used for dependency info table.
    std::vector <const char*> m_libraries = {
       "client.dll",
       "vguimatsurface.dll",
       "shaderapidx9.dll",
       "vstdlib.dll"
    },

    //Used for find offsets.
    m_signatures = {
		//MD5_PseudoRandom					ResetAnimationState			SetAbsOrigin
       "55 8B EC 83 E4 F8 83 EC 70 6A 58", "56 6A 01 68 ? ? ? ? 8B F1", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8 ? ? ? ? 8B 7D",
	   //SetAbsAngles                               IsBreakableEntity? not sure		GetSequenceActivity
       "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1", "51 56 8B F1 85 F6 74 68 83", "55 8B EC 53 8B 5D 08 56 8B F1 83",
	   //CPrediction::RunCommand					IMoveHelper					//Not sure (undocumented)
       "55 8B EC 83 E4 C0 83 EC 34 53 56 8B 75", "8B 0D ? ? ? ? 8B 46 08 68", "55 8B EC 56 8B F1 51 8D",
	   //DoExtraBoneProcessing                                        StandardBlendingRules
       "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C", "55 8B EC FF 75 18 F3 0F 10 45 ? 51",
	   //UpdateClientSideAnimations               //Not sure (undocumented)
       "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74", "8B 35 ? ? ? ? FF 90 ? ? ? ? 50 B9 ? ? ? ? FF 56 24 5E C3",
	   //VIEW_RENDER_BEAMS                          Not sure (undocumented)                Not sure
       "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9", "0F 44 C8 33 C0 5E 39 11 0F 94 C0 C3", "B0 01 83 FE",
	   //Not sure (undocumented)  Not sure (undocumented)                     IsReady
       "B9 ? ? ? ? FF 50 14 5F", "81 C6 ? ? ? ? 8B 4E 0C 8B 56 04 89 4D E4", "55 8B EC 51 56 8B 35 ? ? ? ? 57 83",
	   //RankRevealAll                                IS_LINE_GOES_THROUGH_SMOKE           Not sure (undocumented)
       "C7 00 ? ? ? ? E8 ? ? ? ? 83 EC 08 8D 4E 74", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F", "80 B9 ? ? ? ? ? 75 6C",
	   //Not sure (undocumented)          //Not sure          FIND_HUD
       "83 3D ? ? ? ? ? 57 8B F9 75 06", "A1 ? ? ? ? 74 38", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39",
	   //Not sure                                       //Not sure                            clearNotices
       "55 8B EC 56 8B 75 0C 57 8B F9 B9 ? ? ? ? 89", "B9 ? ? ? ? A3 ? ? ? ? E8 ? ? ? ? 5D", "55 8B EC 83 EC 0C 53 56 8B 71",
	   //Not sure                                                 ClearHudWeaponIcon
       "B9 ? ? ? ? E8 ? ? ? ? 85 C0 74 0A 8B 10 8B C8 FF A2 AC", "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B",
	   //Not sure                                                                                                            //MaintainSequenceTransitions return address
       "0F 84 ? ? ? ? 8B 88 ? ? ? ? 83 F9 FF 0F 84 ? ? ? ? 0F B7 C1 C1 E0 04 05 ? ? ? ? C1 E9 10 39 48 04 0F 85 ? ? ? ? 8B", "84 C0 74 17 8B 87",
	   //Not sure                                Not sure               Not sure
       "84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05", "8B 97 ? ? ? ? 56 8B",  "55 8B EC 83 E4 F0 83 EC 7C 56 FF",
	   //Not sure                     Not sure
       "55 8B EC 8D 51 08 8B 4D 08", "55 8B EC 56 8B 35 ? ? ? ? 85 F6 0F 84 ? ? ? ? 81",
	   //Not sure                                CalcViewModelView        Not sure
       "8B 01 FF B1 ? ? ? ? FF 90 ? ? ? ? 83", "55 8B EC 83 EC 58 56 57", "53 56 8B F1 57 8B 4E 3C",
	   //Not sure                          //AbsVelocity i think?   
       "55 8B EC 56 8B 75 08 57 FF 75 18", "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3",
	   //:invalidate_physics_recursive                  Not sure
       "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7",
	   //Not sure                       Not sure
       "F3 0F 10 A6 ? ? ? ? F3 0F 11", "56 8B F1 8B 0D ? ? ? ? 57 8B 01 FF 76 70",
	   //Not sure                            orig_anim_frame
       "55 8B EC 83 EC 10 A1 ? ? ? ? 89 4D", "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02",
	   //Not sure - not documented anywhere                SetUpVelocity
       "55 8B EC 83 E4 F8 83 EC 5C 53 8B D9 56 57 83 7B", "55 8B EC 83 E4 F8 83 EC 30 56 57 8B 3D",
	   //Not sure                                               Cam_THINK
       "55 8B EC 56 FF 75 08 8B F1 8B 06 FF 90 ? ? ? ? 8B 86", "85 C0 75 30 38 86",
	   //Not sure             Not sure             Some return addr   return_to_accumulate_layers?
       "C7 46 ? ? ? ? ? 43",  "85 C0 74 2D 83 7D", "75 04 B0 01 5F", "84 C0 75 0D F6 87",
	   //SetupVelocity                           No idea
       "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80", "84 C0 75 09 5F 5E 5B 8B E5 5D C2 14",
	   //Not sure                                    Not sure                    Not sure              InitializeKits
       "55 8B EC 81 EC ? ? ? ? 53 56 57 8B D9 E8", "F3 0F 10 4D ? 84 C0 74 12", "FF D0 A1 ? ? ? ? B9", "E8 ? ? ? ? FF 76 0C 8D",
	   //InvalidateBoneCache                         Not sure
       "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81", "8B 35 ? ? ? ? FF 10 0F B7 C0 B9 ? ? ? ? 50 FF 56 08 85",
	   //CustomMaterials         oConstructor?                     Not sure
       "83 BE ? ? ? ? ? 7F 67", "E8 ? ? ? ? EB 02 33 C0 57 8B BE", "E8 ? ? ? ? 8B D0 85 D2 75 07 32 C0 5F",
	   //Not sure                                           GlowObjectMgr   Interpolation List?                  Not sure       UpdateVisibility(AllEntities)
       "E8 ? ? ? ? 8B 46 04 5F 5E 5B 8B E5 5D C2 08 00 6A", "75 4B 0F 57", "0F B7 05 ? ? ? ? 3D ? ? ? ? 74 3F", "56 8D 51 3C", "E8 ? ? ? ? 83 7D D8 00 7C 0F",
	   //Not sure                                                 Not sure            Not sure
       "55 8B EC 83 E4 F8 81 EC ? ? ? ? A1 ? ? ? ? 53 56 8B D9", "E8 ? ? ? ? 99 2B", "E8 ? ? ? ? 8B 0D ? ? ? ? 0F 57 C9 99",
	   //Not sure      IS_LINE_GOES_THROUGH_SMOKE (again??) Not sure            Not sure
       "8D 4E 74 8B", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F", "E8 ? ? ? ? 99 2B", "E8 ? ? ? ? 8B 0D ? ? ? ? 0F 57 C9 99",
	   //Not sure                          Not sure                         CreateConVar Fn
       "8B 0D ? ? ? ? F6 87 ? ? ? ? ? 8B", "55 8B EC F3 0F 10 45 ? 56 6A", "55 8B EC 83 E4 F8 8B 45 0C 83 EC 10",
	   //InsertIntoTreeCallListLeavesInBox? Not sure     WriteUserCmd                        Not sure
       "FF 50 18 89 44 24 14 EB", "A2 ? ? ? ? 8B 45 E8", "55 8B EC 83 E4 F8 51 53 56 8B D9", "55 8B EC 83 EC 48 53 8B 5D"
    };

};