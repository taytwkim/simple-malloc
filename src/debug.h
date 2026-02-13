#ifndef SMALLOC_DEBUG_H
#define SMALLOC_DEBUG_H

#include <unistd.h>

#define SMALLOC_VERBOSE 1 

static inline size_t safe_strlen(const char *s) {
    size_t len = 0;
    while (s && s[len]) len++;
    return len;
}

// Usage: safe_log_msg("smalloc: running global_init\n");
static inline void safe_log_msg(const char *msg) {
    if (SMALLOC_VERBOSE && msg) {
        write(STDERR_FILENO, msg, safe_strlen(msg));
    }
}

// Usage: safe_log_ptr("smalloc: my_malloc returning ", p);
static inline void safe_log_ptr(const char *msg, void *ptr) {
    if (!SMALLOC_VERBOSE) return;

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