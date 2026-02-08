#include "mongoose.h"
#include "gc_core.h"
#include <string.h>
#include <stdlib.h>

static const char *s_http_addr = "http://localhost:8000";

// Helper to send JSON response
static void send_json(struct mg_connection *c, const char *json) {
  mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json);
}

// Helper to construct state JSON
static void get_state_json(char *buf, size_t len) {
    char heap_json[1024] = "[";
    for (int i = 0; i < HEAP_SIZE; i++) {
        char block[64];
        snprintf(block, sizeof(block), "{\"allocated\":%d,\"marked\":%d}%s", 
                 heap[i].allocated, heap[i].marked, (i < HEAP_SIZE - 1) ? "," : "");
        strcat(heap_json, block);
    }
    strcat(heap_json, "]");

    char stack_json[512] = "[";
    for (int i = 0; i <= stack_top; i++) {
        char item[16];
        snprintf(item, sizeof(item), "%d%s", process_stack[i], (i < stack_top) ? "," : "");
        strcat(stack_json, item);
    }
    strcat(stack_json, "]");

    snprintf(buf, len, "{\"heap\":%s,\"stack\":%s}", heap_json, stack_json);
}

// Helper to match URI
static int match_uri(struct mg_http_message *hm, const char *uri) {
    if (hm->uri.len == strlen(uri) && memcmp(hm->uri.buf, uri, hm->uri.len) == 0) {
        return 1;
    }
    return 0;
}

// Helper to get long from (simple) JSON body
static long json_get_long(struct mg_str json, const char *key) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    
    // We need a null-terminated string for strstr
    char *body = (char *)malloc(json.len + 1);
    if (!body) return 0;
    memcpy(body, json.buf, json.len);
    body[json.len] = '\0';
    
    char *p = strstr(body, search);
    long val = 0;
    if (p) {
        val = strtol(p + strlen(search), NULL, 10);
    }
    
    free(body);
    return val;
}

static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (match_uri(hm, "/api/state")) {
        char json[2048];
        get_state_json(json, sizeof(json));
        send_json(c, json);
    } else if (match_uri(hm, "/api/allocate")) {
        long count = json_get_long(hm->body, "count");
        if (count <= 0) count = 1; // Default
        
        int allocated_count = allocate_blocks((int)count);
        
        // Custom JSON construction to include allocation info
        char heap_json[1024] = "[";
        for (int i = 0; i < HEAP_SIZE; i++) {
            char block[64];
            snprintf(block, sizeof(block), "{\"allocated\":%d,\"marked\":%d}%s", 
                     heap[i].allocated, heap[i].marked, (i < HEAP_SIZE - 1) ? "," : "");
            strcat(heap_json, block);
        }
        strcat(heap_json, "]");

        char stack_json[512] = "[";
        for (int i = 0; i <= stack_top; i++) {
            char item[16];
            snprintf(item, sizeof(item), "%d%s", process_stack[i], (i < stack_top) ? "," : "");
            strcat(stack_json, item);
        }
        strcat(stack_json, "]");
        
        char json[2048];
        snprintf(json, sizeof(json), "{\"allocated_count\":%d, \"requested_count\":%ld, \"heap\":%s,\"stack\":%s}", 
                 allocated_count, count, heap_json, stack_json);
        
        send_json(c, json);
    } else if (match_uri(hm, "/api/push")) {
        long idx = json_get_long(hm->body, "index");
        
        int result = push_reference_core((int)idx);
        
        if (result == -1) {
            mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Stack Overflow!\"}");
        } else if (result == -2) {
             mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Invalid heap reference!\"}");
        } else {
            char json[2048];
            get_state_json(json, sizeof(json));
            send_json(c, json);
        }
    } else if (match_uri(hm, "/api/pop")) {
        pop_reference_core();
        char json[2048];
        get_state_json(json, sizeof(json));
        send_json(c, json);
    } else if (match_uri(hm, "/api/mark")) {
        mark_phase();
        char json[2048];
        get_state_json(json, sizeof(json));
        send_json(c, json);
    } else if (match_uri(hm, "/api/sweep")) {
        sweep_phase();
        char json[2048];
        get_state_json(json, sizeof(json));
        send_json(c, json);
    } else if (match_uri(hm, "/api/clear_marks")) {
        clear_marks();
        char json[2048];
        get_state_json(json, sizeof(json));
        send_json(c, json);
    } else if (match_uri(hm, "/api/reset")) {
        init_heap();
        stack_top = -1; 
        char json[2048];
        get_state_json(json, sizeof(json));
        send_json(c, json);
    } else {
        struct mg_http_serve_opts opts = {.root_dir = "."};
        mg_http_serve_dir(c, hm, &opts);
    }
  }
}

int main(void) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_http_addr, fn, NULL);
  
  // Initialize GC core
  init_heap();

  printf("Starting Mongoose server on %s\n", s_http_addr);
  // Auto-open browser
  #ifdef __APPLE__
    system("open http://localhost:8000/dashboard.html");
  #elif _WIN32
    system("start http://localhost:8000/dashboard.html");
  #else
    // system("xdg-open http://localhost:8000/dashboard.html");
  #endif

  for (;;) mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);
  return 0;
}
