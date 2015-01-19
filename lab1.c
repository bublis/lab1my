#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#define BUF_SIZE 4096 /* задаем размер буфера */

DECLARE_WAIT_QUEUE_HEAD(wq); /* очередь для ожидания */

static unsigned int major;  /* старший номер для драйвера */
static struct cdev *c_dev;	/* структура cdev */

static int rbuf_open(struct inode *, struct file *); /* открытие буфера */
static int rbuf_write(struct file *, const char __user *, size_t, loff_t *); /* запись в буффер */
static int rbuf_read(struct file *, char __user *, size_t, loff_t *);	/* чтение из буфера */
static int rbuf_release(struct inode *, struct file *);	/* освобождение буфера */

/* структура операций с буфером */
static const struct file_operations fops = {
	.open = rbuf_open,
	.write = rbuf_write,
	.read = rbuf_read,
	.release = rbuf_release
};
/* струтура самого буфера
 * указатель на таблицу буферов
 */
static struct fbuffer {
	uid_t owner;
	char data[BUF_SIZE];
	int head;
	int tail;
	int counter;
} *table;
static unsigned int users_count; /* текущее число пользоватлей */

static int get_usr_ind(void) /* получение индекса текущего пользователя в таблице буферов */
{
	int i;
	uid_t current_user = get_current_user()->uid.val;

	if (table == NULL)
		return -2;
	for (i = 0; i < users_count; i++)
		if (table[i].owner == current_user)
			return i;
	return -1;
}

static bool write_cond(int i) /* проверям есть ли место в буфере для записи или нет */
{
	if (table[i].counter < BUF_SIZE)
		return true;
	return false;
}

static bool read_cond(int i) /* проверям пустой ли буфер или нет */
{
	if (table[i].counter > 0)
		return true;
	return false;
}

static int rbuf_open(struct inode *inode, struct file *filp) /* открытие буфера */
{
	uid_t current_user = get_current_user()->uid.val;

	pr_warn("Device opened by process with PID: %d PPID: %d",
		current->pid, current->real_parent->pid);
	pr_warn("UID: %d", current_user);
	if (!table) { /* если таблицы нет то создаем */
		int i;

		pr_warn("Creating initial table...");
		table = kalloc(sizeof(*table), GFP_KERNEL); /* выделяем память и обнуляем ее для таблицы буфера */
		table->owner = current_user; /* владелец юзер */
		table->head = 0;
		table->tail = 0;
		table->counter = 0;
		/* обнуление данных буфера.
		 * можно было не делать, т.к. kzalloc заполняет выделенную память 0.
		 */
		for (i = 0; i < BUF_SIZE; i++)
			table->data[i] = 0;
		users_count++;
		pr_warn("Table creation complete.");
	} else {
		if (get_usr_ind() == -1) {
			int i;
			struct fbuffer *temp;	/* создаем новый буфер для другого юзера */
			pr_warn("Creating buffer for new user...");
			temp = table;
			table = krealloc(temp, sizeof(struct fbuffer) * /* довыделяем память */
					 (users_count + 1), GFP_KERNEL);
			temp = NULL;
			table[users_count].owner = current_user;
			table[users_count].head = 0;
			table[users_count].tail = 0;
			table[users_count].counter = 0;
			for (i = 0; i < BUF_SIZE; i++)
				table[users_count].data[i] = 0;
			users_count++;
			pr_warn("Buffer creating complete.");
		}
	}
	return 0;
}

static int rbuf_release(struct inode *inode, struct file *filp) /* вызывается когда все процессы закрыли файл */
{
	pr_alert("Device released by process with PID: %d PPID: %d",
		 current->pid, current->real_parent->pid);
	wake_up(&wq);
	return 0;
}
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
