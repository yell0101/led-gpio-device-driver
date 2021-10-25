#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/gpio.h>

#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

// Tested with kernel 4.9.79



struct GpioRegisters 
{
    uint32_t GPFSEL[6];
    uint32_t Reserved1; 
    uint32_t GPSET[2];
    uint32_t Reserved2;
    uint32_t GPCLR[2];
};

struct GpioRegisters *s_pGpioRegisters;

static void SetGPIOFunction(int ACTGPIO, int LEDGPIO, int PHOTOGPIO, int OutfunCode)
{
    int registerIndex0 = ACTGPIO / 10;
    int bit0 = (ACTGPIO % 10) * 3;

    int registerIndex1 = LEDGPIO / 10;
    int bit1 = (LEDGPIO % 10) * 3;

    int registerIndex2 = PHOTOGPIO / 10;
    int bit2 = (PHOTOGPIO % 10) * 3;

    unsigned oldValue0 = s_pGpioRegisters->GPFSEL[registerIndex0];
    unsigned mask0 = 0b111 << bit0;

    unsigned oldValue1 = s_pGpioRegisters->GPFSEL[registerIndex1];
    unsigned mask1 = 0b111 << bit1;


    s_pGpioRegisters->GPFSEL[registerIndex0] = (oldValue0 & ~mask0) | ((OutfunCode << bit0) & mask0); // GPFSEL[4] [6:8] bit, function code=001
    s_pGpioRegisters->GPFSEL[registerIndex1] = (oldValue1 & ~mask1) | ((OutfunCode << bit1) & mask1); // GPFSEL[1] [24:26] bit, function code=001

    //GPFSEL[1] update

    unsigned oldValue2 = s_pGpioRegisters->GPFSEL[registerIndex2];
    unsigned mask2 = 0b111 << bit2;

    s_pGpioRegisters->GPFSEL[registerIndex2] = (oldValue2 & ~mask2); // GPFSEL[1] [21:23] bit, function code=000
  
}

static void SetGPIOOutputValue(int ACTGPIO, int LEDGPIO, bool outputValue) {
 
    if(outputValue){
        s_pGpioRegisters->GPCLR[ACTGPIO/32] = (1 << (ACTGPIO % 32)); // GPIO 42 -> 10
        s_pGpioRegisters->GPCLR[LEDGPIO/32] = (1 << (LEDGPIO % 32));
        } 
    else {
        s_pGpioRegisters->GPSET[ACTGPIO/ 32] = (1 << (ACTGPIO % 32));
        s_pGpioRegisters->GPSET[LEDGPIO/ 32] = (1 << (LEDGPIO % 32));
    }
}

static struct timer_list s_BlinkTimer;
static int s_BlinkPeriod = 1000;
static const int ActGpioPin = 42; //edit
static const int LedGpioPin = 18; //edit
static const int PhotoGpioPin = 17;

static int photo_irq;

static irqreturn_t gpio_photo_irq (int irq, void *dev_id)
{
    static bool on = false;
    on = !on;
    SetGPIOOutputValue(ActGpioPin, LedGpioPin, on); //42 on

    return 0;

}



#define PERIPH_BASE 0x3f000000
#define GPIO_BASE 0xFE200000 // edit for BCM2711

int LedBlinkModule_init(void) 
{
    int irq_result;

    printk("insmod : LedBlinkModule \n");
    s_pGpioRegisters = (struct GpioRegisters *)ioremap(GPIO_BASE, sizeof(struct GpioRegisters));

    SetGPIOFunction(ActGpioPin,LedGpioPin,PhotoGpioPin,0b001); // set pin output,input

	gpio_request(PhotoGpioPin, "Photo");

    photo_irq = gpio_to_irq(PhotoGpioPin);
    irq_result = request_irq(photo_irq, &gpio_photo_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"Photo",NULL);
    
    printk("Waiting interrupt\n");

    return 0;
}


static void __exit LedBlinkModule_exit(void){
    SetGPIOFunction(ActGpioPin, LedGpioPin, PhotoGpioPin, 0); //Configure the pin as input
    del_timer(&s_BlinkTimer);


}

module_init(LedBlinkModule_init);
module_exit(LedBlinkModule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YerimLee");
MODULE_DESCRIPTION("Raspberry Pi 4 GPIO LED Device Driver Module");