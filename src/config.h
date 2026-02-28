#ifndef SMALLOC_CONFIG_H
#define SMALLOC_CONFIG_H

#include <stddef.h>

typedef struct {
    int injected;
    int verbose;
} smalloc_config_t;

extern smalloc_config_t g_cfg;

void config_init(void);

#endif
