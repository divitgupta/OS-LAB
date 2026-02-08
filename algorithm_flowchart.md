# Algorithm Flowcharts: Mark & Sweep

This document visualizes the logic flows of the project using Mermaid diagrams.

## 1. Overall System Lifecycle
This flowchart describes the interaction between the User, the Process Stack, and the Garbage Collector.

```mermaid
graph TD
    Start([Start Simulation]) --> Init[Initialize Heap & Stack]
    Init --> UserAction{User Action?}
    
    UserAction -- Allocate --> Alloc[Find Free block in Heap]
    Alloc --> UserAction
    
    UserAction -- Push Ref --> Push[Add Heap Index to Stack]
    Push --> UserAction
    
    UserAction -- Run GC --> GC[Start GC Cycle]
    
    subgraph GC_Cycle [Garbage Collection Cycle]
    GC --> Mark[Phase 1: Mark Reachable Blocks]
    Mark --> Sweep[Phase 2: Sweep Unreachable Blocks]
    Sweep --> Clear[Phase 3: Clear all Mark Flags]
    end
    
    Clear --> UserAction
    UserAction -- Exit --> End([End Simulation])
```

---

## 2. Allocation Algorithm Flow
Detailed flow of how memory blocks are allocated.

```mermaid
flowchart TD
    Start([Allocate Request: count N]) --> Init[Set allocated = 0, i = 0]
    Init --> Loop{i < HEAP_SIZE AND allocated < N?}
    
    Loop -- No --> Return[Return allocated count]
    Loop -- Yes --> Check{Is heap i free?}
    
    Check -- No --> Increment1[i++]
    Increment1 --> Loop
    
    Check -- Yes --> Allocate[Set heap i allocated = 1]
    Allocate --> Count[allocated++]
    Count --> Increment2[i++]
    Increment2 --> Loop
    
    Return --> Success{allocated == N?}
    Success -- Yes --> Full[Success: All blocks allocated]
    Success -- No --> Partial[Partial: Heap Full]
```

---

## 3. Push Reference Validation Flow
How the system validates and adds references to the stack.

```mermaid
flowchart TD
    Start([Push Reference: index]) --> CheckStack{stack_top == STACK_SIZE - 1?}
    
    CheckStack -- Yes --> Overflow[Error: Stack Overflow]
    CheckStack -- No --> CheckRange{index >= 0 AND index < HEAP_SIZE?}
    
    CheckRange -- No --> InvalidIndex[Error: Invalid Index]
    CheckRange -- Yes --> CheckAlloc{heap index allocated == 1?}
    
    CheckAlloc -- No --> InvalidRef[Error: Invalid Reference]
    CheckAlloc -- Yes --> Push[stack_top++<br/>process_stack stack_top = index]
    
    Push --> Success[Success: Reference Added]
```

---

## 4. Mark Phase Deep Dive
How the system identifies "Live" memory.

```mermaid
flowchart LR
    subgraph Stack ["Process Stack (Roots)"]
    R1[Ref to Index 1]
    R2[Ref to Index 4]
    end
    
    subgraph Heap [Memory Heap]
    B0[Block 0<br/>allocated]
    B1[Block 1<br/>allocated]
    B2[Block 2<br/>allocated]
    B3[Block 3<br/>allocated]
    B4[Block 4<br/>allocated]
    B5[Block 5<br/>free]
    end
    
    R1 -- "marks" --> B1
    R2 -- "marks" --> B4
    
    classDef marked fill:#f59e0b,stroke:#d97706,color:white;
    classDef allocated fill:#475569,stroke:#64748b,color:white;
    classDef free fill:#0f172a,stroke:#334155,color:#94a3b8;
    
    class B1,B4 marked;
    class B0,B2,B3 allocated;
    class B5 free;
```

---

## 5. Sweep Phase Logic
The decision-making process for every block in memory during the sweep.

```mermaid
flowchart TD
    Scan[Start Heap Scan at i=0] --> CheckAlloc{Is block i allocated?}
    
    CheckAlloc -- No --> Next[Increment i]
    CheckAlloc -- Yes --> CheckMarked{Is block i marked?}
    
    CheckMarked -- Yes --> Next
    CheckMarked -- No --> Free["Set allocated = 0<br/>(Free Memory - Garbage Collected)"]
    
    Free --> Next
    Next --> EndCheck{i == HEAP_SIZE?}
    
    EndCheck -- No --> CheckAlloc
    EndCheck -- Yes --> Finish([Scan Complete])
```

---

## 6. Complete GC Execution Sequence
Step-by-step execution of a full garbage collection cycle.

