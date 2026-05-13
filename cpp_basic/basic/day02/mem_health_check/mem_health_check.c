// 文件名: mem_health_check.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MENG YAN");
MODULE_DESCRIPTION("Simple memory hardware health checker thread");

static struct task_struct *mem_thread;

#define TEST_BLOCK_SIZE 4096  // 4KB
#define SLEEP_MS 5000         // 每5秒检测一次

static int mem_check_thread(void *data)
{
    while (!kthread_should_stop()) {
        void *buf;

        // 分配内存
        buf = kmalloc(TEST_BLOCK_SIZE, GFP_KERNEL);
        if (!buf) {
            pr_err("[MemCheck] Failed to allocate memory block!\n");
        } else {
            int i;
            bool error = false;

            // 写入测试数据
            for (i = 0; i < TEST_BLOCK_SIZE; i++)
                ((char *)buf)[i] = (char)(i & 0xFF);

            // 读取并校验
            for (i = 0; i < TEST_BLOCK_SIZE; i++) {
                if (((char *)buf)[i] != (char)(i & 0xFF)) {
                    pr_err("[MemCheck] Memory verification failed at offset %d\n", i);
                    error = true;
                    break;
                }
            }

            if (!error)
                pr_info("[MemCheck] Memory test passed for one block\n");

            kfree(buf);
        }

        // 等待下一次检测
        msleep(SLEEP_MS);
    }
    pr_info("[MemCheck] Thread stopping\n");
    return 0;
}

static int __init mem_check_init(void)
{
    pr_info("[MemCheck] Module loaded, starting memory check thread\n");
    mem_thread = kthread_run(mem_check_thread, NULL, "mem_health_check");
    if (IS_ERR(mem_thread)) {
        pr_err("[MemCheck] Failed to create thread\n");
        return PTR_ERR(mem_thread);
    }
    return 0;
}

static void __exit mem_check_exit(void)
{
    if (mem_thread) {
        kthread_stop(mem_thread);
        pr_info("[MemCheck] Thread stopped\n");
    }
    pr_info("[MemCheck] Module unloaded\n");
}

module_init(mem_check_init);
module_exit(mem_check_exit);