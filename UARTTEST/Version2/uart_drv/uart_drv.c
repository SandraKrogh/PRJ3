#include <linux/cdev.h>   // cdev_add, cdev_init
#include <linux/uaccess.h>  // copy_to_user
#include <linux/module.h> // module_init, GPL
#include <linux/serial_core.h>

#include <stdint.h>

//Includes til interrupts
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define MAXLEN 32
#define MODULE_DEBUG 1   // Enable/Disable Debug messages
#define IRQF_TRIGGER_RISING 0x00000001

/*Til interrupt*/
int irqNumber;
int isr_gpio_value;
static DECLARE_WAIT_QUEUE_HEAD(wq); //wait queue
static int flag = 0; //initialisere interrupt flag 


/* Char Driver Globals */
static struct uart_driver uart_drv_uart_driver;
struct file_operations uart_drv_fops;
static struct class *uart_drv_class;
static dev_t devno;
static struct cdev uart_drv_cdev;

/* Definition of uart devices */
struct Myuart {
  struct uart_device *uart; // Pointer to uart device
  int channel;            // channel, ex. adc ch 0
};

//gpio 
int gpioRx = 15;
int gpioTx = 14;

/* Array of uart devices */
/* Minor used to index array */
struct Myuart uart_devs[4];
const int uart_devs_len = 4;  // Max nbr of devices
static int uart_devs_cnt = 0; // Nbr devices present

//Fra hjemmeside til at sende og recieve
static const uint32_t DR_OFFSET = 0x000;
static const uint32_t FR_OFFSET = 0x018;
static const uint32_t IBRD_OFFSET = 0x024;
static const uint32_t FBRD_OFFSET = 0x028;
static const uint32_t LCR_OFFSET = 0x02c;
static const uint32_t CR_OFFSET = 0x030;
static const uint32_t IMSC_OFFSET = 0x038;
static const uint32_t DMACR_OFFSET = 0x048;

/* Macro to handle Errors */
#define ERRGOTO(label, ...)                     \
  {                                             \
    printk (__VA_ARGS__);                       \
    goto label;                                 \
  } while(0)

/**********************************************************
 * CHARACTER DRIVER METHODS
 **********************************************************/

/*
 * Character Driver Module Init Method
 */

 struct pl011{

   uint64_t base_address;
 }

 //to access registers 
volatile uint32_t *reg(const struct pl011 *dev, uint32_t offset)
{
    const uint64_t addr = dev->base_address + offset;

    return (volatile uint32_t *)((void *)addr);
}

//waiting for transmission to complete 
static const uint32_t FR_BUSY = (1 << 3);

static void wait_tx_complete(const struct pl011 *dev)
{
    while ((*reg(dev, FR_OFFSET) * FR_BUSY) != 0) {}
}

 //interrupt servicec rutine 
irqreturn_t mygpio_isr(int irqNumber, void* dev_data)
{
  flag = 1; 
  wake_up_interruptible(&wq);

  return IRQ_HANDLED; 
}
 
