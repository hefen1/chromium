// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SANDBOX_LINUX_BPF_DSL_LINUX_SYSCALL_RANGES_H_
#define SANDBOX_LINUX_BPF_DSL_LINUX_SYSCALL_RANGES_H_

#include <asm/unistd.h>  // For __NR_SYSCALL_BASE

#if defined(__x86_64__)

#define MIN_SYSCALL         0u
#define MAX_PUBLIC_SYSCALL  1024u
#define MAX_SYSCALL         MAX_PUBLIC_SYSCALL

#elif defined(__i386__)

#define MIN_SYSCALL         0u
#define MAX_PUBLIC_SYSCALL  1024u
#define MAX_SYSCALL         MAX_PUBLIC_SYSCALL

#elif defined(__arm__) && (defined(__thumb__) || defined(__ARM_EABI__))

// ARM EABI includes "ARM private" system calls starting at |__ARM_NR_BASE|,
// and a "ghost syscall private to the kernel", cmpxchg,
// at |__ARM_NR_BASE+0x00fff0|.
// See </arch/arm/include/asm/unistd.h> in the Linux kernel.
#define MIN_SYSCALL         ((unsigned int)__NR_SYSCALL_BASE)
#define MAX_PUBLIC_SYSCALL  (MIN_SYSCALL + 1024u)
#define MIN_PRIVATE_SYSCALL ((unsigned int)__ARM_NR_BASE)
#define MAX_PRIVATE_SYSCALL (MIN_PRIVATE_SYSCALL + 16u)
#define MIN_GHOST_SYSCALL   ((unsigned int)__ARM_NR_BASE + 0xfff0u)
#define MAX_SYSCALL         (MIN_GHOST_SYSCALL + 4u)

#elif defined(__mips__) && (_MIPS_SIM == _ABIO32)

#define MIN_SYSCALL         __NR_O32_Linux
#define MAX_PUBLIC_SYSCALL  (MIN_SYSCALL + __NR_Linux_syscalls)
#define MAX_SYSCALL         MAX_PUBLIC_SYSCALL

#elif defined(__mips__) && (_MIPS_SIM == _ABI64)

#error "Add support to header file"

#elif defined(__aarch64__)

#define MIN_SYSCALL 0u
#define MAX_PUBLIC_SYSCALL 279u
#define MAX_SYSCALL MAX_PUBLIC_SYSCALL

#else
#error "Unsupported architecture"
#endif

#endif  // SANDBOX_LINUX_BPF_DSL_LINUX_SYSCALL_RANGES_H_