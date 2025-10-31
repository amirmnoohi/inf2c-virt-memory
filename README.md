<!-- 
VM/Cache Simulator - C Implementation
Copyright (c) 2025 Amir Noohi
-->

# VM/Cache Simulator

C implementation for INF2C-CS coursework.

**Features**: Unified cache (all associativities) • Multi-level hierarchy (L1+L2, L3-ready) • 50 test cases • Automated testing

## Quick Start

```bash
make        # Build (zero warnings)
make test   # Test (100% passing - 50/50 tests)
```

## Documentation

- **[STUDENT_GUIDE.md](STUDENT_GUIDE.md)** - Complete guide (build, implement, test, debug)
- **[PROJECT.md](PROJECT.md)** - Specification and task requirements

## Structure

```
include/    - 7 headers (complete interfaces)
src/        - 7 implementations (~3500 LOC)
skeleton/   - 3 student starters (cache, multilevel_cache, pagetable)
tests/      - 50 test cases with comprehensive coverage
tools/      - Test automation scripts
Makefile    - Build system with debug/test targets
```

## Test Results

**✅ 100% PASSING (50/50 test cases)**

All test cases pass with automated verification. The implementation correctly handles all cache configurations and validates invalid inputs.

## Usage Examples

```bash
# Task 1: Fully associative
./sim -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt -v

# Task 3: 2-way set associative
./sim -S 1024 -B 16 -A 3 -T 8 -L 2 -t tests/testcase03/input.txt

# Task 4: Multi-level (L1 + L2)
./sim -S1 32768 -B1 64 -A1 2 -S2 262144 -B2 64 -A2 1 \
      -T 16 -L 2 -t tests/testcase10/input.txt -v
```

## Key Design

- **Unified codebase** - Configuration-driven, single cache.c for all modes
- **Extensible** - Add L3 with zero code changes (loop-based multilevel)
- **Clean implementation** - C11, strict warnings, zero leaks, comprehensive docs

## Architecture & Data Flow

### Complete Read/Write Path

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TRACE FILE: R/W 0xVIRTUAL_ADDRESS                        │
└────────────────────────────────┬────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         MAIN.C: main() loop                                 │
│                     (line 243: fscanf trace file)                           │
└────────────────────────────────┬────────────────────────────────────────────┘
                                 │
                                 ▼
        ┌────────────────────────────────────────────────────┐
        │  Parse: mode='R'/'W', vaddr=0x????????             │
        └────────────────────────┬───────────────────────────┘
                                 │
                                 ▼
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                    STEP 1: ADDRESS TRANSLATION                           ┃
┃                   (Virtual → Physical Address)                           ┃
┃                  main.c: translate_address() [line 76]                   ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                                │
                                ├─► Extract VPN (bits 31-12) & offset (11-0)
                                │
                                ▼
                    ┌─────────────────────────┐
                    │   1a. TLB LOOKUP        │
                    │   tlb.c: tlb_lookup()   │
                    │   [line 217]            │
                    └──┬───────────┬──────────┘
                       │           │
              TLB HIT  │           │  TLB MISS
              ─────────┤           ├──────────
                       │           │
                       ▼           ▼
              ┌────────────┐   ┌──────────────────────┐
              │ Get PPN    │   │ 1b. PAGE TABLE       │
              │ from TLB   │   │     LOOKUP           │
              │            │   │ pagetable.c:         │
              │            │   │ pagetable_lookup()   │
              │            │   │ [line 141]           │
              └─────┬──────┘   └────┬─────────┬───────┘
                    │               │         │
                    │      PT HIT   │         │  PT MISS (Page Fault)
                    │      ─────────┤         ├────────────────────
                    │               │         │
                    │               │         ▼
                    │               │    ┌──────────────────────────┐
                    │               │    │ 1c. HANDLE PAGE FAULT    │
                    │               │    │ pagetable_handle_fault() │
                    │               │    │ [line 163]               │
                    │               │    │                          │
                    │               │    │ • Get free page OR       │
                    │               │    │ • Evict LRU page         │
                    │               │    │   - If dirty: write disk │
                    │               │    │ • Read new page from disk│
                    │               │    │ • Update page table      │
                    │               │    │ • Return PPN             │
                    │               │    └─────────┬────────────────┘
                    │               │              │
                    │               │◄─────────────┘
                    │               │
                    │               ▼
                    │          ┌────────────────────┐
                    │          │ 1d. UPDATE TLB     │
                    │          │ tlb.c:             │
                    │          │ tlb_insert()       │
                    │          │ [line 243]         │
                    │          └─────────┬──────────┘
                    │                    │
                    └────────────────────┼──────────► PPN obtained
                                         │
                                         ▼
                              ┌───────────────────────┐
                              │ If WRITE:             │
                              │ • tlb_set_dirty()     │
                              │ • pagetable_set_dirty│
                              └──────────┬────────────┘
                                         │
                                         ▼
                    ┌─────────────────────────────────────┐
                    │ Construct Physical Address:         │
                    │ paddr = (PPN << 12) | offset        │
                    └────────────┬────────────────────────┘
                                 │
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                       STEP 2: CACHE ACCESS                               ┃
┃                    (Using Physical Address)                              ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                                 │
            ┌────────────────────┴───────────────────────┐
            │                                            │
            ▼ TASKS 1-3 (Single Cache)                   ▼ TASK 4 (Multi-level)
   ┌─────────────────────────┐              ┌───────────────────────────────┐
   │ 2a. SINGLE CACHE        │              │ 2b. MULTILEVEL CACHE          │
   │ cache.c: cache_access() │              │ multilevel_cache.c:           │
   │ [line 283]              │              │ multilevel_cache_access()     │
   │                         │              │ [line 95]                     │
   │ • Parse address:        │              └────┬─────────────┬────────────┘
   │   - offset bits         │                   │             │
   │   - index bits          │                   ▼             │
   │   - tag bits            │         ┌───────────────────┐   │
   │ • Get cache set         │         │ L1 CACHE ACCESS   │   │
   │ • Search for tag        │         │ cache_access()    │   │
   │                         │         └────┬──────┬───────┘   │
   │ ┌─────────────────┐     │              │      │           │
   │ │ HIT:            │     │     L1 HIT   │      │  L1 MISS  │
   │ │ • Update stats  │     │     ─────────┤      ├───────    │
   │ │ • Update LRU    │     │              │      │           │
   │ │ • If write:     │     │              ▼      ▼           │
   │ │   set dirty bit │     │       ┌──────┐  ┌──────────────┐│
   │ └─────────────────┘     │       │RETURN│  │ L2 CACHE     ││
   │                         │       │L1-HIT│  │ ACCESS       ││
   │ ┌─────────────────┐     │       └──────┘  │ cache_access│││
   │ │ MISS:           │     │                 └─┬──────┬─────┘│
   │ │ • Select victim │     │                   │      │      │
   │ │   (LRU or       │     │          L2 HIT   │      │ L2   │
   │ │    invalid)     │     │           ────────┤      │ MISS │
   │ │ • If victim     │     │                   │      │      │
   │ │   dirty: evict  │     │                   ▼      ▼      │
   │ │ • Install new   │     │              ┌────────────────┐ │
   │ │   block         │     │              │ Fetch from     │ │
   │ │ • If write:     │     │              │ memory         │ │
   │ │   set dirty     │     │              │ Install in     │ │
   │ │ • Update LRU    │     │              │ both L1 & L2   │ │
   │ └─────────────────┘     │              └────────────────┘ │
   └─────────┬───────────────┘                                 │
             │                                                 │
             └──────────────────┬──────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │ Return cache_result_t │
                    │ • CACHE_HIT / MISS    │
                    │ • L1-HIT / L2-HIT     │
                    │ • L1-MISS L2-MISS     │
                    └──────────┬────────────┘
                               │
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                       STEP 3: OUTPUT & STATISTICS                       ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
                               │
                               ▼
                    ┌──────────────────────┐
                    │ If verbose mode:     │
                    │ print_verbose()      │
                    │ [line 124]           │
                    │                      │
                    │ R/W 0xVADDR 0xPADDR  │
                    │ TLB-HIT/MISS         │
                    │ PAGE-HIT/FAULT/-     │
                    │ CACHE-HIT/MISS       │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │ Update statistics:   │
                    │ • TLB hits/misses    │
                    │ • Page faults        │
                    │ • Cache hits/misses  │
                    └──────────┬───────────┘
                               │
                               │ (loop back for next trace entry)
                               │
                               ▼
                    ┌──────────────────────┐
                    │ After all accesses:  │
                    │ • tlb_print_stats()  │
                    │ • pagetable_print... │
                    │ • cache_print_stats()│
                    └──────────────────────┘
