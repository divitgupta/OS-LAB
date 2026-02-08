# Exam Q&A: GC Simulator (Viva Prep)

This document is your tactical guide for the "Show me how to change..." questions, focused on the C backend and algorithm logic.

---

## 1. Core Logic & Variables
**Q: How do you increase the system's memory capacity?**
- **Action**: Go to `gc_core.h` or `EL.c` and change `#define HEAP_SIZE 10` (or 20) to a larger number.
- **Logic**: This expands the static array. In the web version, the `dashboard.html` also has a matching constant `HEAP_SIZE` that should be updated to visualize all blocks.

**Q: The stack is filling up too fast. How do you fix it?**
- **Action**: Change `#define STACK_SIZE 5` to `10`.
- **Logic**: This allows more "root" references to be held by the simulated process.

**Q: Where is a "Block" defined?**
- **Action**: Show the `struct Block` in `EL.c` or `gc_core.h`.
  ```c
  struct Block {
      int allocated; // 1 if in use, 0 if free
      int marked;    // 1 if reachable during GC, 0 if garbage
  };
  ```

---

## 2. Mark & Sweep Algorithm
**Q: "Show me the code for the Mark Phase."**
- **Action**: Point to `mark_phase()`:
  ```c
  for(int i = 0; i <= stack_top; i++)
      mark(process_stack[i]);
  ```
- **Explain**: It iterates through the stack (roots) and calls `mark()` on every heap index stored there.

**Q: "How does the Sweep Phase know what to delete?"**
- **Action**: Point to `sweep_phase()`:
  ```c
  if(heap[i].allocated && !heap[i].marked) {
      heap[i].allocated = 0; 
  }
  ```
- **Explain**: It only frees blocks that are `allocated` but were *not* `marked` during the previous phase.

**Q: Why do you need `clear_marks()` at the end?**
- **Answer**: If we don't clear the `marked` flags, the next GC cycle will think everything from the *previous* cycle is still reachable, even if the stack has changed. It resets the state for a fresh start.

---

## 3. Web & API Implementation
**Q: How does the browser get the data from the C program?**
- **Answer**: Using a **REST API**. The C backend (`mongoose_server.c`) listens on a port (e.g., 8000). When the browser asks for `/api/state`, the C code loops through the `heap` array, converts the data into a **JSON string**, and sends it back over HTTP.

**Q: What happens if the server can't allocate enough blocks?**
- **Action**: Show the `allocate()` function logic. It returns the number of blocks actually allocated. If `allocated < count`, the frontend shows a "Heap Full" error message.

---

## 4. DSA & OS Concepts
**Q: Why use an array for the heap instead of a Linked List?**
- **Answer**: 
  1. **Simplicity**: For an EL (Expert Learning) prototype, arrays allow $O(1)$ access to any block by index.
  2. **Predictability**: Static arrays avoid the "Inception" problem of using `malloc` to simulate `malloc`.

**Q: What is a "Memory Leak" in the context of this project?**
- **Answer**: A leak occurs when `heap[i].allocated` is 1, but there is no reference in `process_stack` pointing to it. Without GC, that memory is "lost" forever. Our `sweep_phase` identifies these specific blocks and sets `allocated = 0` to fix the leak.

**Q: Is this "Stop-the-World" or "Incremental" GC?**
- **Answer**: It is **Stop-the-World**. When `run_gc()` is called, the simulation pauses other activities to perform the Mark and Sweep phases sequentially.

---

## 5. Troubleshooting / Common Bugs
**Q: Pushing a reference to an unallocated block.**
- **Action**: In `push_reference()`, find the check `if(!heap[index].allocated)`. 
- **Explain**: We prevent this because a root (stack) should never point to unallocated/raw memory—that would be a "Dangling Pointer" in a real OS.

**Q: Stack Overflow.**
- **Action**: Show `if(stack_top == STACK_SIZE - 1)`.
- **Explain**: This happens if the user tries to add more roots than the system can track.
