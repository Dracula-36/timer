/*************************************************************************
	> File Name: timer.c
	> Author: Guanyu Li
	> Mail: guanyuli@hustunique.com
	> Created Time: 2015年01月31日 星期六 14时22分27秒
 ************************************************************************/

#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/miscdevice.h>
#include<linux/fs.h>
#include<linux/timer.h>
#include<linux/moduleparam.h>
#include<linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guanyu Li");
MODULE_DESCRIPTION("An easy timer");

static int interval = 10, time = 0, timer = 0;
module_param(interval, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(interval, "The size of time interval");
module_param(time, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(time, "The uplimit of timer");

static struct timer_list mytimer;

static void timer_func(unsigned long data)
{
    if ( timer >= time || timer < 0) 
        timer=0;
    printk ("%d\n", timer);
    timer = timer + 1;
    mod_timer(&mytimer, jiffies + msecs_to_jiffies(interval));
}

static char *itoa(int num, char *str)
{
    int i = 0,k;
    char temp;
    do {
        str[i++] = num % 10 + '0';
        num = num / 10;
    } while (num);
    str[i] = '\n';
    str[i+1] = '\0';
    for (k = 0; k <= (i-1)/2; k++) {
        temp = str[k];
        str[k] = str[i-k-1];
        str[i-k-1] = temp;
    }
    return str;
}

static int isspace(int x)  
{  
    if (x == ' ' || x == '\t' || x == '\n' || x == '\f' || x == '\b' || x == '\r')  
        return 1;  
    else   
        return 0;  
}  
  
static int isdigit(int x)  
{  
    if (x <= '9' && x>= '0')           
        return 1;   
    else   
        return 0;      
  }            

static int atoi(const char *nptr)  
{  
    int c;          
    int total;         
    int sign;           
    while (isspace((int)(unsigned char)*nptr)) 
        ++nptr;  
    c = (int)(unsigned char)*nptr++;  
    sign = c;           
    if (c == '-' || c == '+')  
        c = (int)(unsigned char)*nptr++;
    total = 0;  
    while (isdigit(c)) {  
        total = 10 * total + (c - '0');     
        c = (int)(unsigned char)*nptr++;
    }    
    if (sign == '-')  
        return -total;  
    else    
        return total;   
}

static ssize_t timer_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{    
    char output[20] = "now timer = ", num[10];
    itoa(timer,num);
    strcat (output,num);
    int len = strlen (output);
    if (count < len) 
        return -EINVAL;
    if (*ppos != 0) 
        return 0;
    if (copy_to_user(buf,output,len)) 
        return -EINVAL;
    *ppos = len;
    return len;
}

static ssize_t timer_write(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char kerbuf[100];
    int len;
    if (count == 0) 
        return 0;
    if (copy_from_user(kerbuf,buf,count)) 
        return -EINVAL;
    len = strlen (kerbuf);
    timer = atoi(kerbuf);
    mod_timer(&mytimer, jiffies + msecs_to_jiffies(interval));
    return len;
}

static struct file_operations timer_fops = {
    .owner = THIS_MODULE,
    .read = timer_read,
    .write = timer_write,
};

static struct miscdevice timer_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "timer",
    .fops = &timer_fops
};

static int __init timer_init(void)
{
    if (time == 0) time=100*interval;
    setup_timer(&mytimer, timer_func,(unsigned long)timer);
    mytimer.expires = jiffies + msecs_to_jiffies(interval);
    add_timer(&mytimer);
    misc_register(&timer_miscdevice);
    printk(KERN_INFO "timer devices has been registered\n");
    return 0;
}

static void __exit timer_exit(void)
{
    del_timer(&mytimer);
    misc_deregister(&timer_miscdevice);
    printk(KERN_INFO "timer devices has been unregistered\n");
}

module_init(timer_init);
module_exit(timer_exit);
