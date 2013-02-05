/* 
 * arch/arm/mach-mx50/version_control.c
 *
 * Copyright (C) 2012,SW-BSP1
 *
 * Board version control for distinguash wifi wc160&wc121
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/proc_fs.h>
#include <mach/gpio.h>

#if 0
//Please notice schematic about the pins
#define VC0_IO	(4*32 + 12)
#define VC1_IO	(4*32 + 13)
#define VC2_IO	(4*32 + 14)
#define VC3_IO	(4*32 + 15)
#endif

#define PROC_CMD_LEN	20

static int board_set_version(unsigned int version)
{
	//version can be get, but can't be set, version control by hardware.
	return 0;
}

static int board_get_version(void)
{
#if 0
	unsigned int reg = 0;
	int i;
	for(i = 0; i < 8; i++)
	{
		reg = __raw_readl(IO_ADDRESS(GPIO5_BASE_ADDR + i * 4)); 	
		printk("Base=0x%x, reg=0x%x\n", i * 4, reg); 
	}
	//return sprintf(reg, "0x%x", (((__raw_readl(IO_ADDRESS(GPIO5_BASE_ADDR + 0x08)) << 16) >> 28) & 0xF));
#endif
	return (((__raw_readl(IO_ADDRESS(GPIO5_BASE_ADDR + 0x08)) << 16) >> 28) & 0xF);
}


static const struct version_proc {
	char *module_name;
	int (*get_version)(void);
	int (*set_version)(unsigned int);
} version_modules[] = {
	{ "board_version", 	board_get_version, 	board_set_version },
};

ssize_t version_proc_read(char *page, char **start, off_t off,
                      int count, int *eof, void *data)
{
	char *out = page;
	unsigned int version;
	const struct version_proc *proc = data;
	ssize_t len;
	
	version = proc->get_version();

	out += sprintf(out, "%d\n", version);

	len = out - page - off;
	if (len < count)
	{
		*eof = 1;
		if (len <= 0)
		{
			return 0;
		}
	}
	else
	{
		len = count;
	}
	*start = page + off;
	return len;
}

ssize_t version_proc_write(struct file *filp, const char __user *buf,
                           unsigned long len, void *data)
{
	char command[PROC_CMD_LEN];
	const struct version_proc *proc = data;

        if (!buf || len > PAGE_SIZE - 1)
                return -EINVAL;

	if (len > PROC_CMD_LEN) {
		printk("Command to long\n");
		return -ENOSPC;
	}   

	if (copy_from_user(command, buf, len)) {
		return -EFAULT;
	}

        if (len < 1)
		return -EINVAL;

	if (strnicmp(command, "on", 2) == 0 || strnicmp(command, "1", 1) == 0)
	{
		proc->set_version(1);
	}
        else if (strnicmp(command, "off", 3) == 0 || strnicmp(command, "0", 1) == 0)
	{
		proc->set_version(0);
	}
        else
	{
                return -EINVAL;
	}

        return len;
}

int __init version_init(void)
{
	int i;
#if 0
	struct proc_dir_entry *version_proc_root = NULL;
	version_proc_root = proc_mkdir("bversion", 0);
	if (!version_proc_root)
	{
		printk("Create /proc/bversion directory failed.\n");
		return -1;
	}
	version_proc_root->owner = THIS_MODULE;
#endif
	for (i = 0; i < (sizeof(version_modules) / sizeof(*version_modules)); i++)
	{
		struct proc_dir_entry *proc;
		mode_t mode;

		mode = 0;
		if (version_modules[i].get_version)
		{
			mode |= S_IRUGO;
		}
		if (version_modules[i].set_version)
		{
			mode |= S_IWUGO;
		}

		//proc = create_proc_entry(version_modules[i].module_name, mode, version_proc_root);
		proc = create_proc_entry(version_modules[i].module_name, mode, NULL);
		if (proc)
		{
			proc->data = (void *)(&version_modules[i]);
			proc->read_proc = version_proc_read;
			proc->write_proc = version_proc_write;
		}
	}
	return 0;
}


late_initcall(version_init);

