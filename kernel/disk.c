#include "disk.h"
#include "interrupt.h"
#include "APIC.h"
#include "memory.h"
#include "printk.h"
#include "lib.h"
#include "block.h"

static int disk_flags = 0;

void end_request()
{
	kfree((unsigned long *)disk_request.in_using);
	disk_request.in_using = NULL;

	disk_flags = 0;

	if(disk_request.block_request_count)
		cmd_out();
}

void add_request(struct block_buffer_node * node)
{
	list_add_to_before(&disk_request.queue_list,&node->list);
	disk_request.block_request_count++;
}

long cmd_out()
{
	struct block_buffer_node * node = disk_request.in_using = container_of(list_next(&disk_request.queue_list),struct block_buffer_node,list);
	list_del(&disk_request.in_using->list);
	disk_request.block_request_count--;

	while(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_BUSY)
		nop();

	switch(node->cmd)
	{
		case ATA_WRITE_CMD:	

			io_out8(PORT_DISK0_DEVICE,0x40);

			io_out8(PORT_DISK0_ERR_FEATURE,0);
			io_out8(PORT_DISK0_SECTOR_CNT,(node->count >> 8) & 0xff);
			io_out8(PORT_DISK0_SECTOR_LOW ,(node->LBA >> 24) & 0xff);
			io_out8(PORT_DISK0_SECTOR_MID ,(node->LBA >> 32) & 0xff);
			io_out8(PORT_DISK0_SECTOR_HIGH,(node->LBA >> 40) & 0xff);

			io_out8(PORT_DISK0_ERR_FEATURE,0);
			io_out8(PORT_DISK0_SECTOR_CNT,node->count & 0xff);
			io_out8(PORT_DISK0_SECTOR_LOW,node->LBA & 0xff);
			io_out8(PORT_DISK0_SECTOR_MID,(node->LBA >> 8) & 0xff);
			io_out8(PORT_DISK0_SECTOR_HIGH,(node->LBA >> 16) & 0xff);

			while(!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY))
				nop();
			io_out8(PORT_DISK0_STATUS_CMD,node->cmd);

			while(!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ))
				nop();
			port_outsw(PORT_DISK0_DATA,node->buffer,256);
			break;

		case ATA_READ_CMD:

			io_out8(PORT_DISK0_DEVICE,0x40);

			io_out8(PORT_DISK0_ERR_FEATURE,0);
			io_out8(PORT_DISK0_SECTOR_CNT,(node->count >> 8) & 0xff);
			io_out8(PORT_DISK0_SECTOR_LOW ,(node->LBA >> 24) & 0xff);
			io_out8(PORT_DISK0_SECTOR_MID ,(node->LBA >> 32) & 0xff);
			io_out8(PORT_DISK0_SECTOR_HIGH,(node->LBA >> 40) & 0xff);

			io_out8(PORT_DISK0_ERR_FEATURE,0);
			io_out8(PORT_DISK0_SECTOR_CNT,node->count & 0xff);
			io_out8(PORT_DISK0_SECTOR_LOW,node->LBA & 0xff);
			io_out8(PORT_DISK0_SECTOR_MID,(node->LBA >> 8) & 0xff);
			io_out8(PORT_DISK0_SECTOR_HIGH,(node->LBA >> 16) & 0xff);

			while(!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY))
				nop();
			io_out8(PORT_DISK0_STATUS_CMD,node->cmd);
			break;
			
		case GET_IDENTIFY_DISK_CMD:

			io_out8(PORT_DISK0_DEVICE,0xe0);
			
			io_out8(PORT_DISK0_ERR_FEATURE,0);
			io_out8(PORT_DISK0_SECTOR_CNT,node->count & 0xff);
			io_out8(PORT_DISK0_SECTOR_LOW,node->LBA & 0xff);
			io_out8(PORT_DISK0_SECTOR_MID,(node->LBA >> 8) & 0xff);
			io_out8(PORT_DISK0_SECTOR_HIGH,(node->LBA >> 16) & 0xff);

			while(!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY))
				nop();			
			io_out8(PORT_DISK0_STATUS_CMD,node->cmd);

		default:
			color_printk(COL_BLACK,COL_WHITE,"ATA CMD Error\n");
			break;
	}
	return 1;
}

