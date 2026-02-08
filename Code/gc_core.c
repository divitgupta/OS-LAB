#include "gc_core.h"

struct Block heap[HEAP_SIZE];
int process_stack[STACK_SIZE];
int stack_top = -1;

void init_heap() {
    for(int i = 0; i < HEAP_SIZE; i++) {
        heap[i].allocated = 0;
        heap[i].marked = 0;
    }
}

int allocate_blocks(int count) {
    int allocated = 0;
    for(int i = 0; i < HEAP_SIZE && allocated < count; i++) {
        if(!heap[i].allocated) {
            heap[i].allocated = 1;
            allocated++;
        }
    }
    return allocated;
}

int push_reference_core(int index) {
    if(stack_top == STACK_SIZE - 1) return -1;
    if(index < 0 || index >= HEAP_SIZE || !heap[index].allocated) return -2;
    process_stack[++stack_top] = index;
    return 0;
}

int pop_reference_core() {
    if(stack_top == -1) return 0;
    stack_top--;
    return 1;
}

/* ---- GC ---- */

static void mark(int index) {
    if(index < 0 || index >= HEAP_SIZE) return;
    if(heap[index].allocated && !heap[index].marked)
        heap[index].marked = 1;
}

void mark_phase() {
    for(int i = 0; i <= stack_top; i++)
        mark(process_stack[i]);
}

void sweep_phase() {
    for(int i = 0; i < HEAP_SIZE; i++) {
        if(heap[i].allocated && !heap[i].marked)
            heap[i].allocated = 0;
    }
}

void clear_marks() {
    for(int i = 0; i < HEAP_SIZE; i++)
        heap[i].marked = 0;
}

void run_gc_core() {
    mark_phase();
    sweep_phase();
    clear_marks();
}
