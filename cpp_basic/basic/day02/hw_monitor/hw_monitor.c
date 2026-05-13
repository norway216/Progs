// 文件: hw_monitor_6x_final.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/fcntl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MENG YAN");
MODULE_DESCRIPTION("Memory + Disk health monitor for Linux 6.x");

static struct task_struct *monitor_thread;

#define TEST_MEM_SIZE 4096
#define SLEEP_MS 5000

// ------------------- 内存检测 -------------------
static void check_memory(void)
{
    void *buf = kmalloc(TEST_MEM_SIZE, GFP_KERNEL);
    int i;
    bool error = false;

    if (!buf) {
        pr_err("[HWMonitor] Memory allocation failed\n");
        return;
    }

    for (i = 0; i < TEST_MEM_SIZE; i++)
        ((char *)buf)[i] = (char)(i & 0xFF);

    for (i = 0; i < TEST_MEM_SIZE; i++) {
        if (((char *)buf)[i] != (char)(i & 0xFF)) {
            pr_err("[HWMonitor] Memory verification failed at offset %d\n", i);
            error = true;
            break;
        }
    }

    if (!error)
        pr_info("[HWMonitor] Memory test passed\n");

    kfree(buf);
}

// ------------------- 硬盘检测 -------------------
static void check_disk(void)
{
    struct file *filp;

    filp = filp_open("/dev/sda", O_RDONLY, 0);
    if (IS_ERR(filp)) {
        pr_err("[HWMonitor] Cannot open /dev/sda\n");
        return;
    }

    pr_info("[HWMonitor] /dev/sda accessible\n");
    filp_close(filp, NULL);
}

// ------------------- 独立线程 -------------------
static int monitor_thread_fn(void *data)
{
    while (!kthread_should_stop()) {
        pr_info("[HWMonitor] Running health check\n");

        check_memory();
        check_disk();

        msleep(SLEEP_MS);
    }

    pr_info("[HWMonitor] Monitor thread stopping\n");
    return 0;
}

// ------------------- 模块初始化和卸载 -------------------
static int __init hw_monitor_init(void)
{
    pr_info("[HWMonitor] Module loaded, starting monitor thread\n");

    monitor_thread = kthread_run(monitor_thread_fn, NULL, "hw_monitor");
    if (IS_ERR(monitor_thread)) {
        pr_err("[HWMonitor] Failed to create monitor thread\n");
        return PTR_ERR(monitor_thread);
    }

    return 0;
}

static void __exit hw_monitor_exit(void)
{
    if (monitor_thread) {
        kthread_stop(monitor_thread);
        pr_info("[HWMonitor] Monitor thread stopped\n");
    }
    pr_info("[HWMonitor] Module unloaded\n");
}

module_init(hw_monitor_init);
module_exit(hw_monitor_exit);