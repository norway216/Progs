#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/cputime.h>

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>
#include <linux/uidgid.h>
#include <linux/cred.h>
#include <linux/cpumask.h>
#include <linux/math64.h>
#include <linux/mutex.h>
#include <linux/atomic.h>

#define MODULE_NAME "resource_monitor"

static char *log_path = "/tmp/kernel_resource_monitor.log";
module_param(log_path, charp, 0644);
MODULE_PARM_DESC(log_path, "Log file path");

static int monitor_uid = 0;
module_param(monitor_uid, int, 0644);
MODULE_PARM_DESC(monitor_uid, "Linux UID to monitor, for example normal user UID 1000");

static struct task_struct *user_monitor_task = NULL;
static struct task_struct *self_monitor_task = NULL;

static atomic_t stop_flag = ATOMIC_INIT(0);
static DEFINE_MUTEX(log_mutex);

static u64 last_user_cpu_ns = 0;
static u64 last_time_ns = 0;

/*
 * 内核态写文件函数
 *
 * 注意：
 * 生产环境中不推荐内核模块频繁直接写文件。
 * 更推荐 procfs / sysfs / debugfs，然后由用户态程序读取并写文件。
 */
static int write_log(const char *fmt, ...)
{
    struct file *file;
    loff_t pos = 0;
    char buf[512];
    int len;
    int ret;
    va_list args;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len <= 0)
        return len;

    if (len >= sizeof(buf))
        len = sizeof(buf) - 1;

    mutex_lock(&log_mutex);

    file = filp_open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(file)) {
        ret = PTR_ERR(file);
        mutex_unlock(&log_mutex);
        pr_err("[%s] failed to open log file: %s, ret=%d\n",
               MODULE_NAME, log_path, ret);
        return ret;
    }

    pos = file->f_pos;
    ret = kernel_write(file, buf, len, &pos);
    file->f_pos = pos;

    filp_close(file, NULL);

    mutex_unlock(&log_mutex);

    return ret;
}

static void get_current_time_string(char *buf, size_t size)
{
    struct timespec64 ts;
    struct tm tm;

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);

    snprintf(buf, size,
             "%04ld-%02d-%02d %02d:%02d:%02d",
             tm.tm_year + 1900,
             tm.tm_mon + 1,
             tm.tm_mday,
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec);
}

/*
 * 统计指定 UID 下所有进程的 RSS 和 CPU 时间
 */
static void collect_user_resource(int uid,
                                  unsigned long *process_count,
                                  unsigned long *rss_kb,
                                  u64 *cpu_ns)
{
    struct task_struct *task;
    struct mm_struct *mm;
    u64 utime = 0;
    u64 stime = 0;
    unsigned long rss_pages = 0;

    *process_count = 0;
    *rss_kb = 0;
    *cpu_ns = 0;

    rcu_read_lock();

    for_each_process(task) {
        if (__kuid_val(task_uid(task)) != uid)
            continue;

        (*process_count)++;

        /*
         * 统计 CPU 时间
         * utime + stime 代表用户态 + 内核态 CPU 时间
         */
        task_cputime_adjusted(task, &utime, &stime);
        *cpu_ns += utime + stime;

        /*
         * 统计 RSS 内存
         */
        mm = get_task_mm(task);
        if (mm) {
            rss_pages = get_mm_rss(mm);
            *rss_kb += rss_pages << (PAGE_SHIFT - 10);
            mmput(mm);
        }
    }

    rcu_read_unlock();
}

/*
 * 线程 1：
 * 每隔 2 秒采集一次指定 UID 用户的资源占用情况。
 */
static int user_resource_monitor_thread(void *data)
{
    unsigned long process_count = 0;
    unsigned long rss_kb = 0;

    u64 current_cpu_ns = 0;
    u64 current_time_ns = 0;
    u64 delta_cpu_ns = 0;
    u64 delta_time_ns = 0;
    u64 cpu_percent_x100 = 0;

    int cpu_count;
    char time_buf[64];

    write_log("========== user resource monitor thread started, uid=%d ==========\n",
              monitor_uid);

    while (!kthread_should_stop() && !atomic_read(&stop_flag)) {
        collect_user_resource(monitor_uid,
                              &process_count,
                              &rss_kb,
                              &current_cpu_ns);

        current_time_ns = ktime_get_ns();

        if (last_time_ns != 0 && current_time_ns > last_time_ns) {
            delta_cpu_ns = current_cpu_ns - last_user_cpu_ns;
            delta_time_ns = current_time_ns - last_time_ns;

            cpu_count = num_online_cpus();

            /*
             * CPU 使用率估算：
             *
             * cpu_percent =
             *   delta_cpu_time / (delta_real_time * online_cpu_count) * 100
             *
             * 这里放大 100 倍，避免内核态使用浮点数。
             */
            if (delta_time_ns > 0 && cpu_count > 0) {
                cpu_percent_x100 =
                    div64_u64(delta_cpu_ns * 10000ULL,
                              delta_time_ns * cpu_count);
            } else {
                cpu_percent_x100 = 0;
            }
        } else {
            cpu_percent_x100 = 0;
        }

        last_user_cpu_ns = current_cpu_ns;
        last_time_ns = current_time_ns;

        get_current_time_string(time_buf, sizeof(time_buf));

        write_log("[USER] time=%s uid=%d process_count=%lu rss=%lu KB cpu=%llu.%02llu%%\n",
                  time_buf,
                  monitor_uid,
                  process_count,
                  rss_kb,
                  cpu_percent_x100 / 100,
                  cpu_percent_x100 % 100);

        ssleep(2);
    }

    write_log("========== user resource monitor thread exited ==========\n");

    return 0;
}

