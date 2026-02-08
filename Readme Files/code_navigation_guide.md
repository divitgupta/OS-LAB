# Code Navigation Guide: Where to Find Everything

This guide helps you quickly jump to the most important parts of the code when an examiner asks "Show me where you implemented X."

---

## 1. Core GC Logic: `gc_core.c` & `gc_core.h`

### A. Data Structures (The Foundation)
- **`gc_core.h` Lines 1-10**: Look for `struct Block` definition.
    - *Tip*: Explain that each block has two flags: `allocated` (in use) and `marked` (reachable).
- **`gc_core.c` Line 3**: `struct Block heap[HEAP_SIZE]` - The main memory pool.
- **`gc_core.c` Line 4**: `int process_stack[STACK_SIZE]` - The root references.
- **`gc_core.c` Line 5**: `int stack_top = -1` - Stack pointer (-1 means empty).

### B. Core Algorithms (The Logic)
- **`gc_core.c` Line 7-12**: `void init_heap()`
    - Initializes all blocks to free state. Point here for system setup.
- **`gc_core.c` Line 14-23**: `int allocate_blocks(int count)`
    - The **allocation algorithm**. Scans heap for free blocks.
- **`gc_core.c` Line 25-30**: `int push_reference_core(int index)`
    - **Stack management**. Returns error codes: -1 (overflow), -2 (invalid ref).
- **`gc_core.c` Line 40-44**: `static void mark(int index)`
    - The **Mark helper**. Sets the marked flag on a single block.
- **`gc_core.c` Line 46-49**: `void mark_phase()`
    - **Mark Phase**. Iterates through stack and marks all referenced blocks.
- **`gc_core.c` Line 51-56**: `void sweep_phase()`
    - **Sweep Phase**. Frees all allocated but unmarked blocks.
- **`gc_core.c` Line 58-61**: `void clear_marks()`
    - **Cleanup**. Resets all marked flags for the next GC cycle.
- **`gc_core.c` Line 63-67**: `void run_gc_core()`
    - **Complete GC**. Orchestrates mark → sweep → clear sequence.

---

## 2. Web Server Backend: `mongoose_server.c`

### A. API Endpoints (The Interface)
- **Line 68-71**: `GET /api/state`
    - Returns current heap and stack state as JSON.
- **Line 72-100**: `POST /api/allocate`
    - Allocates N blocks and returns success count.
- **Line 101-114**: `POST /api/push`
    - Pushes a reference to stack with validation (returns 400 on error).
- **Line 115-119**: `POST /api/pop`
    - Pops top reference from stack.
- **Line 120-124**: `POST /api/mark`
    - Triggers mark phase only.
- **Line 125-129**: `POST /api/sweep`
    - Triggers sweep phase only.
- **Line 130-134**: `POST /api/clear_marks`
    - Clears all mark flags.
- **Line 135-140**: `POST /api/reset`
    - Resets entire system (heap + stack).

### B. Helper Functions
- **Line 9-11**: `send_json(...)`
    - Utility to send JSON responses with proper headers.
- **Line 14-33**: `get_state_json(...)`
    - Constructs JSON from heap and stack arrays (manual serialization).
- **Line 44-62**: `json_get_long(...)`
    - Parses JSON request body to extract integer values.

---

## 3. Frontend: `dashboard.html`

### A. Visualization Logic (JavaScript)
- **Line 471-484**: `async function fetchState()`
    - Fetches current state from backend and updates UI.
- **Line 490-507**: `function renderHeap()`
    - Renders the heap grid with color-coded blocks (free/allocated/marked).
- **Line 509-523**: `function renderStack()`
    - Displays stack contents as a vertical list.
- **Line 543-566**: `async function allocateBlocks()`
    - Sends allocation request and handles partial allocation errors.
- **Line 568-594**: `async function pushReference()`
    - Validates and pushes references with error handling.
- **Line 622-636**: `async function markPhase()`
    - Triggers mark phase and updates visualization.
- **Line 638-654**: `async function sweepPhase()`
    - Triggers sweep phase and shows garbage collection.
- **Line 671-680**: `async function runGC()`
    - **Full GC cycle** with visual pauses between phases.

### B. UI Components
- **Line 377-379**: Heap Grid (`#heapGrid`)
    - 5x4 grid (for 20 blocks) or 5x2 (for 10 blocks).
- **Line 422-424**: Stack Display (`#stackDisplay`)
    - Vertical list showing references from top to bottom.
- **Line 429**: Message Area (`#messageArea`)
    - Inline status messages (success/error/info).

---

## 4. CLI Version: `EL.c`

### A. Main Loop
- **Line 129-156**: `int main()`
    - Menu-driven interface with 7 options.
- **Line 42-59**: `void allocate()`
    - Interactive allocation with user input.
- **Line 61-76**: `void push_reference()`
    - Interactive push with validation.
- **Line 117-127**: `void run_gc()`
    - Complete GC with console output for each phase.

---

## 5. Cheat Sheet for "Show Me" Questions

| If they ask: | Go to: | Look for: |
| :--- | :--- | :--- |
| "How do you store the heap?" | `gc_core.c` | `struct Block heap[HEAP_SIZE]` |
| "Where is the stack?" | `gc_core.c` | `int process_stack[STACK_SIZE]` |
| "Show the Mark algorithm." | `gc_core.c` | `mark_phase()` function |
| "Show the Sweep algorithm." | `gc_core.c` | `sweep_phase()` function |
| "How do you validate references?" | `gc_core.c` | `push_reference_core()` error checks |
| "Where is the API routing?" | `mongoose_server.c` | `fn()` event handler |
| "How does the UI update?" | `dashboard.html` | `fetchState()` and `renderHeap()` |
| "Show me error handling." | `mongoose_server.c` | Lines 106-109 (push validation) |
| "How is JSON constructed?" | `mongoose_server.c` | `get_state_json()` manual serialization |
| "Where is the full GC cycle?" | `gc_core.c` | `run_gc_core()` |

---

## 6. File Structure Overview

```
OS_LAB_EL/
├── EL.c                    # CLI version (standalone)
├── test/
│   ├── gc_core.c          # Core GC logic
│   ├── gc_core.h          # Header with constants & structs
│   ├── mongoose_server.c  # Web server with API
│   ├── mongoose.c/.h      # Mongoose library
│   ├── dashboard.html     # Frontend UI
│   └── server             # Compiled binary
└── GC_Project_Documentation/
    ├── ULTIMATE_GC_MASTER_GUIDE.md
    ├── viva_exam_qa.md
    └── algorithm_flowchart.md
```
