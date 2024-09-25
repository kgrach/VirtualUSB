#ifndef __URB_SERIALIZER
#define __URB_SERIALIZER

#include <linux/usb.h>

void urb2log(struct urb *urb, const char* context);

#endif