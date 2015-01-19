#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

static int __init rbuf_init(void) /* регистрация модуля */
{
	int ret;
	dev_t dev_number;

	c_dev = cdev_alloc(); /* выделяем память под cdev */
	c_dev->owner = THIS_MODULE; /* заполнение поля owner (владелец) */
	ret = alloc_chrdev_region(&dev_number, 0, 1, "ring_buffer"); /* динамическое выделение региона номеров устройства */
	if (ret < 0) {
		printk("Char device region allocation failed.");
		return ret;
	}
	major = MAJOR(dev_number);
	ret = cdev_add(c_dev, dev_number, 1); /* добавление  cdev */
	if (ret < 0) {
		unregister_chrdev_region(MKDEV(major, 0), 1);
		printk("Major number allocation failed.");
		return ret;
	}
	cdev_init(c_dev, &fops); /* регистрация файловых операций */
	printk("ring buffer module installed");
	printk("Major = %d\tMinor = %d", MAJOR(dev_number), MINOR(dev_number));
	return 0;
}

static void __exit rbuf_exit(void) /* выгрузка модуля */
{
	kfree(table); /* освобождение памяти, выделленой под таблицу */
	cdev_del(c_dev); /* освобождение памяти, выделленой под cdev */
	unregister_chrdev_region(MKDEV(major, 0), 1); /* освобождение, выделенного региона номеров устройства */
	printk("ring buffer module removed");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bublis@github.com");
MODULE_DESCRIPTION("Ring buffer char devices");
module_init(rbuf_init);
module_exit(rbuf_exit);
