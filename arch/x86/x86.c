/*
 * x86 feature check
 *
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
 * Author:
 *  Jim Kukunas
 * 
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "x86.h"

int x86_cpu_has_sse42 = 0;
int x86_cpu_has_pclmul = 0;
int x86_cpu_has_avx2 = 0;
int x86_cpu_has_avx512 = 0;
int x86_cpu_has_vpclmulqdq = 0;

static void _x86_check_features(void);

#ifndef _MSC_VER
#include <pthread.h>
#include <cpuid.h>

pthread_once_t cpu_check_inited_once = PTHREAD_ONCE_INIT;

void x86_check_features(void)
{
    pthread_once(&cpu_check_inited_once, _x86_check_features);
}

static void cpuid(int info, unsigned* eax, unsigned* ebx, unsigned* ecx, unsigned* edx) 
{
    __cpuid(info, *eax, *ebx, *ecx, *edx);    
}

static void cpuidex(int info, int subinfo, unsigned* eax, unsigned* ebx, unsigned* ecx, unsigned* edx)
{
    __cpuid_count(info, subinfo, *eax, *ebx, *ecx, *edx);
}
#else

// In the CRT in Visual C++ isa_availability.h is available which exposes CPU features already:
// #include <isa_availability.h>
// __isa_enabled & (1 << __ISA_AVAILABLE_SSE42)
// __isa_enabled & (1 << __ISA_AVAILABLE_AVX)
// __isa_enabled & (1 << __ISA_AVAILABLE_AVX2)
// __isa_enabled & (1 << __ISA_AVAILABLE_AVX512)
//
// We won't be able to check for vpclmulqdq though

#include <intrin.h>
#include <windows.h>

static INIT_ONCE once_control /*= INIT_ONCE_STATIC_INIT*/;

static _Success_(return != FALSE) BOOL CALLBACK _x86_check_features_once(PINIT_ONCE InitOnce, PVOID Parameter, PVOID * Context)
{
    _x86_check_features();
	return TRUE;
}

void x86_check_features(void)
{
    // Consider using a flag whether _x86_check_features_once has been called or not
    // In a multi thread environment, in the worst case _x86_check_features_once is called
    // multiple times instead of once.
    InitOnceExecuteOnce(&once_control, _x86_check_features_once, NULL, NULL);
}

static void cpuid(int info, unsigned* eax, unsigned* ebx, unsigned* ecx, unsigned* edx) {
    int registers[4];
    __cpuid(registers, info);

    *eax = (unsigned)registers[0];
    *ebx = (unsigned)registers[1];
    *ecx = (unsigned)registers[2];
    *edx = (unsigned)registers[3];
}

static void cpuidex(int info, int subinfo, unsigned* eax, unsigned* ebx, unsigned* ecx, unsigned* edx)
{
    int registers[4];
    __cpuidex(registers, info, subinfo);

    *eax = (unsigned) registers[0];
    *ebx = (unsigned) registers[1];
    *ecx = (unsigned) registers[2];
    *edx = (unsigned) registers[3];
}
#endif  /* _MSC_VER */

static void _x86_check_features(void)
{
    unsigned eax, ebx, ecx, edx;
    cpuid(1 /*CPU_PROCINFO_AND_FEATUREBITS*/, &eax, &ebx, &ecx, &edx);

    x86_cpu_has_sse42 = ecx & 0x100000;
    // All known cpus from Intel and AMD with CLMUL also support SSE4.2
    x86_cpu_has_pclmul = ecx & 0x2;

    cpuid(0, &eax, &ebx, &ecx, &edx);
    if (eax >= 7)
    {
        cpuidex(7 /*CPU_EXTENDED_PROC_INFO_FEATURE_BITS*/, 0, &eax, &ebx, &ecx, &edx);

        x86_cpu_has_avx2 = ebx & (1 << 5);
        x86_cpu_has_avx512 = ebx & 0x00010000;
        x86_cpu_has_vpclmulqdq = ecx & 0x400;
    }
    else
    {
        x86_cpu_has_avx2 = 0;
    }
}
