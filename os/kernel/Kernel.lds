/*
* 对链接脚本的一些说明：
* . = 0xffff800000000000 + 0x100000表示将.text位置设置在0xffff800000000000 + 0x100000
* 其中"."表示当前位置
* OUTPUT_FORMAT对输出格式进行了说明，其中三个参数分别代表默认、大端、小端，在此脚本中全部设置为elf64-x86-64
* OUTPUT_ARCH对输出文件的处理器体系进行了说明，此脚本中设置为i386:x86-64
* ENTRY指定了head.S中的_start表示符为程序入口地址
* 在SECTIONS中，. = ALIGN(32768);表示将定位器向后对齐至32768边界处
*/

OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{

	. = 0xffff800000000000 + 0x100000;
	.text :
	{
		_text = .;
		*(.text)
		_etext = .;
	}

	. = ALIGN(8);

	.data :
	{
		_data = .;
		*(.data)		
		_edata = .;
	}

	.rodata : 
	{
		_rodata = .;	
		*(.rodata)
		_erodata = .;
	}

	. = ALIGN(32768);
	.data.init_task : { *(.data.init_task) }

	.bss :
	{
		_bss = .;
		*(.bss)
		_ebss = .;
	}

	_end = .;
}
