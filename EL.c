#include <stdio.h>

#define HEAP_SIZE 10
#define STACK_SIZE 5

struct Block {
    int allocated;
    int marked;
};

struct Block heap[HEAP_SIZE];
int process_stack[STACK_SIZE];
int stack_top = -1;

void init_heap() {
    for(int i = 0; i < HEAP_SIZE; i++) {
        heap[i].allocated = 0; 
        heap[i].marked = 0;
    }
}

void display_heap() {
    printf("\nHeap Status:\n");
    printf("Index | Allocated | Marked\n");
    for(int i = 0; i < HEAP_SIZE; i++) {
        printf("  %d   |     %d     |   %d\n",
                i, heap[i].allocated, heap[i].marked);
    }
}

void display_stack() {
    printf("\nProcess Stack (Roots): ");
    if(stack_top == -1) {
        printf("Empty");
    } else {
        for(int i = 0; i <= stack_top; i++)
            printf("%d ", process_stack[i]);
    }
    printf("\n");
}

void allocate() {
    int count, allocated = 0;

    printf("Enter number of memory blocks to allocate: ");
    scanf("%d", &count);

    for(int i = 0; i < HEAP_SIZE && allocated < count; i++) {
        if(!heap[i].allocated) {
            heap[i].allocated = 1;
            printf("Allocated memory block at index %d\n", i);
            allocated++;
        }
    }

    if(allocated < count) {
        printf("Heap Full! Allocation failed.\n");
    }
}

void push_reference() {
    int index;
    if(stack_top == STACK_SIZE - 1) {
        printf("Stack Overflow!\n");
        return;
    }
    printf("Enter heap index to reference: ");
    scanf("%d", &index);

    if(index < 0 || index >= HEAP_SIZE || !heap[index].allocated) {
        printf("Invalid heap reference!\n");
        return;
    }
    process_stack[++stack_top] = index;
    printf("Reference added to stack.\n");
}

void pop_reference() {
    if(stack_top == -1) {
        printf("Stack Underflow!\n");
        return;
    }
    printf("Reference %d removed from stack.\n", process_stack[stack_top]);
    stack_top--;
}

void mark(int index) {
    if(index < 0 || index >= HEAP_SIZE)
        return;

    if(heap[index].allocated && !heap[index].marked) {
        heap[index].marked = 1;
    }
}

void mark_phase() {
    for(int i = 0; i <= stack_top; i++)
        mark(process_stack[i]);
}

void sweep_phase() {
    for(int i = 0; i < HEAP_SIZE; i++) {
        if(heap[i].allocated && !heap[i].marked) {
            heap[i].allocated = 0;
            printf("Garbage Collected block %d\n", i);
        }
    }
}

void clear_marks() {
    for(int i = 0; i < HEAP_SIZE; i++) {
        heap[i].marked = 0;
    }
    printf("All marks cleared for next GC cycle.\n");
}

void run_gc() {
    printf("\nRunning Mark & Sweep Garbage Collection...\n");
    printf("\n---Mark Phase---\n");
    mark_phase();
    display_heap();
    printf("\n---Sweep Phase---\n");
    sweep_phase();
    display_heap();
    clear_marks();
    printf("Garbage Collection completed.\n");
}

int main() {
    int choice;
    init_heap();

    while(1) {
        printf("\n--- OS Garbage Collection Prototype ---\n");
        printf("1. Allocate Memory\n");
        printf("2. Push Reference (Process Stack)\n");
        printf("3. Pop Reference\n");
        printf("4. Run Garbage Collector\n");
        printf("5. Display Heap\n");
        printf("6. Display Process Stack\n");
        printf("7. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: allocate(); break;
            case 2: push_reference(); break;
            case 3: pop_reference(); break;
            case 4: run_gc(); break;
            case 5: display_heap(); break;
            case 6: display_stack(); break;
            case 7: return 0;
            default: printf("Invalid choice!\n");
        }
    }
}
