# GC Methodology Flowchart

This is a concise, memorable flowchart showing the complete Mark-and-Sweep garbage collection methodology.

---

## System Execution Flow (Complete Methodology)

```mermaid
graph TD
    S([Start]) --> Init[Initialize Heap & Stack Arrays]
    Init --> L{User Action?}
    
    L -- "Allocate N blocks" --> Scan[Linear Scan: Find Free Blocks]
    Scan --> SetAlloc[Set allocated = 1]
    SetAlloc --> L
    
    L -- "Push Reference" --> Valid{Valid Index & Allocated?}
    Valid -- "Yes" --> AddStack[Stack: Push heap index]
    Valid -- "No" --> Error[Return Error Code]
    AddStack --> L
    Error --> L
    
    L -- "Run GC" --> Mark[Mark Phase: Trace from Stack]
    Mark --> MarkLoop[For each Stack ref: heap.marked = 1]
    MarkLoop --> Sweep[Sweep Phase: Scan Heap]
    Sweep --> SweepLoop[If allocated && !marked: FREE]
    SweepLoop --> Clear[Clear Marks: Reset all flags]
    Clear --> L
    
    L -- "Exit" --> E([End])
```

---

## Key Algorithm Details

### Mark Phase (Reachability Tracing)
- **Input**: Process Stack (Root Set)
- **Process**: `for(i=0; i<=stack_top; i++) heap[stack[i]].marked = 1`
- **Complexity**: $O(R)$ where R = number of roots

### Sweep Phase (Memory Reclamation)
- **Process**: `for(i=0; i<HEAP_SIZE; i++) if(allocated && !marked) allocated=0`
- **Complexity**: $O(H)$ where H = heap size
- **Result**: All unreachable blocks freed

### Total GC Complexity
- **Time**: $O(R + H)$ = Linear
- **Space**: $O(1)$ = In-place algorithm

---

## Data Structure Summary

| Structure | Implementation | Purpose |
|:----------|:--------------|:--------|
| **Heap** | `Block heap[HEAP_SIZE]` | Static array of memory blocks |
| **Stack** | `int stack[STACK_SIZE]` | Root references (live objects) |
| **Block** | `{allocated, marked}` | 2 flags per memory block |

---

## Why Mark-and-Sweep?

**Problem Solved**: Circular references (A→B, B→A, but no root→A)
- **Reference Counting**: Would leak (counts never reach 0)
- **Mark-and-Sweep**: Correctly identifies as garbage ✓

**Trade-off**: "Stop-the-world" pause during GC
- **Solution in Production**: Incremental/Concurrent marking (Java G1, Python)
