// SPDX-License-Identifier: GPL-2.0+

#ifndef VUSB_H
#define VUSB_H

#include <linux/device.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/wait.h>

struct vhci_device {
	struct usb_device *udev;

	/*
	 * devid specifies a remote usb device uniquely instead
	 * of combination of busnum and devnum.
	 */
	__u32 devid;

	/* speed of a remote device */
	enum usb_device_speed speed;

	/* vhci root-hub port to which this device is attached */
	__u32 rhport;

	//struct usbip_device ud;

	/* lock for the below link lists */
	spinlock_t priv_lock;

	/* vhci_priv is linked to one of them. */
	struct list_head priv_tx;
	struct list_head priv_rx;

	/* vhci_unlink is linked to one of them */
	struct list_head unlink_tx;
	struct list_head unlink_rx;

	/* vhci_tx thread sleeps for this queue */
	wait_queue_head_t waitq_tx;
};

/* urb->hcpriv, use container_of() */
struct vhci_priv {
	unsigned long seqnum;
	struct list_head list;

	struct vhci_device *vdev;
	struct urb *urb;
};

struct vhci_unlink {
	/* seqnum of this request */
	unsigned long seqnum;

	struct list_head list;

	/* seqnum of the unlink target */
	unsigned long unlink_seqnum;
};

enum hub_speed {
	HUB_SPEED_HIGH = 0,
	HUB_SPEED_SUPER,
};

/* Number of supported ports. Value has an upperbound of USB_MAXCHILDREN */
#ifdef CONFIG_USBIP_VHCI_HC_PORTS
#define VHCI_HC_PORTS CONFIG_USBIP_VHCI_HC_PORTS
#else
#define VHCI_HC_PORTS 8
#endif

/* Each VHCI has 2 hubs (USB2 and USB3), each has VHCI_HC_PORTS ports */
#define VHCI_PORTS	(VHCI_HC_PORTS*2)

#ifdef CONFIG_USBIP_VHCI_NR_HCS
#define VHCI_NR_HCS CONFIG_USBIP_VHCI_NR_HCS
#else
#define VHCI_NR_HCS 1
#endif

#define MAX_STATUS_NAME 16

struct vhci {
	spinlock_t lock;

	struct platform_device *pdev;

	struct vhci_hcd *vhci_hcd_hs;
	struct vhci_hcd *vhci_hcd_ss;
};

/* for usb_hcd.hcd_priv[0] */
struct vhci_hcd {
	struct vhci *vhci;

	u32 port_status[VHCI_HC_PORTS];

	unsigned resuming:1;
	unsigned long re_timeout;

	atomic_t seqnum;

	/*
	 * NOTE:
	 * wIndex shows the port number and begins from 1.
	 * But, the index of this array begins from 0.
	 */
	struct vhci_device vdev[VHCI_HC_PORTS];
};


#endif // VUSB_H
