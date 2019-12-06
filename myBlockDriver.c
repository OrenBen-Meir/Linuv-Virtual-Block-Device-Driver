#include <linux/module.h>
#include <linux/fs.h>		
#include <linux/errno.h>	
#include <linux/types.h>	
#include <linux/fcntl.h>	
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/string.h>
#include "partition.h"

#define MEMSIZE 1024 
# define KERNSIZE (512)
int major_num = 0;
int secsize = 512;

#define MINORS 8

#define SECTOR_SIZE 512
#define MBR_SIZE SECTOR_SIZE
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16 
#define PARTITION_TABLE_SIZE 64 
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55
#define BR_SIZE SECTOR_SIZE
#define BR_SIGNATURE_OFFSET 510
#define BR_SIGNATURE_SIZE 2
#define BR_SIGNATURE 0xAA55

typedef struct
{
	unsigned char boot_type; 
	unsigned char start_head;
	unsigned char start_sec:6;
	unsigned char start_cyl_hi:2;
	unsigned char start_cyl;
	unsigned char part_type;
	unsigned char end_head;
	unsigned char end_sec:6;
	unsigned char end_cyl_hi:2;
	unsigned char end_cyl;
	unsigned int abs_start_sec;
	unsigned int sec_in_part;
} PartEntry;

typedef PartEntry PartTable[4];

static PartTable def_part_table =
{
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x2,
		start_cyl: 0x00,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x09,
		abs_start_sec: 0x00000001,
		sec_in_part: 0x0000013F
	},
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x1,
		start_cyl: 0x0A, 
		part_type: 0x05,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x13,
		abs_start_sec: 0x00000140,
		sec_in_part: 0x00000140
	},
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x1,
		start_cyl: 0x14,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x1F,
		abs_start_sec: 0x00000280,
		sec_in_part: 0x00000180
	},
	{
	}
};

static unsigned int def_log_part_br_cyl[] = {0x0A, 0x0E, 0x12};
static const PartTable def_log_part_table[] =
{
	{
		{
			boot_type: 0x00,
			start_head: 0x00,
			start_sec: 0x2,
			start_cyl: 0x0A,
			part_type: 0x83,
			end_head: 0x00,
			end_sec: 0x20,
			end_cyl: 0x0D,
			abs_start_sec: 0x00000001,
			sec_in_part: 0x0000007F
		},
		{
			boot_type: 0x00,
			start_head: 0x00,
			start_sec: 0x1,
			start_cyl: 0x0E,
			part_type: 0x05,
			end_head: 0x00,
			end_sec: 0x20,
			end_cyl: 0x11,
			abs_start_sec: 0x00000080,
			sec_in_part: 0x00000080
		},
	},
	{
		{
			boot_type: 0x00,
			start_head: 0x00,
			start_sec: 0x2,
			start_cyl: 0x0E,
			part_type: 0x83,
			end_head: 0x00,
			end_sec: 0x20,
			end_cyl: 0x11,
			abs_start_sec: 0x00000001,
			sec_in_part: 0x0000007F
		},
		{
			boot_type: 0x00,
			start_head: 0x00,
			start_sec: 0x1,
			start_cyl: 0x12,
			part_type: 0x05,
			end_head: 0x00,
			end_sec: 0x20,
			end_cyl: 0x13,
			abs_start_sec: 0x00000100,
			sec_in_part: 0x00000040
		},
	},
	{
		{
			boot_type: 0x00,
			start_head: 0x00,
			start_sec: 0x2,
			start_cyl: 0x12,
			part_type: 0x83,
			end_head: 0x00,
			end_sec: 0x20,
			end_cyl: 0x13,
			abs_start_sec: 0x00000001,
			sec_in_part: 0x0000003F
		},
	}
};

static void copy_mbr(u8 *disk)
{
	memset(disk, 0x0, MBR_SIZE);
	*(unsigned long *)(disk + MBR_DISK_SIGNATURE_OFFSET) = 0x36E5756D;
	memcpy(disk + PARTITION_TABLE_OFFSET, &def_part_table, PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + MBR_SIGNATURE_OFFSET) = MBR_SIGNATURE;
}
static void copy_br(u8 *disk, int start_cylinder, const PartTable *part_table)
{
	disk += (start_cylinder * 32 * SECTOR_SIZE);
	memset(disk, 0x0, BR_SIZE);
	memcpy(disk + PARTITION_TABLE_OFFSET, part_table,
		PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + BR_SIGNATURE_OFFSET) = BR_SIGNATURE;
}
void copy_mbr_n_br(u8 *disk)
{
	int i;

	copy_mbr(disk);
	for (i = 0; i < ARRAY_SIZE(def_log_part_table); i++)
	{
		copy_br(disk, def_log_part_br_cyl[i], &def_log_part_table[i]);
	}
}