static int __init uart_drv_init(void)
{
  int err=0;

  printk("uart_drv driver initializing\n");

  //Request GPIO

  err=gpio_request(gpioRx, "GpioRx");
  if(err < 0)
   ERRGOTO(err_no_cleanup, "Failed to requst gpio");

   err = gpio_direction_input(gpioRx)
    if(err < 0)
   ERRGOTO(err_Rx_input,"Failed to set Rx as input");

    err=gpio_request(gpioTx, "GpioTx");
  if(err < 0)
   ERRGOTO(err_gpioTx, "Failed to request gpio");

    err = gpio_direction_output(gpioTx)
    if(err < 0)
   ERRGOTO(err_Tx_output,"Failed to set Tx as output");

  /* Allocate major number and register fops*/
  err = alloc_chrdev_region(&devno, 0, 255, "uart_drv driver");
  if(MAJOR(devno) <= 0)
    ERRGOTO(err_fail_region, "Failed to register chardev\n");
  printk(KERN_ALERT "Assigned major no: %i\n", MAJOR(devno));

  cdev_init(&uart_drv_cdev, &uart_drv_fops);
  err = cdev_add(&uart_drv_cdev, devno, 255);
  if (err)
    ERRGOTO(err_cleanup_chrdev, "Failed to create class");

  /* Polulate sysfs entries */
  uart_drv_class = class_create(THIS_MODULE, "uart_drv_class");
  if (IS_ERR(uart_drv_class))
    ERRGOTO(err_cleanup_cdev, "Failed to create class");

  /* Register uart Driver */
  /* THIS WILL INVOKE PROBE, IF DEVICE IS PRESENT!!! */
  err = uart_register_driver(&uart_drv_uart_driver);
  if(err)
    ERRGOTO(err_cleanup_class, "Failed uart Registration\n");

    //Initialisere interrupt RX
    irqNumber = gpio_to_irq(gpioRx);
    if(irqNumber < 0)
    ERRGOTO(err_gpio_irq, "Failed to get interrupt line attached to GPIO\n");

    pr_info("IRQ Line: %i\n",irqNumber);

    err = request_irq(irqNumber, mygpio_isr, IRQF_TRIGGER_RISING, "my_irq", NULL);
    if(err != 0)
    ERRGOTO(err_request_irq, "Failed to request irq\n");


  /* Success */
  return 0;

  /* Errors during Initialization */

  err_request_irq:

  err_gpio_irq:
  uart_unregister_driver(&uart_drv_uart_driver);

 err_cleanup_class:
  class_destroy(uart_drv_class);

 err_cleanup_cdev:
  cdev_del(&uart_drv_cdev);

  err_cleanup_chrdev:
  unregister_chrdev_region(devno, 255);

  err_fail_region:

  err_Tx_output:
  gpio_free(gpioTx);

  err_gpioTx:

  err_Rx_input:
  gpio_free(gpioRx);

  err_no_cleanup:
  return err;
}

/*
 * Character Driver Module Exit Method
 */
static void __exit uart_drv_exit(void)
{
  printk("uart_drv driver Exit\n");

  free_irq(irqNumber, NULL);
  uart_unregister_driver(&uart_drv_uart_driver);
  class_destroy(uart_drv_class);
  cdev_del(&uart_drv_cdev);
  unregister_chrdev_region(devno, 255);
  gpio_free(gpioTx);
  gpio_free(gpioRx);
}

/*
 * Character Driver Write File Operations Method
 */

int ad_uart_read_byte(struct uart_device *uart, u8 adr, u16 *data)
{
  return err;
}

ssize_t spi_drv_write(struct file *filep, const char __user *ubuf,
                      size_t count, loff_t *f_pos)
{
  int minor, len, value,err;
  char kbuf[MAXLEN];

  minor = iminor(filep->f_inode);

  printk(KERN_ALERT "Writing to uart_drv [Minor] %i \n", minor);

  /* Limit copy length to MAXLEN allocated andCopy from user */
  
  len = count < MAXLEN ? count : MAXLEN;
  if(copy_from_user(kbuf, ubuf, len))
    return -EFAULT;

  /* Pad null termination to string */
  kbuf[len] = '\0';

  if(MODULE_DEBUG)
    printk("string from user: %s\n", kbuf);

  /* Convert sting to int */
  sscanf(kbuf,"%i", &value);
  if(MODULE_DEBUG)
    printk("value %i\n", value);

  /* Legacy file ptr f_pos. Used to support
   * random access but in char drv we dont!
   * Move it the length actually  written
   * for compability */
  *f_pos += len;

  /* return length actually written */
  return len;
}

/*
 * Character Driver Read File Operations Method
 */
