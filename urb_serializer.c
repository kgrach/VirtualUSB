#include <linux/printk.h>
#include <linux/string.h>
#include <linux/spinlock.h>

#include "urb_serializer.h"

static unsigned long	flags;
static spinlock_t       lock;
unsigned int log_num = 0;


void urb2log(struct urb *urb, const char* context) {

    spin_lock_irqsave(&lock, flags);

    pr_debug("---- Start %d, (%s) ----", log_num, context);

    print_hex_dump(KERN_DEBUG, "urb.pipe                        ", DUMP_PREFIX_NONE, sizeof(urb->pipe),       
                    4, &urb->pipe, sizeof(urb->pipe), false);

    print_hex_dump(KERN_DEBUG, "urb.stream_id                   ", DUMP_PREFIX_NONE, sizeof(urb->stream_id),  
                    4, &urb->stream_id, sizeof(urb->stream_id), false);

    print_hex_dump(KERN_DEBUG, "urb.status                      ", DUMP_PREFIX_NONE, sizeof(urb->status),  
                    4, &urb->status, sizeof(urb->status), false);
    
    print_hex_dump(KERN_DEBUG, "urb.transfer_flags              ", DUMP_PREFIX_NONE, sizeof(urb->transfer_flags),  
                    4, &urb->transfer_flags, sizeof(urb->transfer_flags), false);

    print_hex_dump(KERN_DEBUG, "urb.transfer_buffer_length      ", DUMP_PREFIX_NONE, sizeof(urb->transfer_buffer_length),  
                    4, &urb->transfer_buffer_length, sizeof(urb->transfer_buffer_length), false);
    if(urb->transfer_buffer_length > 0  && urb->transfer_buffer_length <= 0xFF) {
        print_hex_dump(KERN_DEBUG, "urb.transfer_buffer   ", DUMP_PREFIX_OFFSET, 24,  
                        1, urb->transfer_buffer, urb->transfer_buffer_length, false);
    }
    print_hex_dump(KERN_DEBUG, "urb.actual_length               ", DUMP_PREFIX_NONE, sizeof(urb->actual_length),  
                    4, &urb->actual_length, sizeof(urb->actual_length), false);
    if(urb->setup_packet) {
        print_hex_dump(KERN_DEBUG, "urb.setup_packet                ", DUMP_PREFIX_NONE, 8,  
                        1, urb->setup_packet, 8, false); 
    }
    print_hex_dump(KERN_DEBUG, "urb.start_frame                 ", DUMP_PREFIX_NONE, sizeof(urb->start_frame),  
                    4, &urb->start_frame, sizeof(urb->start_frame), false);

    print_hex_dump(KERN_DEBUG, "urb.number_of_packets           ", DUMP_PREFIX_NONE, sizeof(urb->number_of_packets),  
                    4, &urb->number_of_packets, sizeof(urb->number_of_packets), false);

    print_hex_dump(KERN_DEBUG, "urb.interval                    ", DUMP_PREFIX_NONE, sizeof(urb->interval),  
                    4, &urb->interval, sizeof(urb->interval), false);
    
    print_hex_dump(KERN_DEBUG, "urb.error_count                 ", DUMP_PREFIX_NONE, sizeof(urb->error_count),  
                    4, &urb->error_count, sizeof(urb->error_count), false);

    print_hex_dump(KERN_DEBUG, "urb.num_mapped_sgs              ", DUMP_PREFIX_NONE, sizeof(urb->num_mapped_sgs),  
                    4, &urb->num_mapped_sgs, sizeof(urb->num_mapped_sgs), false);

    print_hex_dump(KERN_DEBUG, "urb.num_sgs                     ", DUMP_PREFIX_NONE, sizeof(urb->num_sgs),  
                    4, &urb->num_sgs, sizeof(urb->num_sgs), false);	

    pr_debug("---- Stop %d ----", log_num);
    
    ++log_num;
    
    spin_unlock_irqrestore(&lock, flags);
}