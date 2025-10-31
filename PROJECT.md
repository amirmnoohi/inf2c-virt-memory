<!-- 
Project Specification - VM/Cache Simulator
Copyright (c) 2025 Amir Noohi
-->

# Project Specification

## Objective

Build a virtual memory and cache simulator in C that models:
- Hardware cache (write-back, write-allocate, LRU)
- TLB (Translation Lookaside Buffer)
- Page table (linear, LRU eviction)
- Multi-level cache hierarchy (L1+L2, extensible to L3+)

## Progressive Tasks

### Task 1: Fully Associative Cache
Implement a fully associative cache with fixed 16-byte blocks.

**Learning objectives**:
- Cache fundamentals
- LRU replacement policy
- Hit/miss detection

**Implementation**: `cache.c`

### Task 2: Variable Block Sizes
Extend Task 1 to support configurable block sizes.

**Learning objectives**:
- Address bit field calculation
- Dynamic memory allocation
- Offset/index/tag extraction

**Modification**: `cache_init()` in `cache.c`

### Task 3: All Associativities
Support direct-mapped, 2-way, 4-way, and fully associative caches.

**Learning objectives**:
- Set-associative caches
- Cache geometry calculations
- Unified implementation patterns

**Modification**: `cache_init()` and `cache_access()` in `cache.c`

### Task 4: Multi-Level Cache
Implement L1+L2 cache hierarchy with extensible design.

**Learning objectives**:
- Memory hierarchy
- Cache inclusion policies
- Extensible architectures

**Implementation**: `multilevel_cache.c`

## Technical Specifications

### Cache
- **Size**: Configurable (-S parameter)
- **Block size**: Configurable (-B parameter, default 16)
- **Associativity**: 1/2/3/4 (-A parameter, default 2)
- **Write policy**: Write-back, write-allocate
- **Replacement**: LRU (for associativity > 1)

### Multi-Level Cache (Task 4)
- **L1**: Configurable size, block size, associativity
- **L2**: Must be >= L1 size
- **Access flow**: L1 → L2 → Memory
- **Extensible**: Designed to easily add L3, L4, etc.

### TLB
- **Entries**: Configurable (-T parameter)
- **Associativity**: 1/2/3/4 (-L parameter)
- **Page size**: Fixed 4KB (12-bit offset)
- **Replacement**: LRU

### Page Table
- **Type**: Linear (array-based)
- **Entries**: 2^14 (26-bit virtual address space)
- **Physical pages**: 256 (1MB physical memory)
- **Replacement**: LRU

## Input Format

Trace file contains memory accesses:
```
R 0x00001000   # Read from 0x00001000
W 0x00002000   # Write to 0x00002000
R 0x00001004   # Read from 0x00001004
```

## Output Format

### Normal Mode
Statistics for TLB, Page Table, and Cache(s).

### Verbose Mode (-v)
Per-access details plus statistics and final entries.

## Configuration Validation

Program must print "Invalid configuration" and exit for:
- Block size not multiple of 4 or < 4
- Invalid associativity code
- For 2-way: size not divisible by (block_size * 2)
- For 4-way: size not divisible by (block_size * 4)
- TLB entries < 2 or not power of 2
- For 2-way TLB: entries not divisible by 2
- For 4-way TLB: entries not divisible by 4
- Trace file does not exist
- For Task 4: L2 size < L1 size

## Requirements

### Code Quality
- **Standard**: C11 (`-std=c11`)
- **Warnings**: Must compile with `-Wall -Wextra -Wpedantic -Werror`
- **Memory**: No memory leaks (verify with valgrind)
- **Comments**: Well-documented code
- **Style**: Consistent indentation and naming

### Functionality
- All test cases must pass
- Output format must match specification exactly
- Verbose mode must work correctly
- All associativity modes must be supported

### Design
- Clean modular design
- Proper separation of concerns
- No code duplication (unified approach)
- Extensible architecture

## Provided vs. Implement

### Provided to Students
- `main.c` - Complete main program
- `config.c` - Configuration parsing
- `ll.c` - Linked list utilities
- `src/tlb.c` - TLB (optional: can be skeleton)
- All header files with full documentation
- Makefile
- Test cases

### Students Implement
- `cache.c` - Unified cache (Tasks 1-3)
- `multilevel_cache.c` - Multi-level wrapper (Task 4)
- `pagetable.c` - Page table and memory management

## Design Philosophy

### Unified Codebase
Single `cache.c` handles all associativities through configuration, not separate implementations. This teaches:
- Configuration-driven design
- DRY principle
- Industrial best practices

### Extensible Multi-Level
Loop-based design allows adding L3, L4, etc. with zero code changes. This teaches:
- Scalable architecture
- Abstraction
- Future-proof design

## Copyright

Copyright (c) 2025 Amir Noohi. Educational use only.