```

### Memory Hierarchy & Data Structures

```
┌─────────────────────────────────────────────────────────────┐
│                    MEMORY HIERARCHY                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  TLB (tlb_t) [malloc'd]                                     │
│    └─> sets[] [malloc'd]                                    │
│         └─> entries[] per set [malloc'd]                    │
│              • LRU linked list                              │
│              • VPN → PPN mappings                           │
│                                                             │
│  PAGE TABLE (static array)                                  │
│    └─> page_table[16384] (2^14 entries)                     │
│         • present, dirty, ppn fields                        │
│                                                             │
│  PHYSICAL MEMORY                                            │
│    └─> frame_table[256] → page_t* [malloc'd]                │
│         • free_page_list (linked)                           │
│         • used_page_list (LRU, linked)                      │
│                                                             │
│  CACHE (cache_t) [malloc'd] - Tasks 1-3                     │
│    └─> sets[] [malloc'd]                                    │
│         └─> lines[] per set [malloc'd]                      │
│              └─> data[] per line [malloc'd]                 │
│                   • LRU linked list                         │
│                   • valid, dirty, tag                       │
│                                                             │
│  MULTILEVEL CACHE (multilevel_cache_t) [malloc'd] - Task 4  │
│    └─> levels[0] = L1 cache_t* [malloc'd]                   │
│    └─> levels[1] = L2 cache_t* [malloc'd]                   │
│         (each with same structure as single cache)          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Execution Summary

1. **READ/WRITE from trace** → Virtual address + mode
2. **TLB lookup** → Fast VPN→PPN translation (if hit)
3. **Page Table** → If TLB miss, get PPN from page table
4. **Page Fault Handler** → If page not present, allocate/evict
5. **Physical Address** → Construct from PPN + offset
6. **Cache Access** → Single-level or L1→L2 hierarchy
7. **LRU Updates** → Move accessed items to MRU position
8. **Statistics** → Track hits/misses at each level

**All dynamic memory uses malloc/calloc** as required!

---

Copyright (c) 2025 Amir Noohi. All rights reserved.
