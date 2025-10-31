<!-- 
Student Guide - VM/Cache Simulator
Copyright (c) 2025 Amir Noohi
-->

# Student Guide

## Table of Contents
1. [Quick Start](#quick-start)
2. [What You're Building](#what-youre-building)
3. [Implementation Guide](#implementation-guide)
4. [Testing & Debugging](#testing--debugging)
5. [Common Issues](#common-issues)
6. [Reference](#reference)

---

## Quick Start

### Setup (30 seconds)
```bash
cd inf2cs-virt-memory-c
cp skeleton/cache.c src/
cp skeleton/multilevel_cache.c src/
cp skeleton/pagetable.c src/
```

### Build & Test
```bash
make        # Build simulator
make test   # Run tests (shows GREEN pass or RED fail)
```

### First Run
```bash
./sim -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt -v
```

---

## What You're Building

### Components

**You Implement** (3 files):
1. `cache.c` - Unified cache (Tasks 1-3)
2. `multilevel_cache.c` - L1+L2 hierarchy (Task 4)
3. `pagetable.c` - Page table with LRU

**Already Provided**:
- `main.c` - Main program
- `config.c` - Argument parsing
- `ll.c` - Linked list utilities
- `tlb.c` - TLB (can be reference or you implement)

### Architecture

```
Virtual Address
      ↓
    [TLB] ← You see this working
      ↓
 [Page Table] ← You implement this
      ↓
Physical Address
      ↓
    [Cache] ← You implement this
      ↓
   Memory
```

---

## Implementation Guide

### Understanding Cache Complexity

**Important: Consider starting with Direct-Mapped for simplicity!**

While the tasks are ordered 1-4, you might find it easier to implement in this order:

1. **Start with Direct-Mapped** (Task 3, but simpler):
   - Only 1 way per set → no replacement policy needed
   - No LRU list required
   - Simplest to debug
   - Then extend to other associativities

2. **Add Set-Associative & Fully-Associative**:
   - Requires LRU replacement policy
   - Must maintain doubly-linked list per set
   - More complex victim selection

**Key Implementation Requirements:**

⚠️ **All dynamic structures MUST use malloc/calloc**:
- `cache_t` structure → `calloc(1, sizeof(cache_t))`
- `cache->sets[]` array → `calloc(num_sets, sizeof(cache_set_t))`
- `cache->sets[i].lines[]` array → `calloc(ways_per_set, sizeof(cache_line_t))`
- `line->data[]` block → `calloc(block_size, sizeof(uint8_t))`

**Replacement Policy Summary:**

| Cache Type | Replacement Policy | LRU List? |
|------------|-------------------|-----------|
| Direct-mapped | None (only 1 choice) | ❌ No |
| 2-way / 4-way | LRU required | ✅ Yes |
| Fully-assoc | LRU required | ✅ Yes |

---

### Task 1: Fully Associative Cache

**Goal**: Implement basic cache with LRU replacement

**Note**: This requires LRU! Consider implementing direct-mapped first (see above).

**Key Functions** in `cache.c`:

#### 1. `cache_init(config)`
```c
cache_t* cache_init(cache_config_t config) {
    // 1. Allocate cache structure
    // 2. For fully-assoc:
    //    - num_sets = 1
    //    - ways_per_set = size / block_size
    // 3. Calculate bit fields:
    //    - offset_bits = log2(block_size)
    //    - index_bits = 0 (fully-assoc)
    //    - tag_bits = 32 - offset_bits
    // 4. Allocate cache sets and lines
    // 5. Initialize LRU doubly-linked list
    // 6. Return cache pointer
}
```

**Hints**:
- Use `calloc()` to allocate and zero-initialize
- Create doubly-linked list: `head ←→ line[0] ←→ line[1] ←→ ... ←→ tail`
- Each line needs: valid, dirty, tag, data block, prev, next pointers

#### 2. `cache_access(cache, addr, is_write)`
```c
cache_result_t cache_access(cache_t *cache, uint32_t addr, bool is_write) {
    // 1. Update statistics (accesses++, reads++ or writes++)
    // 2. Extract tag: addr >> offset_bits
    // 3. Search set for matching tag (loop through lines)
    // 4. If FOUND (hit):
    //    a. Update hit statistics
    //    b. Move line to head of LRU list
    //    c. Set dirty=true if write
    //    d. Return CACHE_HIT
    // 5. If NOT FOUND (miss):
    //    a. Update miss statistics  
    //    b. Select victim (find invalid line OR use LRU tail)
    //    c. If victim is dirty, write to memory (dummy function)
    //    d. Install new line (set valid=true, tag=tag, dirty=is_write)
    //    e. Move to head of LRU
    //    f. Return CACHE_MISS
}
```

**LRU Update Algorithm**:
```c
// To move line to head:
if (line == head) return;  // Already at head

// Remove from current position
if (line->prev) line->prev->next = line->next;
if (line->next) line->next->prev = line->prev;
if (line == tail) tail = line->prev;

// Insert at head
line->prev = NULL;
line->next = head;
head->prev = line;
head = line;
```

#### 3. `cache_destroy(cache)`
Free all allocated memory (sets, lines, data blocks).

**Test Task 1**:
```bash
make
./sim -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt -v
```

---

### Task 2: Variable Block Sizes

**Goal**: Support configurable block sizes

**Changes Needed**:
- `cache_init()` already handles `config.block_size`
- Recalculate `offset_bits = log2(block_size)`
- Allocate `block_size` bytes for each line's data

**If you did Task 1 correctly, this should already work!**

**Test Task 2**:
```bash
./sim -S 1024 -B 32 -T 4 -L 1 -t tests/testcase02/input.txt
```

---

### Task 3: All Associativities

**Goal**: Support direct-mapped, 2-way, 4-way, fully-assoc

**⚠️ Critical: LRU Requirements**

- **Direct-mapped**: NO LRU needed (1 way = no choice)
  - Skip LRU list initialization
  - Skip LRU updates in `cache_access()`
  - Simplest victim selection: just use the only line

- **Set-Associative (2-way, 4-way) & Fully-Associative**: LRU REQUIRED
  - Must maintain doubly-linked list per set
  - Update LRU on every access (HIT or MISS)
  - Select LRU tail as victim when all lines valid

**Modify `cache_init()`**:
```c
switch (config.associativity) {
    case DIRECT_MAPPED:
        num_sets = size / block_size;
        ways_per_set = 1;
        break;
    case TWO_WAY:
        ways_per_set = 2;
        num_sets = size / (block_size * 2);
        break;
    case FOUR_WAY:
        ways_per_set = 4;
        num_sets = size / (block_size * 4);
        break;
    case FULLY_ASSOC:
        num_sets = 1;
        ways_per_set = size / block_size;
        break;
}

// Calculate index_bits
if (associativity == FULLY_ASSOC) {
    index_bits = 0;
} else {
    index_bits = log2(num_sets);
}
```

**Modify `cache_get_index()`**:
```c
if (cache->associativity == FULLY_ASSOC) {
    return 0;  // Single set
}
// Extract index bits: (addr >> offset_bits) & ((1 << index_bits) - 1)
```

**Test Task 3**:
```bash
# Direct-mapped
./sim -S 1024 -B 16 -A 1 -T 4 -L 1 -t tests/testcase03/input.txt

# 2-way
./sim -S 1024 -B 16 -A 3 -T 4 -L 1 -t tests/testcase03/input.txt

# 4-way
./sim -S 1024 -B 16 -A 4 -T 4 -L 1 -t tests/testcase03/input.txt
```

---

### Task 4: Multi-Level Cache

**Goal**: Implement L1+L2 cache hierarchy

**File**: `multilevel_cache.c`

#### 1. `multilevel_cache_init(configs[], num_levels)`
```c
multilevel_cache_t* multilevel_cache_init(cache_config_t *configs, 
                                         uint32_t num_levels) {
    // 1. Validate num_levels (must be 2-4)
    // 2. Allocate multilevel_cache_t
    // 3. For each level:
    //    mlc->levels[i] = cache_init(configs[i]);
    // 4. Validate hierarchy (L2 size >= L1 size)
    // 5. Return mlc pointer
}
```

#### 2. `multilevel_cache_access(mlc, addr, is_write)`
```c
cache_result_t multilevel_cache_access(multilevel_cache_t *mlc,
                                       uint32_t addr, bool is_write) {
    // Loop through levels (EXTENSIBLE DESIGN!)
    for (uint32_t level = 0; level < mlc->num_levels; level++) {
        // Try this level
        result = cache_access(mlc->levels[level], addr, is_write);
        
        if (result == CACHE_HIT) {
            return CACHE_HIT_L1 + level;  // L1 hit, L2 hit, etc.
        }
        
        // Track accesses to L2+ (L1 gets all accesses via cache_access)
        if (level > 0) {
            mlc->level_accesses[level]++;
        }
    }
    
    // All levels missed
    return CACHE_MISS_ALL_LEVELS;
}
```

**Why loop-based?** Adding L3 later requires ZERO code changes!

**Test Task 4**:
```bash
./sim -S1 32768 -B1 64 -A1 2 -S2 262144 -B2 64 -A2 1 \
     -T 4 -L 1 -t tests/testcase04/input.txt -v
```

---

### Page Table Implementation

**File**: `pagetable.c`

**Data Structures**:
```c
static pte_t page_table[PAGE_TABLE_ENTRIES];  // 2^14 entries
static page_t *free_page_list;                 // Free pages
static page_t *used_page_list;                 // Used pages (LRU)
static page_t *frame_table[NUM_PHYSICAL_PAGES]; // 256 frames
```

#### 1. `pagetable_init()`
```c
void pagetable_init(void) {
    // 1. Initialize all page_table[] entries:
    //    present = false, dirty = false, ppn = 0
    
    // 2. Create 256 pages and add to free_page_list:
    for (i = 255; i >= 0; i--) {
        page = create_page(i);
        frame_table[i] = page;
        ll_insert_head(&free_page_list, page);  // Use ll.h!
    }
    
    // 3. used_page_list = NULL (empty initially)
}
```

#### 2. `pagetable_lookup(vpn, *ppn, *dirty)`
```c
pt_result_t pagetable_lookup(uint32_t vpn, uint32_t *ppn, bool *dirty) {
    pt_accesses++;
    
    if (page_table[vpn].present) {
        // Page HIT
        *ppn = page_table[vpn].ppn;
        *dirty = page_table[vpn].dirty;
        
        // Update LRU: move page to head of used_page_list
        page_t *page = frame_table[*ppn];
        ll_move_to_head(&used_page_list, page);
        
        return PT_HIT;
    }
    
    // Page fault
    return PT_MISS;
}
```

#### 3. `pagetable_handle_fault(vpn)`
```c
uint32_t pagetable_handle_fault(uint32_t vpn) {
    page_faults++;
    
    // Try free page first
    page_t *page = ll_remove_head(&free_page_list);
    
    if (!page) {
        // No free pages - evict LRU
        page = ll_get_tail(used_page_list);
        ll_remove_page(&used_page_list, page);
        
        // Write back if dirty
        if (page->pte->dirty) {
            page_faults_dirty++;
            write_page_to_disk(page->data);
        }
        
        // Mark old PTE as not present
        page->pte->present = false;
    }
    
    // Read new page from disk
    read_page_from_disk(page->data, vpn);
    
    // Update page table
    page_table[vpn].present = true;
    page_table[vpn].dirty = false;
    page_table[vpn].ppn = page->frame_id;
    page->pte = &page_table[vpn];
    
    // Add to head of used list (MRU)
    ll_insert_head(&used_page_list, page);
    
    return page->frame_id;
}
```

---

## Testing & Debugging

### Automated Testing
```bash
make test
```

Shows color-coded results:
- **GREEN [PASS]** - Test passed ✓
- **RED [FAIL]** - Test failed ✗

Example output:
```
[PASS] testcase01
[PASS] testcase02
[FAIL] testcase03: Output mismatch

Tests passed: 2/3
```

### Test Specific Case
```bash
# Run single test
./sim -S 32 -B 8 -A 1 -T 2 -L 1 -t tests/testcase01/input.txt -v

# Compare with expected
diff tests/testcase01/output.txt <(./sim ... -v)
```

### Debugging with Verbose Mode
```bash
./sim -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt -v
```

Output shows each access:
```
R 0x00001000 0x00000000 TLB-MISS PAGE-FAULT CACHE-MISS
R 0x00001004 0x00000004 TLB-HIT - CACHE-HIT
...

TLB Entries (Valid-Bit Dirty-Bit VPN PPN)
1 0 0x00001 0x00000

Page Table Entries (Present-Bit Dirty-Bit VPN PPN)
1 0 0x00001 0x00000
```

### Using GDB
```bash
make debug
gdb ./sim
```

```gdb
(gdb) break cache_access
(gdb) run -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt
(gdb) print cache->sets[0]
(gdb) next
(gdb) print *line
```

### Check Memory Leaks
```bash
valgrind --leak-check=full ./sim -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt
```

Should see: `All heap blocks were freed -- no leaks are possible`

### Add Debug Prints
```c
printf("DEBUG: index=%u, tag=%u, ways=%u\n", index, tag, cache->ways_per_set);
```

**Remember**: Remove before submission!

---

## Common Issues

### Issue 1: Segmentation Fault

**Causes**:
- Forgot to allocate memory for cache lines
- Forgot to allocate data blocks
- Array out of bounds

**Fix**:
```c
// In cache_init(), for each line:
line->data = calloc(block_size, sizeof(uint8_t));

// Check allocations:
if (!cache->sets) { /* handle error */ }
```

### Issue 2: LRU Not Working

**Cause**: Not updating LRU list on access

**Fix**:
```c
// In cache_access(), on HIT:
lru_move_to_head(set, line);  // Mark as most recently used
```

### Issue 3: Wrong Statistics

**Cause**: Updating counters in wrong places

**Fix**:
```c
// EVERY access:
cache->accesses++;
if (is_write) cache->writes++; else cache->reads++;

// On HIT:
cache->hits++;
if (is_write) cache->write_hits++; else cache->read_hits++;

// On MISS:
cache->misses++;  // Note: NO write_misses or read_misses counter
```

### Issue 4: Compilation Errors

**Common**:
```
error: 'strdup' undeclared
```

**Fix**: Already fixed in config.c with `#define _POSIX_C_SOURCE 200809L`

### Issue 5: Test Fails

**Debug Process**:
1. Run with `-v` to see detailed output
2. Compare line-by-line with expected output
3. Check statistics match
4. Verify LRU order in verbose output

---

## Reference

### Command-Line Arguments

**Single-Level Cache** (Tasks 1-3):
```
-S size      Cache size in bytes
-B blocksize Block size (default: 16, must be ≥4 and multiple of 4)
-A assoc     1=direct-mapped, 2=fully-assoc, 3=2-way, 4=4-way
-T entries   TLB entries (must be power of 2, ≥2)
-L assoc     TLB associativity (1/2/3/4)
-t file      Trace file path
-v           Verbose mode
```

**Multi-Level Cache** (Task 4):
```
-S1 size     L1 cache size
-B1 blocksize L1 block size
-A1 assoc    L1 associativity
-S2 size     L2 cache size (must be ≥ L1 size)
-B2 blocksize L2 block size
-A2 assoc    L2 associativity
-T entries   TLB entries
-L assoc     TLB associativity
-t file      Trace file
-v           Verbose mode
```

### Address Breakdown (32-bit)

**For Cache**:
```
|-- Tag --|-- Index --|-- Offset --|
  varies     varies      log2(block_size) bits
```

**For Page Table**:
```
|-- VPN (20 bits) --|-- Offset (12 bits) --|
    bits 31-12           bits 11-0
                         (4KB pages)
```

### Configuration Rules

**Must Error ("Invalid configuration")** if:
- Block size not multiple of 4 or < 4
- Cache size not multiple of 4 or ≤ 0
- For 2-way: `size % (block_size * 2) != 0`
- For 4-way: `size % (block_size * 4) != 0`
- TLB entries < 2 or not power of 2
- For 2-way TLB: `entries % 2 != 0`
- For 4-way TLB: `entries % 4 != 0`
- Trace file doesn't exist
- Task 4: L2 size < L1 size

### Useful Functions

**From ll.h** (for page lists):
```c
ll_insert_head(&list, page);      // Add to front
ll_remove_head(&list);             // Remove from front
ll_move_to_head(&list, page);     // Move to front (LRU update)
ll_get_tail(list);                 // Get LRU victim
ll_remove_page(&list, page);      // Remove specific page
```

**From cache.h**:
```c
log2_uint32(n);                    // Calculate log2 (for bit fields)
cache_get_index(cache, addr);      // Extract index from address
cache_get_tag(cache, addr);        // Extract tag from address
cache_get_offset(cache, addr);     // Extract offset from address
```

### Test Case Format

**input.txt**:
```
R 0x00001000
W 0x00002000
R 0x00001004
```

**params.txt**:
```
S - 1024
B - 16
A - 2
T - 4
L - 1
```

**output.txt**: Generated by reference implementation (with `-v`)

---

## Implementation Tips

### Start Simple
1. Get Task 1 working first (just fully-assoc)
2. Test thoroughly before moving to Task 2
3. Add one feature at a time
4. Test after each change

### Code Organization
```c
// Good: Small focused functions
static cache_line_t* find_line(cache_set_t *set, uint32_t tag);
static cache_line_t* select_victim(cache_set_t *set);
static void lru_move_to_head(cache_set_t *set, cache_line_t *line);

// Use them in cache_access()
```

### Memory Management
```c
// Allocate
cache->sets = calloc(num_sets, sizeof(cache_set_t));

// Always check
if (!cache->sets) return NULL;

// Free everything in cache_destroy()
```

### Testing Strategy
1. **Unit test**: Test one function at a time
2. **Integration test**: Run full simulator
3. **Automated test**: Use `make test`
4. **Edge cases**: Test with different configs
5. **Memory test**: Run valgrind

---

## Submission Checklist

Before submitting:

- [ ] Code compiles without warnings: `make`
- [ ] All tests pass: `make test`
- [ ] No memory leaks: `valgrind --leak-check=full ./sim ...`
- [ ] Code is well-commented (algorithm explanations)
- [ ] No debug printf() statements
- [ ] Proper indentation (4 spaces)
- [ ] Function signatures match headers exactly
- [ ] Tested all associativity modes
- [ ] Tested Task 4 multi-level cache

---

## Getting Help

1. **Read headers** - Full API documentation in `include/*.h`
2. **Check skeleton** - Detailed TODOs in `skeleton/*.c`
3. **Use verbose mode** - See what your code is doing
4. **Compare outputs** - Diff your output vs. expected
5. **Ask on Piazza** - Don't post complete solutions!

---

## Examples

### Task 1 Example
```bash
./sim -S 1024 -T 4 -L 1 -t tests/testcase01/input.txt -v
```

### Task 3 Example (2-way set-assoc)
```bash
./sim -S 2048 -B 16 -A 3 -T 8 -L 2 -t tests/testcase05/input.txt
```

### Task 4 Example (L1+L2)
```bash
./sim -S1 16384 -B1 32 -A1 3 \
      -S2 131072 -B2 32 -A2 1 \
      -T 16 -L 2 -t tests/testcase10/input.txt -v
```

---

**Good luck! Start simple, test frequently, debug systematically.** 🚀

---

Copyright (c) 2025 Amir Noohi. All rights reserved.
