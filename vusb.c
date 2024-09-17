// SPDX-License-Identifier: GPL-2.0+
#include <linux/init.h>
#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "usbip_common.h"
#include "vhci.h"


#define DRIVER_AUTHOR "Grachev Kirill"
#define DRIVER_DESC "Driver virtual USB device"

int vhci_num_controllers = 1;
struct vhci *vhcis;

static const char driver_name[] = "vurtual_usb";

static struct platform_driver vhci_driver = {
	.probe	=  NULL,
	.remove_new = NULL,//vhci_hcd_remove,
	.suspend = NULL,//vhci_hcd_suspend,
	.resume	= NULL,//vhci_hcd_resume,
	.driver	= {
		.name = driver_name,
	},
};

static void del_platform_devices(void)
{
	int i;

	for (i = 0; i < vhci_num_controllers; i++) {
		platform_device_unregister(vhcis[i].pdev);
		vhcis[i].pdev = NULL;
	}
	sysfs_remove_link(&platform_bus.kobj, driver_name);
}

static int __init vusb_init(void)
{
	int i, ret;

	//if (usb_disabled())
	//	return -ENODEV;

	if (vhci_num_controllers < 1)
		vhci_num_controllers = 1;

	vhcis = kcalloc(vhci_num_controllers, sizeof(struct vhci), GFP_KERNEL);
	if (vhcis == NULL)
		return -ENOMEM;

	ret = platform_driver_register(&vhci_driver);
	if (ret)
		goto err_driver_register;

	for (i = 0; i < vhci_num_controllers; i++) {
		void *vhci = &vhcis[i];
		struct platform_device_info pdevinfo = {
			.name = driver_name,
			.id = i,
			.data = &vhci,
			.size_data = sizeof(void *),
		};

		vhcis[i].pdev = platform_device_register_full(&pdevinfo);
		ret = PTR_ERR_OR_ZERO(vhcis[i].pdev);
		if (ret < 0) {
			while (i--)
				platform_device_unregister(vhcis[i].pdev);
			goto err_add_hcd;
		}
	}

	return 0;

err_add_hcd:
	platform_driver_unregister(&vhci_driver);
err_driver_register:
	kfree(vhcis);
	return ret;
}

static void __exit vusb_exit(void)
{
	del_platform_devices();
	platform_driver_unregister(&vhci_driver);
	kfree(vhcis);
}

module_init(vusb_init);
module_exit(vusb_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
