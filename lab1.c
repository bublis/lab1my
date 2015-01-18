#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bublis@github.com");
MODULE_DESCRIPTION("Ring buffer char devices");
module_init(rbuf_init);
module_exit(rbuf_exit);
