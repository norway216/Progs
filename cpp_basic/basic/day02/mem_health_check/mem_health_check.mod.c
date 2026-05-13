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
	{ 0xfaabfe5e, "kmalloc_caches" },
	{ 0xc064623f, "__kmalloc_cache_noprof" },
	{ 0xe8213e80, "_printk" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x67628f51, "msleep" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x7f79e79a, "kthread_create_on_node" },
	{ 0xda78a6ac, "wake_up_process" },
	{ 0x0571dc46, "kthread_stop" },
	{ 0xd272d446, "__fentry__" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xfaabfe5e,
	0xc064623f,
	0xe8213e80,
	0xcb8b6ec6,
	0x67628f51,
	0xd272d446,
	0x7f79e79a,
	0xda78a6ac,
	0x0571dc46,
	0xd272d446,
	0x5e505530,
	0xbd03ed67,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"_printk\0"
	"kfree\0"
	"msleep\0"
	"__x86_return_thunk\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"kthread_stop\0"
	"__fentry__\0"
	"kthread_should_stop\0"
	"random_kmalloc_seed\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "846D6BE86C96A74FD34D308");