ssize_t uart_drv_read(struct file *filep, char __user *ubuf,
                     size_t count, loff_t *f_pos)
{
  wait_event_interruptible(wq,flag==1);
  flag = 0;

  isr_gpio_value = gpio_get_value(gpioRx);

  char kbuf[12];


  ad_uar_read_byte(spi_devs,)

  int minor, len;
  char resultBuf[MAXLEN];
  s16 result=1234;

  minor = iminor(filep->f_inode);

  ad_uar_read_byte(spi_devs,)

  
  if(MODULE_DEBUG)
    printk(KERN_ALERT "%s-%i read: %i\n",
           spi_devs[minor].spi->modalias, spi_devs[minor].channel, result);

  /* Convert integer to string limited to "count" size. Returns
   * length excluding NULL termination */
  len = snprintf(resultBuf, count, "%d\n", result);

  /* Append Length of NULL termination */
  len++;

  /* Copy data to user space */
  if(copy_to_user(ubuf, resultBuf, len))
    return -EFAULT;

  /* Move fileptr */
  *f_pos += len;

  return len;
}

/*
 * Character Driver File Operations Structure
 */
struct file_operations spi_drv_fops =
  {
    .owner   = THIS_MODULE,
    .write   = spi_drv_write,
    .read    = spi_drv_read,
  };

/**********************************************************
 * LINUX DEVICE MODEL METHODS (spi)
 **********************************************************/

/*
 * spi_drv Probe
 * Called when a device with the name "spi_drv" is
 * registered.
 */
static int spi_drv_probe(struct spi_device *sdev)
{
  int err = 0;
  struct device *spi_drv_device;

  printk(KERN_DEBUG "New SPI device: %s using chip select: %i\n",
         sdev->modalias, sdev->chip_select);

  /* Check we are not creating more
     devices than we have space for */
  if (spi_devs_cnt > spi_devs_len) {
    printk(KERN_ERR "Too many SPI devices for driver\n");
    return -ENODEV;
  }

  /* Configure bits_per_word, always 8-bit for RPI!!! */
  sdev->bits_per_word = 8;
  spi_setup(sdev);

  /* Create devices, populate sysfs and
     active udev to create devices in /dev */

  /* We map spi_devs index to minor number here */
  spi_drv_device = device_create(spi_drv_class, NULL,
                                 MKDEV(MAJOR(devno), spi_devs_cnt),
                                 NULL, "spi_drv%d", spi_devs_cnt);
  if (IS_ERR(spi_drv_device))
    printk(KERN_ALERT "FAILED TO CREATE DEVICE\n");
  else
    printk(KERN_ALERT "Using spi_devs%i on major:%i, minor:%i\n",
           spi_devs_cnt, MAJOR(devno), spi_devs_cnt);

  /* Update local array of SPI devices */
  spi_devs[spi_devs_cnt].spi = sdev;
  spi_devs[spi_devs_cnt].channel = 0x00; // channel address
  ++spi_devs_cnt;

  return err;
}

/*
 * spi_drv Remove
 * Called when the device is removed
 * Can deallocate data if needed
 */
static int spi_drv_remove(struct spi_device *sdev)
{
  int its_minor = 0;

  printk (KERN_ALERT "Removing spi device\n");

  /* Destroy devices created in probe() */
  device_destroy(spi_drv_class, MKDEV(MAJOR(devno), its_minor));

  return 0;
}

/*
 * spi Driver Struct
 * Holds function pointers to probe/release
 * methods and the name under which it is registered
 */
static const struct of_device_id of_spi_drv_spi_device_match[] = {
  { .compatible = "ase, spi_drv", }, {},
};

static struct spi_driver spi_drv_spi_driver = {
  .probe      = spi_drv_probe,
  .remove           = spi_drv_remove,
  .driver     = {
    .name   = "spi_drv",
    .bus    = &spi_bus_type,
    .of_match_table = of_spi_drv_spi_device_match,
    .owner  = THIS_MODULE,
  },
};

/**********************************************************
 * GENERIC LINUX DEVICE DRIVER STUFF
 **********************************************************/

/*
 * Assignment of module init/exit methods
 */
module_init(spi_drv_init);
module_exit(spi_drv_exit);

/*
 * Assignment of author and license
 */
MODULE_AUTHOR("Peter Hoegh Mikkelsen <phm@ase.au.dk>");
MODULE_LICENSE("GPL");