```mermaid
sequenceDiagram
    participant User
    participant Frontend
    participant API
    participant GC as GC Core
    participant Heap
    participant Stack

    User->>Frontend: Click "Run GC"
    Frontend->>API: POST /api/mark
    API->>GC: mark_phase()
    GC->>Stack: Read all references
    
    loop For each reference in Stack
        Stack->>GC: Return heap index
        GC->>Heap: Set marked = 1
    end
    
    API-->>Frontend: Return updated state
    Frontend->>User: Show marked blocks (amber)
    
    Note over User,Stack: Brief pause for visualization
    
    Frontend->>API: POST /api/sweep
    API->>GC: sweep_phase()
    
    loop For each block in Heap
        GC->>Heap: Check allocated & marked
        alt Not marked
            GC->>Heap: Set allocated = 0 (FREE)
        else Marked
            GC->>Heap: Keep allocated
        end
    end
    
    API->>GC: clear_marks()
    GC->>Heap: Reset all marked flags to 0
    API-->>Frontend: Return final state
    Frontend->>User: Show reclaimed blocks
```

---

## 7. Frontend-Backend Architecture
How the web interface communicates with the C backend.

```mermaid
graph TB
    subgraph Browser ["Browser (Frontend)"]
        UI[Dashboard HTML/JS]
        Heap_Display[Heap Grid Visualization]
        Stack_Display[Stack List Display]
    end
    
    subgraph Server ["Mongoose Server (Backend)"]
        HTTP[HTTP Handler]
        Router[API Router]
        
        subgraph Endpoints [API Endpoints]
            State[GET /api/state]
            Alloc[POST /api/allocate]
            Push[POST /api/push]
            Pop[POST /api/pop]
            Mark[POST /api/mark]
            Sweep[POST /api/sweep]
            Clear[POST /api/clear_marks]
            Reset[POST /api/reset]
        end
    end
    
    subgraph Core ["GC Core (C)"]
        HeapMem[heap array]
        StackMem[process_stack array]
        GC_Logic[GC Functions]
    end
    
    UI -->|HTTP Request| HTTP
    HTTP --> Router
    Router --> Endpoints
    Endpoints --> GC_Logic
    GC_Logic --> HeapMem
    GC_Logic --> StackMem
    
    GC_Logic -->|JSON Response| Router
    Router -->|HTTP Response| UI
    UI --> Heap_Display
    UI --> Stack_Display
```

---

## 8. Memory Block State Machine
Lifecycle of a single memory block.

```mermaid
stateDiagram-v2
    [*] --> Free: System Init
    Free --> Allocated: allocate_blocks()
    Allocated --> Marked: mark_phase()<br/>(if referenced)
    Allocated --> Free: sweep_phase()<br/>(if NOT marked)
    Marked --> Allocated: clear_marks()
    Allocated --> Free: User pops all refs<br/>+ GC runs
```

---

## 9. Why Mark-and-Sweep? (The Circular Reference Problem)
This diagram shows how we handle a circular reference that is no longer reachable from the roots.

```mermaid
graph TD
    subgraph Roots [Application Roots]
    Stack[Process Stack]
    end
    
    subgraph Reachable ["Live Set (Reachable)"]
    Rooted[Object A<br/>marked = 1]
    end
    
    subgraph Garbage ["Garbage Set (Unreachable)"]
    ObjB[Object B<br/>marked = 0] <--> ObjC[Object C<br/>marked = 0]
    end
    
    Stack --> Rooted
    
    style ObjB fill:#ef4444,stroke:#dc2626,color:white
    style ObjC fill:#ef4444,stroke:#dc2626,color:white
    style Rooted fill:#10b981,stroke:#059669,color:white
    
    note1[Note: B and C point to each other<br/>but are NOT reachable from Stack.<br/>Mark & Sweep will RECLAIM them.]
```

---

## 10. Comparison: Mark-Sweep vs Reference Counting

```mermaid
graph LR
    subgraph MS ["Mark & Sweep"]
        MS_Start[Scan from Roots] --> MS_Mark[Mark all reachable]
        MS_Mark --> MS_Sweep[Free unmarked]
        MS_Sweep --> MS_Result[Handles Cycles ✓]
    end
    
    subgraph RC ["Reference Counting"]
        RC_Start[Track ref count] --> RC_Inc[Increment on reference]
        RC_Inc --> RC_Dec[Decrement on dereference]
        RC_Dec --> RC_Check{Count == 0?}
        RC_Check -- Yes --> RC_Free[Free memory]
        RC_Check -- No --> RC_Keep[Keep allocated]
        RC_Keep --> RC_Problem[Cycles cause leaks ✗]
    end
    
    style MS_Result fill:#10b981,stroke:#059669,color:white
    style RC_Problem fill:#ef4444,stroke:#dc2626,color:white
```
