#include "AEConfig.h"
#include "AE_EffectVers.h"

resource 'PiPL' (16000) {
    {
        Kind { AEEffect },
        Name { "MO Effector Native" },
        Category { "MO Tools" },
#ifdef AE_OS_WIN
    #if defined(AE_PROC_INTELx64)
        CodeWin64X86 { "EffectMain" },
    #elif defined(AE_PROC_ARM64)
        CodeWinARM64 { "EffectMain" },
    #endif
#endif
        AE_PiPL_Version { 2, 0 },
        AE_Effect_Spec_Version { PF_PLUG_IN_VERSION, PF_PLUG_IN_SUBVERS },
        AE_Effect_Version { 524289 },
        AE_Effect_Info_Flags { 0 },
        AE_Effect_Global_OutFlags { 33588288 },
        AE_Effect_Global_OutFlags_2 { 0x8001400 },
        AE_Effect_Match_Name { "com.brunojorri.MOEffectorNative" },
        AE_Reserved_Info { 0 },
        AE_Effect_Support_URL { "https://www.instagram.com/brunojorri_work/" }
    }
};
