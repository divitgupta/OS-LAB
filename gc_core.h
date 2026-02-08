#ifndef GC_CORE_H
#define GC_CORE_H

#define HEAP_SIZE 20
#define STACK_SIZE 20

struct Block {
    int allocated;
    int marked;
};

extern struct Block heap[HEAP_SIZE];
extern int process_stack[STACK_SIZE];
extern int stack_top;

void init_heap();
int allocate_blocks(int count);
int push_reference_core(int index);
int pop_reference_core();
void mark_phase();
void sweep_phase();
void clear_marks();
void run_gc_core();

#endif
