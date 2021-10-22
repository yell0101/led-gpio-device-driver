//=============================
//원본코드 
//gpiosw.c	
// using gpio.h
//	LED/BTN device driver
//
//=============================


/* GPIO Input Device Driver */


#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("melee");
MODULE_DESCRIPTION("Raspberry Pi 3 GPIO Switch Device Driver Module");

#define GPIOSW_MAJOR   202
//#define GPIOSW_MAJOR   0	// automatic allocation
#define GPIOSW_NAME    "gpiosw"

#define GPIO_SW		17	//5
#define GPIO_LED	18	//26

// address offset of registers for BCM_GPIO #18
#define GPFSEL0		(0x00/4)	// int *
#define GPSET0		(0x1C/4)
#define GPCLR0		(0x28/4)
#define GPLEV0		(0x34/4)

volatile unsigned int * gpio_addr;

static int switch_irq;
static char hello[40] = {0};

int state;		// sw state

static irqreturn_t button_interrupt(int irq, void *dev_id){
	char temp[40] = "GPIO 5 Switch was Pushed!";

	strcpy(hello, temp);

	state = 1;		// press!

	return 0;
}

static ssize_t gpiosw_read(struct file *filp, char *buf, size_t count, loff_t *l){
	int result;

	result = copy_to_user(buf,&hello,sizeof(hello));

	printk("Interrupt");

	// LED toggling....
	if(state) {
		if (!gpio_get_value(GPIO_LED)) {
			int i;
			for (i=0;i<5;i++) {
				gpio_set_value(GPIO_LED, 1);
				mdelay(500);
				gpio_set_value(GPIO_LED, 0);
				mdelay(500);
			}
		}

		else {
			gpio_set_value(GPIO_LED, 0);
		}

		state = 0;		// btn flag clear
	}

	memset(hello, 0, 40);		// clear!!

	return result;
}


static int gpiosw_open(struct inode *inode, struct file *filp){
	int result;

	printk("Kernel Module Open(): %s\n", GPIOSW_NAME);

	gpio_request(GPIO_SW, "SWITCH");
	gpio_direction_input(GPIO_SW);
	gpio_set_debounce(GPIO_SW, 200);		// debouncing

	gpio_request(GPIO_LED, "LED");
	gpio_direction_output(GPIO_LED, 0);

 switch_irq = gpio_to_irq(GPIO_SW);

	result = request_irq(switch_irq, &button_interrupt,IRQF_TRIGGER_FALLING,"SWITCH",NULL);

//	a = request_irq(irqris,(irq_handler_t) rpi_gpio_irqrising, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING ,"rpi_gpio_irqrising",NULL);

	if(result < 0)
		printk(KERN_ERR "%s: Request for IRQ %d failed\n",__FUNCTION__, GPIO_SW);

 	return 0;
}

static int gpiosw_release(struct inode *inode, struct file *filp){
	printk("Kernel Module close(): %s\n", GPIOSW_NAME);

	disable_irq(GPIO_SW);

	return 0;
}

static struct file_operations gpiosw_fops = {
	read: 		gpiosw_read,
	open:		gpiosw_open,
	release:	gpiosw_release,
};

int gpiosw_init(void){
	int result;

	result = register_chrdev(GPIOSW_MAJOR,"GPIO INTERRUPT",&gpiosw_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get major %d\n",GPIOSW_MAJOR);
	}

	printk("init module, GPIO major number : %d\n",GPIOSW_MAJOR);

	return 0;
}

void gpiosw_exit(void){
	unregister_chrdev(GPIOSW_MAJOR,"GPIO INTERRUPT");

	free_irq(switch_irq, NULL);
	gpio_free(GPIO_SW);
	gpio_free(GPIO_LED);

	printk("%s DRIVER CLEANUP\n",GPIOSW_NAME);

	return;
}

module_init(gpiosw_init);
module_exit(gpiosw_exit);

MODULE_LICENSE("GPL");