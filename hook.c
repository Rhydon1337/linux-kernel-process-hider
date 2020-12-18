#include "hook.h"

static int fh_resolve_hook_address(struct ftrace_hook *hook) {
    hook->address = kallsyms_lookup_name(hook->name);
    if (!hook->address) {
        printk("unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }
    *((unsigned long*) hook->original) = hook->address;
    return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
        struct ftrace_ops *ops, struct pt_regs *regs) {

    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);
    if (!within_module(parent_ip, THIS_MODULE)) {
        regs->ip = (unsigned long) hook->function;
    }
}

int fh_install_hook(struct ftrace_hook *hook) {
    int err = fh_resolve_hook_address(hook);
    if (err) {
        return err;
    }
    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
                    | FTRACE_OPS_FL_RECURSION_SAFE
                    | FTRACE_OPS_FL_IPMODIFY;
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        printk("ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }
    err = register_ftrace_function(&hook->ops);
    if (err) {
        printk("register_ftrace_function() failed: %d\n", err);
        ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
        return err;
    }
    return 0;
}

void fh_remove_hook(struct ftrace_hook *hook) {
    int err = unregister_ftrace_function(&hook->ops);
    if (err) {
        printk("unregister_ftrace_function() failed: %d\n", err);
    }
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) {
        printk("ftrace_set_filter_ip() failed: %d\n", err);
    }
}