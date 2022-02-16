/* PCI设备相关 */

#ifndef __PCI_H__
#define __PCI_H__

/* 获取PCI地址 */
#define make_pci_address(bus, device, function, offset) (0x80000000|(bus<<16)|(device<<11)|(function<<8)|(offset&0xfc))

/* PCI配置端口 */
#define CONFIG_ADDRESS	0xcf8
#define CONFIG_DATA		0xcfc


#endif
