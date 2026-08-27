#pragma once
#include <stdint.h>
#include <kernel/scheduler/thread.h>
#include <kernel/scheduler/process.h>

uint64_t brk(uint64_t addr);