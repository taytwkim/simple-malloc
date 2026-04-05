#include <stdlib.h>
#include <pthread.h>
#include "config.h"
#include "debug.h"

tkmalloc_config_t g_cfg = {0};  // zero-initializes env var

void config_init(void) {
    if (getenv("TKMALLOC_INJECTED")) {
        char* msg = "WARNING! You are using tkmalloc.\n";
        ignore_write_result(write(1, msg, safe_strlen(msg)));
        g_cfg.injected = 1;
    }

    if (getenv("TKMALLOC_VERBOSE")) {
        char* msg = "Logs enabled.\n";
        ignore_write_result(write(1, msg, safe_strlen(msg)));
        g_cfg.verbose = 1;
    }
    
    if (getenv("TKMALLOC_DISABLE_ARENAS")) {
        if (g_cfg.verbose) {
            char* msg = "Per-thread arenas disabled.\n";
            ignore_write_result(write(1, msg, safe_strlen(msg)));
        }
        g_cfg.disable_arenas = 1;
    }

    if (getenv("TKMALLOC_DISABLE_TCACHE")) {
        if (g_cfg.verbose) {
            char* msg = "Tcache disabled.\n";
            ignore_write_result(write(1, msg, safe_strlen(msg)));
        }
        g_cfg.disable_tcache = 1;
    }
}
