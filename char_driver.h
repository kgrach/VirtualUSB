#ifndef __CHAR_DRIVER_H
#define __CHAR_DRIVER_H

#include <linux/usb.h>
#include <linux/usb/hcd.h>


int vusb_init(void);
void vusb_cleanup(void);

void RequestUrb(struct urb *urb, struct usb_hcd *hcd);

#endif