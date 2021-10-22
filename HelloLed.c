//=======================================
// helloLed.c
// hello + LED module
//=======================================
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h> // ioremap(), iounmap()

#define GPIO_BASE 0xFE200000 // for BCM2711
#define BLOCK_SIZE 4096

// address offset of registers for BCM_GPIO #18 #10
#define GPIO_LED 18 // BCD_GPIO #18
#define ACT_LED 10
#define GPFSEL0 (0x04/4) // int *
#define GPSET0 (0x1C/4)
#define GPCLR0 (0x28/4)
#define GPFSEL4 (0x10/4)
#define GPSET1 (0x20/4)
#define GPCLR1 (0x2c/4)

volatile unsigned int * gpio_addr;

// LED blinking
static void blinking(void) {
    int i;

    for(i=0; i<5 ; i++){
        // 1C h : 28 dec = 4 * 7
        *(gpio_addr+GPSET0) |= 1 << (GPIO_LED);
        *(gpio_addr+GPSET1) |= 1 << (ACT_LED);
        mdelay(1000);
        // 28 h : 40 dec = 4 * 10
        *(gpio_addr+GPCLR0) |= 1 << (GPIO_LED);
        *(gpio_addr+GPCLR1) |= 1 << (ACT_LED);
        mdelay(1000);
    }
}

static int hello_init(void) {
    printk("insmod : Hello, GPIO LED Module Test.....\n");
    gpio_addr = ioremap(GPIO_BASE, BLOCK_SIZE);

    // GPFSEL1 = gpio_addr + 1, #18
    *(gpio_addr+GPFSEL0) &= ~(6 << (((GPIO_LED)%10)*3));
    *(gpio_addr+GPFSEL4) &= ~(6 << (((ACT_LED)%10)*3));

    blinking(); // LED ON/OFF

  return 0;
}

static void hello_exit(void) 
{
 iounmap(gpio_addr); 
 printk("rmmod : Hello, GPIO LED Module Test......\n");
}

module_init(hello_init); 
module_exit(hello_exit);