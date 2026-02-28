#include <stdlib.h>
#include <pthread.h>
#include "config.h"
#include "debug.h"

smalloc_config_t g_cfg = {0};

void config_init(void) {
    if (getenv("SMALLOC_INJECTED")) {
        char* msg = "WARNING! You are using smalloc.\n";
        write(1, msg, safe_strlen(msg));
        g_cfg.injected = 1;
    }

    if (getenv("SMALLOC_VERBOSE")) {
        char* msg = "Logs enabled.\n";
        write(1, msg, safe_strlen(msg));
        g_cfg.verbose = 1;
    }
}
