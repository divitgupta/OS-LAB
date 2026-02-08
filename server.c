#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "gc_core.h"

#define PORT 8080

static void send_state(int client) {
    char response[4096];
    char json[2048];

    strcpy(json, "{ \"heap\": [");

    for (int i = 0; i < HEAP_SIZE; i++) {
        char buf[64];
        sprintf(buf,
            "{ \"allocated\": %d, \"marked\": %d }%s",
            heap[i].allocated,
            heap[i].marked,
            (i < HEAP_SIZE - 1 ? "," : "")
        );
        strcat(json, buf);
    }

    strcat(json, "], \"stack\": [");

    for (int i = 0; i <= stack_top; i++) {
        char buf[16];
        sprintf(buf, "%d%s",
            process_stack[i],
            (i < stack_top ? "," : "")
        );
        strcat(json, buf);
    }

    strcat(json, "] }");

    sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %ld\r\n\r\n"
        "%s",
        strlen(json), json);

    write(client, response, strlen(response));
}

int main() {
    int server_fd, client;
    struct sockaddr_in addr;
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    /* Initial demo state */
    init_heap();
    allocate_blocks(4);
    push_reference_core(1);
    push_reference_core(3);

    printf("GC backend running on http://localhost:%d\n", PORT);

    while (1) {
        client = accept(server_fd, NULL, NULL);
        read(client, buffer, sizeof(buffer));

        if (strncmp(buffer, "GET /gc", 7) == 0) {
            run_gc_core();
            send_state(client);
        }
        else if (strncmp(buffer, "GET /state", 10) == 0) {
            send_state(client);
        }
        else {
            write(client,
                "HTTP/1.1 404 Not Found\r\n\r\n",
                26);
        }

        close(client);
    }

    return 0;
}
