//
// Created by viski on 30.04.2025.
//

#ifndef X64_CSTACKTRACE_H
#define X64_CSTACKTRACE_H

#include <cstdio>
#include <dlfcn.h>
#include <dlfcn.h>
#include <unwind.h>
#include "../main.h"

#if VER_x32
#define PRINT_CRASH_STATES(context) \
	Log(OBFUSCATE("register states:")); \
	Log(OBFUSCATE("r0: 0x%X, r1: 0x%X, r2: 0x%X, r3: 0x%X"), (context)->uc_mcontext.arm_r0, (context)->uc_mcontext.arm_r1, (context)->uc_mcontext.arm_r2, (context)->uc_mcontext.arm_r3); \
	Log(OBFUSCATE("r4: 0x%x, r5: 0x%x, r6: 0x%x, r7: 0x%x"), (context)->uc_mcontext.arm_r4, (context)->uc_mcontext.arm_r5, (context)->uc_mcontext.arm_r6, (context)->uc_mcontext.arm_r7); \
	Log(OBFUSCATE("r8: 0x%x, r9: 0x%x, sl: 0x%x, fp: 0x%x"), (context)->uc_mcontext.arm_r8, (context)->uc_mcontext.arm_r9, (context)->uc_mcontext.arm_r10, (context)->uc_mcontext.arm_fp); \
	Log(OBFUSCATE("ip: 0x%x, sp: 0x%x, lr: 0x%x, pc: 0x%x"), (context)->uc_mcontext.arm_ip, (context)->uc_mcontext.arm_sp, (context)->uc_mcontext.arm_lr, (context)->uc_mcontext.arm_pc); \
    Log(OBFUSCATE("1: libGTASA.so + 0x%X"), context->uc_mcontext.arm_pc - g_libGTASA); \
    Log(OBFUSCATE("2: libGTASA.so + 0x%X"), context->uc_mcontext.arm_lr - g_libGTASA); \
    Log(OBFUSCATE("1: libmultirium-client.so + 0x%X"), context->uc_mcontext.arm_pc - g_libMultirium); \
    Log(OBFUSCATE("2: libmultirium-client.so + 0x%X"), context->uc_mcontext.arm_lr - g_libMultirium);
#else
#define PRINT_CRASH_STATES(context) \
    Log(OBFUSCATE("1: libGTASA.so + 0x%llx"), context->uc_mcontext.pc - g_libGTASA); \
    Log(OBFUSCATE("2: libGTASA.so + 0x%llx"), context->uc_mcontext.regs[30] - g_libGTASA); \
    Log(OBFUSCATE("1: libmultirium-client.so + 0x%llx"), context->uc_mcontext.pc - g_libMultirium); \
    Log(OBFUSCATE("2: libmultirium-client.so + 0x%llx"), context->uc_mcontext.regs[30] - g_libMultirium);
#endif

void Log(const char* fmt, ...);
extern int g_iLastRenderedObject;

extern uint16_t g_usLastProcessedModelIndexAutomobile;
extern int g_iLastProcessedModelIndexAutoEnt;

extern int g_iLastProcessedSkinCollision;
extern int g_iLastProcessedEntityCollision;
class CStackTrace
{
public:
    static void printBacktrace()
    {
        Log(OBFUSCATE("Build times: %s %s"), __TIME__, __DATE__);
        Log(OBFUSCATE("Last rendered object: %d"), g_iLastRenderedObject);
        Log(OBFUSCATE("Last processed auto and entity: %d %d"), g_usLastProcessedModelIndexAutomobile, g_iLastProcessedModelIndexAutoEnt);
        Log(OBFUSCATE("Last processed skin and entity: %d %d"), g_iLastProcessedSkinCollision, g_iLastProcessedEntityCollision);
        Log(OBFUSCATE("------------ START BACKTRACE ------------"));
        Log(OBFUSCATE(" "));
        PrintStackTrace();
    }

private:
    static _Unwind_Reason_Code TraceFunction(_Unwind_Context* context, void* arg) {
        uintptr_t pc = _Unwind_GetIP(context);

        Dl_info info;
        if (dladdr(reinterpret_cast<void*>(pc), &info) && info.dli_sname != nullptr) {
            Log(OBFUSCATE("[adr: %p multirium: %p gta: %p] %s\n"),
                     reinterpret_cast<void*>(pc),
                     reinterpret_cast<void*>(pc - g_libMultirium),
                     reinterpret_cast<void*>(pc - g_libGTASA),
                     info.dli_sname);
        } else {
            Log(OBFUSCATE("[adr: %p multirium: %p gta: %p] name not found\n"),
                     reinterpret_cast<void*>(pc),
                     reinterpret_cast<void*>(pc - g_libMultirium),
                     reinterpret_cast<void*>(pc - g_libGTASA));
        }

        return _URC_NO_REASON;
    }

    static void PrintStackTrace() {
        _Unwind_Backtrace(TraceFunction, nullptr);
    }

};

#endif //X64_CSTACKTRACE_H