/* Block device structure */
struct virtualdrive_dev 
{
	int size;
	u8 *data;
	spinlock_t lock;
	struct request_queue *queue;
	struct gendisk *gd;

} device;

struct virtualdrive_dev *x;

static int device_open(struct block_device *x, fmode_t mode)	 
{
	int ret=0;
	printk(KERN_ALERT "virtualdrive : open \n");
	return ret;
}

static void device_release(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_ALERT "virtualdrive : closed \n");
}

static struct block_device_operations fops =
{
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
};

static int rb_transfer(struct request *req)
{
	int dir = rq_data_dir(req);
	int ret = 0;
	sector_t start_sector = blk_rq_pos(req);
	unsigned int sector_cnt = blk_rq_sectors(req); 
	struct bio_vec bv;
	#define BV_PAGE(bv) ((bv).bv_page)
	#define BV_OFFSET(bv) ((bv).bv_offset)
	#define BV_LEN(bv) ((bv).bv_len)
	struct req_iterator iter;
	sector_t sector_offset;
	unsigned int sectors;
    u8 *buffer;
	sector_offset = 0;
	rq_for_each_segment(bv, req, iter)
	{
		buffer = page_address(BV_PAGE(bv)) + BV_OFFSET(bv);
		if (BV_LEN(bv) % (secsize) != 0)
		{
			printk(KERN_ERR"bio size is not a multiple ofsector size\n");
			ret = -EIO;
		}
		sectors = BV_LEN(bv) / secsize;
		printk(KERN_DEBUG "virtual drive: Start Sector: %llu, Sector Offset: %llu;\
		Buffer: %p; Length: %u sectors\n",\
		(unsigned long long)(start_sector), (unsigned long long) \
		(sector_offset), buffer, sectors);
		
		if (dir == WRITE) 
		{
			memcpy((device.data)+((start_sector+sector_offset)*secsize)\
			,buffer,sectors*secsize);				
		}
		else 
		{
			memcpy(buffer, (device.data)+((start_sector+sector_offset)\
			*secsize), sectors*secsize);	
		}
		sector_offset += sectors;
	}	
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "virtualdrive: bio info doesn't match with the request info");
		ret = -EIO;
	}
	return ret;
}

/** request handling function**/
static void dev_request(struct request_queue *q)
{
	struct request *req;
	int error;
	while ((req = blk_fetch_request(q)) != NULL) 
	{
		error=rb_transfer(req);
		__blk_end_request_all(req, error); 
	}
}

int virtualdrive_init(void)
{
	(device.data) = vmalloc(MEMSIZE*secsize);
	copy_mbr_n_br(device.data);
	return MEMSIZE;	
}

static int __init myvirtualdrive_init(void)
{	
	int ret=0;
	virtualdrive_init();
	major_num = register_blkdev(major_num, "virtualdrive");
	printk(KERN_ALERT "Major Number is : %d", major_num);
	spin_lock_init(&device.lock); 
	device.queue = blk_init_queue( dev_request, &device.lock); 

	device.gd = alloc_disk(MINORS);
	
	(device.gd)->major=major_num; 
	device.gd->first_minor=0; 
	device.gd->minors=MINORS;
	device.gd->fops = &fops;
	device.gd->private_data = &device;
	device.gd->queue = device.queue;
	device.size= virtualdrive_init();
	printk(KERN_INFO"THIS IS DEVICE SIZE %d",device.size);	
	sprintf(((device.gd)->disk_name), "virtualdrive");
	set_capacity(device.gd, MEMSIZE*(KERNSIZE/secsize));  
	add_disk(device.gd);	
	return ret;
}

void __exit myvirtualdrive_exit(void)
{	
	del_gendisk(device.gd);
	unregister_blkdev(major_num, "virtualdrive");
	put_disk(device.gd);
	
	blk_cleanup_queue(device.queue);
	vfree(device.data);
	spin_unlock(&device.lock);	
	printk(KERN_ALERT "unregistered virtualdrive");		
}

module_init(myvirtualdrive_init);
module_exit(myvirtualdrive_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("OREN");
MODULE_DESCRIPTION("VIRTUAL BLOCK DRIVER");
