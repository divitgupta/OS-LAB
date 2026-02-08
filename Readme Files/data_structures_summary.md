# Data Structures & Memory Management Summary

This document explains the data structures used in the GC Simulator and their performance characteristics.

---

## 1. Core Data Structures

| Data Structure | Role in System | Justification |
| :--- | :--- | :--- |
| **Static Array (Heap)** | Physical Memory Pool | Models a fixed-size memory region where objects are allocated. Using a static array (`struct Block heap[HEAP_SIZE]`) provides $O(1)$ access to any block and avoids the "inception problem" of using `malloc` to simulate `malloc`. |
| **Static Array (Stack)** | Root Set Storage | Stores references to live objects that the application currently owns. Stack operations (push/pop) are $O(1)$, and iteration during mark phase is $O(R)$ where $R$ is the number of roots. |
| **Metadata Flags** | Object State Tracking | Each block has `allocated` and `marked` flags (2 bits of metadata). This allows the GC to distinguish between free, allocated, and reachable memory without additional data structures. |
| **HTTP/JSON Protocol** | Frontend-Backend Communication | The Mongoose server uses event-driven I/O to handle multiple concurrent requests. Manual JSON serialization avoids library dependencies while keeping the protocol human-readable. |

---

## 2. Memory Management: The Heap

### A. Heap Structure
```c
struct Block {
    int allocated;  // 1 = in use, 0 = free
    int marked;     // 1 = reachable (during GC), 0 = garbage
};

struct Block heap[HEAP_SIZE];  // Global static array
```

### B. Allocation Strategy
- **Algorithm**: First-fit linear scan
- **Complexity**: $O(N)$ where $N$ is heap size
- **Rationale**: Simple and predictable. For small heaps (10-20 blocks), this is faster than maintaining a free list.

### C. Fragmentation
- **Internal**: None (all blocks are same size)
- **External**: Possible (free blocks may be scattered)
- **Mitigation**: In a real system, we'd use **Mark-Compact** instead of Mark-Sweep to eliminate fragmentation.

---

## 3. Memory Management: The Stack

### A. Stack Structure
```c
int process_stack[STACK_SIZE];  // Array of heap indices
int stack_top = -1;             // -1 = empty, 0 = one element
```

### B. Stack Operations
- **Push**: `process_stack[++stack_top] = index;` → $O(1)$
- **Pop**: `stack_top--;` → $O(1)$
- **Validation**: Before push, check:
  1. Stack not full: `stack_top < STACK_SIZE - 1`
  2. Index in range: `0 <= index < HEAP_SIZE`
  3. Block allocated: `heap[index].allocated == 1`

---

## 4. Garbage Collection Algorithm

### A. Mark Phase
- **Input**: `process_stack` (the root set)
- **Process**: For each index in stack, set `heap[index].marked = 1`
- **Complexity**: $O(R)$ where $R$ is the number of roots
- **Note**: In a real GC, we'd also trace heap-to-heap references (object graphs). Our simulator only has stack-to-heap references.

### B. Sweep Phase
- **Process**: Scan entire heap. If `allocated == 1 && marked == 0`, set `allocated = 0`
- **Complexity**: $O(H)$ where $H$ is heap size
- **Result**: All unreachable blocks are freed

### C. Clear Marks Phase
- **Process**: Reset all `marked` flags to 0
- **Complexity**: $O(H)$
- **Purpose**: Prepare for next GC cycle

### D. Total GC Complexity
- **Time**: $O(R + H)$ = Linear in roots + heap size
- **Space**: $O(1)$ = No additional memory needed (in-place algorithm)

---

## 5. DSA Performance Matrix

| Object | Operations | Complexity | Why This Matters |
| :--- | :--- | :--- | :--- |
| **Heap Allocation** | Find free block | $O(N)$ | Acceptable for small heaps. Real systems use free lists or bitmaps for $O(1)$ allocation. |
| **Stack Push/Pop** | Add/Remove reference | $O(1)$ | Instant root management. |
| **Mark Phase** | Trace from roots | $O(R)$ | Scales with number of active references, not heap size. |
| **Sweep Phase** | Scan heap | $O(H)$ | Must check every block. Unavoidable in Mark-Sweep. |
| **Memory Overhead** | Metadata per block | 2 integers | In production, this would be 2 bits (packed into a byte). |

---

## 6. Comparison: Mark-Sweep vs Other GC Algorithms

### A. Mark-Sweep (Our Implementation)
- **Pros**: 
  - Handles circular references ✓
  - No reference counting overhead ✓
  - Simple to implement ✓
- **Cons**:
  - "Stop-the-world" pause ✗
  - Fragmentation (external) ✗
  - Must scan entire heap ✗

### B. Reference Counting
- **Pros**: 
  - Immediate reclamation ✓
  - No GC pauses ✓
- **Cons**:
  - Cannot handle cycles ✗
  - Overhead on every pointer operation ✗

### C. Mark-Compact
- **Pros**: 
  - No fragmentation ✓
  - Handles cycles ✓
- **Cons**:
  - More complex (must move objects) ✗
  - Longer GC pause ✗

### D. Generational GC (Java/Python)
- **Pros**: 
  - Fast for short-lived objects ✓
  - Reduces pause time ✓
- **Cons**:
  - Complex implementation ✗
  - Multiple heap regions needed ✗

---

## 7. Memory Safety Features

### A. Bounds Checking
- **Heap Access**: All indices validated against `HEAP_SIZE`
- **Stack Access**: Push checks for overflow, pop checks for underflow
- **Result**: No buffer overflows or segmentation faults

### B. Dangling Pointer Prevention
- **Rule**: Cannot push reference to unallocated block
- **Check**: `if (!heap[index].allocated) return ERROR;`
- **Result**: Stack only contains valid references

### C. Double-Free Prevention
- **Mechanism**: Sweep only frees blocks where `allocated == 1`
- **Result**: Cannot free the same block twice

---

## 8. Real-World Analogies

| Our Simulator | Real Operating System |
| :--- | :--- |
| `heap[]` array | Physical RAM pages |
| `allocated` flag | Page table "present" bit |
| `marked` flag | Reference bit (for LRU) |
| `process_stack[]` | Process page tables |
| `run_gc()` | Kernel memory reclamation |
| Sweep phase | Page frame reclaimer |

---

## 9. Exam Talking Points

1. **"Why static arrays instead of malloc?"**
   - Avoids the "inception problem" of simulating dynamic memory with dynamic memory.
   - Provides predictable, bounded memory usage.

2. **"How does this scale?"**
   - Current: $O(H)$ sweep is acceptable for small heaps.
   - Production: Use generational GC or incremental marking to reduce pause times.

3. **"What about multi-threading?"**
   - Our simulator is single-threaded.
   - Real GCs use "tri-color marking" to allow concurrent execution.

4. **"Why not use a free list?"**
   - For educational clarity. A free list would optimize allocation to $O(1)$ but adds complexity.
