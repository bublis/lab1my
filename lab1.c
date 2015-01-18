#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
module_init(buf_init);
module_exit(buf_exit);
