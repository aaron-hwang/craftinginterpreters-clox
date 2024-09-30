//
// Created by aaron on 8/6/2024.
//

#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Comment these out for better performance.
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

// Stress test mode for GC, will run as frequently as possible
#define DEBUG_STRESS_GC
// Option for logging whenever we do something with dynamic memory (allocation, free, etc)
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)

#endif