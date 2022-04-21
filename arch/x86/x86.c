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

static void _x86_check_features(void);

#ifndef _MSC_VER
#include <pthread.h>
#include <cpuid.h>

pthread_once_t cpu_check_inited_once = PTHREAD_ONCE_INIT;

void x86_check_features(void)
{
    pthread_once(&cpu_check_inited_once, _x86_check_features);
}

static void cpuid(int info, unsigned* eax, unsigned* ebx, unsigned* ecx, unsigned* edx) {
    unsigned int _eax;
    unsigned int _ebx;
    unsigned int _ecx;
    unsigned int _edx;
    __cpuid(info, _eax, _ebx, _ecx, _edx);
    *eax = _eax;
    *ebx = _ebx;
    *ecx = _ecx;
    *edx = _edx;
}
#else
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
#endif  /* _MSC_VER */

static void _x86_check_features(void)
{
    unsigned eax, ebx, ecx, edx;
    cpuid(1 /*CPU_PROCINFO_AND_FEATUREBITS*/, &eax, &ebx, &ecx, &edx);

    x86_cpu_has_sse42 = ecx & 0x100000;
    // All known cpus from Intel and AMD with CLMUL also support SSE4.2
    x86_cpu_has_pclmul = ecx & 0x2;
}
