# linux-kernel-process-hider
## TL;DR
Process hider for Linux systems using a kernel agent.

Tested on linux kernel versions: 4.19.91, 5.4.0-48-generic.

The compiled kernel options must include: CONFIG_KALLSYMS, CONFIG_HAVE_DYNAMIC_FTRACE, CONFIG_FTRACE, CONFIG_FUNCTION_TRACER. On most distros these options are default included.

## How it works
When we do "ls" on /proc the function proc_pid_readdir from procfs is called, in this function procfs fill the pid sub directory (/proc/1 or /proc/2).

How proc_pid_readdir function finds all running processes?

It's using this magical function find_ge_pid, this function finds the first pid that is greater than or equal to the given one.

From linux kernel sources documentation: 
proc_pid_readdir is used by proc to find the first pid that is greater than or equal to nr.

We can infer that although this function is seperated from procfs, it was created for him and used only by him (I validated it).

I decided to hook this function and when the next pid is a pid that we need to hide we will just skip the pid to next one.

[More details about the hook can be found here](https://github.com/Rhydon1337/linux-kernel-ata-sniffer).

## Limitations
1. It won't stop someone inside the kernel that will try to enumare it
2. if we tried to hide {pid}, someone still able to access it directly from /proc/{pid}

## Usage
cd linux-kernel-process-hider

make

insmod process_hider.ko

echo add {pid} > /dev/process_hider // hide {pid}

cat /dev/process_hider // see all hidden pids

echo del {pid} > /dev/process_hider // unhide {pid}