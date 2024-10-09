#ifndef __CHAR_DRIVER_H
#define __CHAR_DRIVER_H

#include <linux/usb.h>


int vusb_init(void);
void vusb_cleanup(void);

void RequestUrb(struct urb *urb);

#endif