void read_handler(unsigned long nr, unsigned long parameter)
{
	struct block_buffer_node * node = ((struct request_queue *)parameter)->in_using;
	
	if(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
		color_printk(COL_RED,COL_BLACK,"read_handler:%#010x\n",io_in8(PORT_DISK0_ERR_FEATURE));
	else
		port_insw(PORT_DISK0_DATA,node->buffer,256);

	end_request();
}

void write_handler(unsigned long nr, unsigned long parameter)
{
	if(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
		color_printk(COL_RED,COL_BLACK,"write_handler:%#010x\n",io_in8(PORT_DISK0_ERR_FEATURE));

	end_request();
}

void other_handler(unsigned long nr, unsigned long parameter)
{
	struct block_buffer_node * node = ((struct request_queue *)parameter)->in_using;

	if(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
		color_printk(COL_RED,COL_BLACK,"other_handler:%#010x\n",io_in8(PORT_DISK0_ERR_FEATURE));
	else
		port_insw(PORT_DISK0_DATA,node->buffer,256);

	end_request();
}

struct block_buffer_node * make_request(long cmd,unsigned long blocks,long count,unsigned char * buffer)
{
	struct block_buffer_node * node = (struct block_buffer_node *)kmalloc(sizeof(struct block_buffer_node),0);
	list_init(&node->list);

	switch(cmd)
	{
		case ATA_READ_CMD:
			node->end_handler = read_handler;
			node->cmd = ATA_READ_CMD;
			break;

		case ATA_WRITE_CMD:
			node->end_handler = write_handler;
			node->cmd = ATA_WRITE_CMD;
			break;

		default:///
			node->end_handler = other_handler;
			node->cmd = cmd;
			break;
	}
	
	node->LBA = blocks;
	node->count = count;
	node->buffer = buffer;

	return node;
}

void submit(struct block_buffer_node * node)
{	
	add_request(node);
	
	if(disk_request.in_using == NULL)
		cmd_out();
}

void wait_for_finish()
{
	disk_flags = 1;
	while(disk_flags)
		nop();
}

long IDE_open()
{
	color_printk(COL_BLACK,COL_WHITE,"DISK0 Opened\n");
	return 1;
}

long IDE_close()
{
	color_printk(COL_BLACK,COL_WHITE,"DISK0 Closed\n");
	return 1;
}

long IDE_ioctl(long cmd,long arg)
{
	struct block_buffer_node * node = NULL;
	
	if(cmd == GET_IDENTIFY_DISK_CMD)
	{
		node = make_request(cmd,0,0,(unsigned char *)arg);
		submit(node);
		wait_for_finish();
		return 1;
	}
	
	return 0;
}

long IDE_transfer(long cmd,unsigned long blocks,long count,unsigned char * buffer)
{
	struct block_buffer_node * node = NULL;
	if(cmd == ATA_READ_CMD || cmd == ATA_WRITE_CMD)
	{
		node = make_request(cmd,blocks,count,buffer);
		submit(node);
		wait_for_finish();
	}
	else
	{
		return 0;
	}
	
	return 1;
}

struct block_device_operation IDE_device_operation = 
{
	.open = IDE_open,
	.close = IDE_close,
	.ioctl = IDE_ioctl,
	.transfer = IDE_transfer,
};

hw_int_controller disk_int_controller = 
{
	.enable = IOAPIC_enable,
	.disable = IOAPIC_disable,
	.install = IOAPIC_install,
	.uninstall = IOAPIC_uninstall,
	.ack = IOAPIC_edge_ack,
};

void disk_handler(unsigned long nr, unsigned long parameter, struct pt_regs * regs)
{
	struct block_buffer_node * node = ((struct request_queue *)parameter)->in_using;
	node->end_handler(nr,parameter);
}

void disk_init()
{
	struct IO_APIC_RET_entry entry;

	entry.vector = 0x2e;
	entry.deliver_mode = APIC_ICR_IOAPIC_Fixed ;
	entry.dest_mode = ICR_IOAPIC_DELV_PHYSICAL;
	entry.deliver_status = APIC_ICR_IOAPIC_Idle;
	entry.polarity = APIC_IOAPIC_POLARITY_HIGH;
	entry.irr = APIC_IOAPIC_IRR_RESET;
	entry.trigger = APIC_ICR_IOAPIC_Edge;
	entry.mask = APIC_ICR_IOAPIC_Masked;
	entry.reserved = 0;

	entry.destination.physical.reserved1 = 0;
	entry.destination.physical.phy_dest = 0;
	entry.destination.physical.reserved2 = 0;

	register_irq(0x2e, &entry , &disk_handler, (unsigned long)&disk_request, &disk_int_controller, "disk0");

	io_out8(PORT_DISK0_ALT_STA_CTL,0);
	
	list_init(&disk_request.queue_list);
	disk_request.in_using = NULL;
	disk_request.block_request_count = 0;
	
	disk_flags = 0;
}

void disk_exit()
{
	unregister_irq(0x2e);
}
