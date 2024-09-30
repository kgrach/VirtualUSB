obj-m := vhci-hcd.o

ccflags-y := -DDEBUG -DCONFIG_USBIP_DEBUG
#ccflags-y := -DDEBUG 

vhci-hcd-y := vhci_sysfs.o vhci_tx.o vhci_rx.o vhci_hcd.o usbip_common.o usbip_event.o urb_serializer.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