/*
 * 获取某个 task 的 CPU 时间。
 */
static u64 get_task_cpu_time_ns(struct task_struct *task)
{
    u64 utime = 0;
    u64 stime = 0;

    if (!task)
        return 0;

    task_cputime_adjusted(task, &utime, &stime);

    return utime + stime;
}

/*
 * 线程 2：
 * 监控当前内核模块自身的两个线程资源占用。
 * 运行 20 秒后通知两个线程一起退出。
 */
static int self_resource_monitor_thread(void *data)
{
    int elapsed = 0;
    u64 user_thread_cpu_ns = 0;
    u64 self_thread_cpu_ns = 0;
    char time_buf[64];

    write_log("========== self resource monitor thread started ==========\n");

    while (!kthread_should_stop() && !atomic_read(&stop_flag)) {
        user_thread_cpu_ns = get_task_cpu_time_ns(user_monitor_task);
        self_thread_cpu_ns = get_task_cpu_time_ns(current);

        get_current_time_string(time_buf, sizeof(time_buf));

        write_log("[SELF] time=%s elapsed=%d sec user_thread_cpu=%llu ns self_thread_cpu=%llu ns\n",
                  time_buf,
                  elapsed,
                  user_thread_cpu_ns,
                  self_thread_cpu_ns);

        if (elapsed >= 20) {
            write_log("[SELF] 20 seconds reached, stopping all monitor threads\n");
            atomic_set(&stop_flag, 1);
            break;
        }

        ssleep(2);
        elapsed += 2;
    }

    write_log("========== self resource monitor thread exited ==========\n");

    return 0;
}

static int __init resource_monitor_init(void)
{
    int ret = 0;

    pr_info("[%s] module init\n", MODULE_NAME);

    atomic_set(&stop_flag, 0);
    last_user_cpu_ns = 0;
    last_time_ns = 0;

    write_log("\n\n========== resource monitor module loaded ==========\n");
    write_log("log_path=%s monitor_uid=%d\n", log_path, monitor_uid);

    user_monitor_task = kthread_run(user_resource_monitor_thread,
                                    NULL,
                                    "user_res_monitor");

    if (IS_ERR(user_monitor_task)) {
        ret = PTR_ERR(user_monitor_task);
        user_monitor_task = NULL;
        pr_err("[%s] failed to create user monitor thread, ret=%d\n",
               MODULE_NAME, ret);
        return ret;
    }

    self_monitor_task = kthread_run(self_resource_monitor_thread,
                                    NULL,
                                    "self_res_monitor");

    if (IS_ERR(self_monitor_task)) {
        ret = PTR_ERR(self_monitor_task);
        self_monitor_task = NULL;

        pr_err("[%s] failed to create self monitor thread, ret=%d\n",
               MODULE_NAME, ret);

        atomic_set(&stop_flag, 1);

        if (user_monitor_task) {
            kthread_stop(user_monitor_task);
            user_monitor_task = NULL;
        }

        return ret;
    }

    pr_info("[%s] module loaded successfully\n", MODULE_NAME);

    return 0;
}

static void __exit resource_monitor_exit(void)
{
    pr_info("[%s] module exit\n", MODULE_NAME);

    atomic_set(&stop_flag, 1);

    if (user_monitor_task) {
        kthread_stop(user_monitor_task);
        user_monitor_task = NULL;
    }

    if (self_monitor_task) {
        kthread_stop(self_monitor_task);
        self_monitor_task = NULL;
    }

    write_log("========== resource monitor module unloaded ==========\n");

    pr_info("[%s] module unloaded\n", MODULE_NAME);
}

module_init(resource_monitor_init);
module_exit(resource_monitor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Meng Yan Demo");
MODULE_DESCRIPTION("Linux kernel module with two kthreads to monitor user and self resource usage");
MODULE_VERSION("1.0");