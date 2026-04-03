#ifndef TKMALLOC_CONFIG_H
#define TKMALLOC_CONFIG_H

#include <stddef.h>

typedef struct {
    int injected;
    int verbose;
    int disable_tcache;
} tkmalloc_config_t;

extern tkmalloc_config_t g_cfg;

void config_init(void);

#endif
