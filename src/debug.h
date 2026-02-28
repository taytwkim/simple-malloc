#ifndef SMALLOC_DEBUG_H
#define SMALLOC_DEBUG_H

#include <unistd.h>
#include "config.h"

static inline size_t safe_strlen(const char *s) {
    size_t len = 0;
    while (s && s[len]) len++;
    return len;
}

static inline void safe_log_msg(const char *msg) {
    if (g_cfg.verbose && msg) {
        write(1, msg, safe_strlen(msg));
    }
}

static inline void safe_log_ptr(const char *msg, void *ptr) {
    if (!g_cfg.verbose) return;

    // Print the message first
    safe_log_msg(msg);

    // Convert pointer to hex manually
    unsigned long n = (unsigned long)ptr;
    char buf[19]; 
    buf[0] = '0'; buf[1] = 'x';
    buf[18] = '\n'; // Add newline at the end

    for (int i = 17; i >= 2; i--) {
        int digit = n % 16;
        buf[i] = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
        n /= 16;
    }
    
    write(STDERR_FILENO, buf, 19);
}

#endif