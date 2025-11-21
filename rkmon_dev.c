#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/tick.h>

#include "cpu_update.h"
#include "mem_update.h"
#include "network_update.h"
#include "rkmon_uapi.h"

#define DEVICE_NAME "rkmon"

static int update_interval_ms = 500;

static dev_t dev;
static struct cdev c_dev;

static long total, free;
static struct net_rate net_rate_global = {0};
static struct netinfo net;
static struct net_rate *r = &net_rate_global;
static struct netdev_list nl;

static int cpu_count;

static char buffer[1000];

static struct timer_list cpu_timer;

static void cpu_timer_func(struct timer_list *t)
{
    update_cpu_load_all();

    mod_timer(&cpu_timer,
              jiffies + msecs_to_jiffies(update_interval_ms));
}

static ssize_t dev_read(struct file *filep, char __user *buf,
                        size_t len, loff_t *offset)
{
    int pos = 0;
    int i;

    // struct netinfo net;
    // struct net_rate *r = &net_rate_global;

    // struct netdev_list nl;
    get_netdev_list(&nl);

    get_memory_info(&total, &free);

    pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                     "%d\n", cpu_count);

    for (i = 0; i < cpu_count; i++)
    {
        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "%d ", cpu_loads[i]);
    }
    pos += scnprintf(buffer + pos, sizeof(buffer) - pos, "\n");
    for (i = 0; i < cpu_count; i++)
    {
        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "%lld ", cpu_freq[i]);
    }
    pos += scnprintf(buffer + pos, sizeof(buffer) - pos, "\n");
    pos += scnprintf(buffer + pos, sizeof(buffer) - pos, "%ld %ld\n", total, free);

    pos += scnprintf(buffer + pos, sizeof(buffer) - pos, "\n");
    pos += scnprintf(buffer + pos, sizeof(buffer) - pos, "%d\n", nl.count);

    /* Network section */
    for (int i = 0; i < nl.count; i++)
    {
        get_netinfo(nl.names[i], &net, r);
        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "NET %s\n", net.ifname);

        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "LINK %d\n", net.link_up ? 1 : 0);

        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "MAC %pM\n", net.mac);

        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "IPv4 %pI4\n", &net.ipv4);

        if (net.has_ipv6)
            pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                             "IPv6 %pI6\n", &net.ipv6);
        else
            pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                             "IPv6 -\n");

        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "RX %llu KB/s  packets=%llu  errors=%llu\n",
                         net.rx_kbs, net.rx_packets, net.rx_errors);

        pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
                         "TX %llu KB/s  packets=%llu  errors=%llu\n",
                         net.tx_kbs, net.tx_packets, net.tx_errors);
    }

    // for (i = 0; i < nl.count; i++)
    // {
    //     pos += scnprintf(buffer + pos, sizeof(buffer) - pos,
    //                      "%s ", nl.names[i]);
    // }

    pos += scnprintf(buffer + pos, sizeof(buffer) - pos, "\n");
    return simple_read_from_buffer(buf, len, offset, buffer, pos);
}

static long rkmon_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int val;

    switch (cmd)
    {
    case RKMON_SET_UPDATE_RATE:
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;

        if (val < 50 || val > 5000)
            return -EINVAL;

        update_interval_ms = val;

        mod_timer(&cpu_timer,
                  jiffies + msecs_to_jiffies(update_interval_ms));

        pr_info("rkmon: update interval set to %d ms\n", val);
        return 0;
    case RKMON_GET_CPUINFO:
    {
        struct rkmon_cpu_request req;
        memset(&req, 0, sizeof(req));

        req.cpu_count = cpu_count;

        for (int i = 0; i < cpu_count; i++)
        {
            req.cpu_load[i] = cpu_loads[i];
            req.cpu_freq[i] = cpu_freq[i];
        }

        /* среднее и максимум */
        int sum = 0, maxv = 0;
        for (int i = 0; i < cpu_count; i++)
        {
            sum += cpu_loads[i];
            if (cpu_loads[i] > maxv)
                maxv = cpu_loads[i];
        }
        req.avg_load = sum / cpu_count;
        req.max_load = maxv;

        if (copy_to_user((void __user *)arg, &req, sizeof(req)))
            return -EFAULT;

        return 0;
    }
    case RKMON_GET_NETINFO:
    {
        struct rkmon_net_request req;
        struct netinfo tmp; // внутренняя структура ядра

        if (copy_from_user(&req, (void __user *)arg, sizeof(req)))
            return -EFAULT;

        get_netinfo(req.ifname, &tmp, &net_rate_global);

        memset(&req.info, 0, sizeof(req.info));

        strncpy(req.info.ifname, tmp.ifname, sizeof(req.info.ifname));
        memcpy(req.info.mac, tmp.mac, 6);

        req.info.link_up = tmp.link_up;
        req.info.has_ipv6 = tmp.has_ipv6;

        req.info.rx_packets = tmp.rx_packets;
        req.info.tx_packets = tmp.tx_packets;
        req.info.rx_errors = tmp.rx_errors;
        req.info.tx_errors = tmp.tx_errors;

        req.info.rx_kbs = tmp.rx_kbs;
        req.info.tx_kbs = tmp.tx_kbs;

        req.info.ipv4 = tmp.ipv4;
        memcpy(req.info.ipv6, &tmp.ipv6, 16);

        if (copy_to_user((void __user *)arg, &req, sizeof(req)))
            return -EFAULT;

        return 0;
    }

    default:
        return -EINVAL;
    }
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .unlocked_ioctl = rkmon_ioctl,
};

static int __init my_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "rkmon: alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&c_dev, &fops);
    ret = cdev_add(&c_dev, dev, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "rkmon: cdev_add failed\n");
        return ret;
    }

    cpu_count = num_online_cpus();
    if (cpu_count > MAX_CPUS)
        cpu_count = MAX_CPUS;

    printk(KERN_INFO "rkmon: loaded\n");
    printk(KERN_INFO "rkmon: device major=%d minor=0\n", MAJOR(dev));

    timer_setup(&cpu_timer, cpu_timer_func, 0);
    mod_timer(&cpu_timer, jiffies + HZ / 2);

    return 0;
}

static void __exit my_exit(void)
{
    del_timer_sync(&cpu_timer);

    cdev_del(&c_dev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "rkmon: unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexey Kuznetsov, GITHUB: TheCakeMan1");
MODULE_DESCRIPTION("RK Monitor kernel module");
