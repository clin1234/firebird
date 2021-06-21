#ifndef _H_MMU
#define _H_MMU

#include "cpu.h"
#include "emu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Translate a VA to a PA, using a page table lookup */
uint32_t mmu_translate(uint32_t addr, bool writing, fault_proc *fault, uint8_t *s_status);

/* Does access as-if in USR mode and calls data_abort on failure */
void mmu_user_access(uint32_t addr, bool writing);

/* addr_cache is used to cache the translation of VA to PA per page.
 * It has two entries per page (1k), one for reading and one for writing.
 * The format for 32-bit x86 with JIT is different to minimize the needed
 * registers in the x86 memory access functions and won't work for all
 * platforms:
 *
 * a) Pointer entry
 *    The result of adding the virtual address (VA) to the entry has bit 31
 *    clear, and that sum is a pointer to within mem_and_flags.
 *    It would be cleaner to just use bit 0 or 1 to distinguish this case, but
 *    this way we can simultaneously check both that this is a pointer entry,
 *    and that the address is aligned, with a single bit test instruction.
 * b) Physical address entry
 *    VA + entry has bit 31 set, and the entry (not the sum) has bit 22 clear.
 *    Bits 0-21 contain the difference between virtual and physical address.
 * c) Invalid entry
 *    VA + entry has bit 31 set, entry has bit 22 set. Entry is invalid and
 *    addr_cache_miss must be called.
 *
 * In all other cases the format is simpler:
 * a) Pointer entry: Bit 0 clear, rest difference to pointer to start of physical page
 * b) Physical entry: Bit 0 set, Bit 1 clear, rest difference to physical address
 * c) Invalid entry: Bit 0 set, Bit 1 set
 */

#define AC_NUM_ENTRIES (((1ull << 32) >> 10) * 2)
typedef uint8_t *ac_entry;
extern ac_entry *addr_cache __asm__("addr_cache");

#if defined(__i386__) && !defined(NO_TRANSLATION)
    #define AC_SET_ENTRY_PTR(entry, va, ptr) \
            entry = (ptr) - (va);
    #define AC_NOT_PTR 0x80000000
    #define AC_INVALID (1 << 22)
    #define AC_SET_ENTRY_PHYS(entry, va, pa) \
            entry = (ac_entry)(((pa) - (va)) >> 10); \
            entry += (~(uintptr_t)((va) + entry) & AC_NOT_PTR);
    #define AC_SET_ENTRY_INVALID(entry, va) \
            entry = (ac_entry)(AC_INVALID); \
            entry += (~(uintptr_t)((va) + entry) & AC_NOT_PTR);
#else
    #define AC_SET_ENTRY_PTR(entry, va, ptr) \
            entry = (ptr) - (va);
    #define AC_NOT_PTR 0x1
    #define AC_INVALID 0x2
    #define AC_FLAGS 0x3
    #define AC_SET_ENTRY_PHYS(entry, va, pa) \
            entry = (ac_entry)(((pa) - (va)) | AC_NOT_PTR);
    #define AC_SET_ENTRY_INVALID(entry, va) \
            entry = (ac_entry)(AC_INVALID | AC_NOT_PTR);
#endif

bool addr_cache_pagefault(void *addr);
void *addr_cache_miss(uint32_t addr, bool writing, fault_proc *fault) __asm__("addr_cache_miss");
void addr_cache_flush();
void mmu_dump_tables(void);

#ifdef __cplusplus
}
#endif

#endif
