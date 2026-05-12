#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x9d52f437, "filp_close" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x0571dc46, "kthread_stop" },
	{ 0x7f79e79a, "kthread_create_on_node" },
	{ 0xda78a6ac, "wake_up_process" },
	{ 0x680628e7, "ktime_get_real_ts64" },
	{ 0x7fd36f2e, "time64_to_tm" },
	{ 0x40a621c5, "snprintf" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0xd272d446, "__rcu_read_lock" },
	{ 0x0d428105, "init_task" },
	{ 0xd272d446, "__rcu_read_unlock" },
	{ 0x717d40db, "task_cputime_adjusted" },
	{ 0xbf2c538b, "get_task_mm" },
	{ 0xcf46e6bd, "mmput" },
	{ 0x97acb853, "ktime_get" },
	{ 0x67628f51, "msleep" },
	{ 0x2182515b, "__num_online_cpus" },
	{ 0x2719b9fa, "const_current_task" },
	{ 0x0040afbe, "param_ops_int" },
	{ 0x0040afbe, "param_ops_charp" },
	{ 0xd272d446, "__fentry__" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x37197a78, "vsnprintf" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0x1cae9a42, "filp_open" },
	{ 0xdbb4ec87, "kernel_write" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x9d52f437,
	0xf46d5bf3,
	0xd272d446,
	0xe8213e80,
	0xd272d446,
	0x0571dc46,
	0x7f79e79a,
	0xda78a6ac,
	0x680628e7,
	0x7fd36f2e,
	0x40a621c5,
	0x5e505530,
	0xd272d446,
	0x0d428105,
	0xd272d446,
	0x717d40db,
	0xbf2c538b,
	0xcf46e6bd,
	0x97acb853,
	0x67628f51,
	0x2182515b,
	0x2719b9fa,
	0x0040afbe,
	0x0040afbe,
	0xd272d446,
	0xbd03ed67,
	0x37197a78,
	0xf46d5bf3,
	0x1cae9a42,
	0xdbb4ec87,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"filp_close\0"
	"mutex_unlock\0"
	"__x86_return_thunk\0"
	"_printk\0"
	"__stack_chk_fail\0"
	"kthread_stop\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"ktime_get_real_ts64\0"
	"time64_to_tm\0"
	"snprintf\0"
	"kthread_should_stop\0"
	"__rcu_read_lock\0"
	"init_task\0"
	"__rcu_read_unlock\0"
	"task_cputime_adjusted\0"
	"get_task_mm\0"
	"mmput\0"
	"ktime_get\0"
	"msleep\0"
	"__num_online_cpus\0"
	"const_current_task\0"
	"param_ops_int\0"
	"param_ops_charp\0"
	"__fentry__\0"
	"__ref_stack_chk_guard\0"
	"vsnprintf\0"
	"mutex_lock\0"
	"filp_open\0"
	"kernel_write\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9CA510447812BE49D0B5332");
