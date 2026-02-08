# The Master Guide: OS Garbage Collection (Mark & Sweep) Simulator

This document is the "Source of Truth" for your project. It combines the technical architecture, implementation logic (C backend + Web frontend), and viva preparation into one comprehensive guide.

---

## 1. Project Overview & Mission
**Goal**: To build an interactive simulation of an **Operating System Memory Manager** that implements the **Mark-and-Sweep Garbage Collection** algorithm.
**Key Objective**: To visualize how a system identifies "Live" vs. "Dead" memory and reclaims space without manual programmer intervention.

---

## 2. Technical Roadmap: Data Structures (The "How")

### A. Core Memory Components
| Feature | Implementation | DSA / OS Concept | Role |
| :--- | :--- | :--- | :--- |
| **Physical Heap** | `struct Block heap[10/20]` | **Static Array** | Fixed-size memory pool where objects are stored. |
| **Object State** | `allocated`, `marked` flags | **Metadata Bitmask** | Tracks whether memory is in use and if it has been "visited". |
| **Process Stack** | `int process_stack[5/20]` | **Linear Stack** | Stores the **Roots**: handles that the application currently "owns". |
| **API Backend** | `Mongoose Server` | **Event-Driven Networking** | Bridges the C-logic to the Web interface via JSON. |

### B. Logical flow
1. **Allocation**: Application requests memory $\rightarrow$ System scans Heap for `allocated == 0`.
2. **Referencing**: Application stores a pointer $\rightarrow$ Index pushed to `process_stack`.
3. **Trigger**: System memory runs low $\rightarrow$ Start **Mark & Sweep**.

---

## 3. Algorithmic Deep Dive (The "Mark & Sweep")

### A. The Mark Phase (Tracing)
We start from known **Roots** (the Stack) and follow arrows to reachable objects.
- **Formula**: `if (exists in Stack) reachable = true`.
- **In our code**: 
  ```c
  for(i = 0; i <= stack_top; i++) {
      heap[process_stack[i]].marked = 1;
  }
  ```
- **Complexity**: $O(R)$ where $R$ is the number of root references.

### B. The Sweep Phase (Reclaiming)
We scan the entire memory surface. Anything not "marked" is "garbage".
- **Formula**: `if (allocated == 1 && marked == 0) free()`.
- **In our code**:
  ```c
  for(i = 0; i < HEAP_SIZE; i++) {
      if(heap[i].allocated && !heap[i].marked) {
          heap[i].allocated = 0; // RECLAIMED
      }
  }
  ```
- **Complexity**: $O(H)$ where $H$ is the total Heap size.

---

## 4. Frontend-Backend Architecture

### The Communication Loop
- **Backend (`mongoose_server.c`)**: Maintains the global `heap` and `stack` arrays. It doesn't store state on the disk, only in active memory (static variables).
- **Communication Protocol**: JSON over HTTP.
- **Frontend (`dashboard.html`)**: Dynamic DOM updates. It color-codes the heap grid:
  - **Gray**: Free
  - **Blue/Slate**: Allocated
  - **Amber**: Marked (Reachable)

---

## 5. Viva Preparation: The "Defense" Guide

### "Show me how to change..."
- **Heap Size**: Modify `#define HEAP_SIZE` in `gc_core.h` or `EL.c`. 
- **Stack Limit**: Change `#define STACK_SIZE`. If you push more than this, call `Stack Overflow`.
- **GC Trigger Policy**: In `server.c`, we currently trigger GC manually via a button. You could add logic in `allocate()` to call `run_gc()` automatically if `allocated_count > 80%`.

### Critical Comparisons
- **Mark-Sweep vs. Reference Counting**: 
  - *Ref Counting* suffers from **Cycles** (Object A points to B, B points to A, but no one points to them). 
  - *Mark-Sweep* solves cycles because it only cares about what is reachable from the **Roots**.
- **Fragmentation**: Our simple simulator handles fragmentation by just searching for the next free block. A more advanced version would use **Mark-Compact** to squeeze all live objects together.

---

## 6. How to Run & Present

1.  **The Hook**: "This system prevents memory leaks by automatically reclaiming unreachable data using the Mark-and-Sweep algorithm."
2.  **The Master Sequence**:
    -   **Step 1**: Allocate 5 blocks.
    -   **Step 2**: Push references to blocks 0 and 1.
    -   **Step 3**: Show the Heap (all 5 allocated).
    -   **Step 4**: Run GC.
    -   **Result**: Blocks 2, 3, and 4 are reclaimed (Free), while 0 and 1 stay (Live).
3.  **The Conclusion**: "Visualizing these phases helps software engineers understand why languages like Java or Python have a brief 'stop-the-world' pause during collection."
