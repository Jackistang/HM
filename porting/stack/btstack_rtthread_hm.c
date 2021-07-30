/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define BTSTACK_FILE__ "btstack_rtthread_hm.c"

// *****************************************************************************
//
// minimal setup for HCI code
//
// *****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "btstack_config.h"

#include "btstack_debug.h"
#include "btstack_event.h"
#include "btstack_memory.h"
#include "btstack_run_loop.h"
#include "btstack_run_loop_rtthread.h"
#include "btstack_uart.h"
#include "bluetooth_company_id.h"
#include "ble/le_device_db_tlv.h"
#include "hci.h"
#include "hci_dump.h"
#include "hci_dump_rtthread_stdout.h"
#include "hci_transport.h"
#include "hci_transport_h4.h"
#include "hm_hci_transport_h4.h"
#include "btstack_stdin.h"
#include "btstack_chipset_zephyr.h"
#include "btstack_tlv_posix.h"

int btstack_main(int argc, const char * argv[]);

#define TLV_DB_PATH_PREFIX "/tmp/btstack_"
#define TLV_DB_PATH_POSTFIX ".tlv"
static char tlv_db_path[100];
static const btstack_tlv_t * tlv_impl;
static btstack_tlv_posix_t   tlv_context;

static btstack_packet_callback_registration_t hci_event_callback_registration;

static const uint8_t read_static_address_command_complete_prefix[] = { 0x0e, 0x1b, 0x01, 0x09, 0xfc };
static bd_addr_t static_address;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    if (packet_type != HCI_EVENT_PACKET) return;
    switch (hci_event_packet_get_type(packet)){
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) break;
            printf("BTstack up and running as %s\n",  bd_addr_to_str(static_address));
            // setup TLV
            strcpy(tlv_db_path, TLV_DB_PATH_PREFIX);
            strcat(tlv_db_path, bd_addr_to_str(static_address));
            strcat(tlv_db_path, TLV_DB_PATH_POSTFIX);
            tlv_impl = btstack_tlv_posix_init_instance(&tlv_context, tlv_db_path);
            btstack_tlv_set_instance(tlv_impl, &tlv_context);
            le_device_db_tlv_configure(tlv_impl, &tlv_context);
            break;
        case HCI_EVENT_COMMAND_COMPLETE:
            if (memcmp(packet, read_static_address_command_complete_prefix, sizeof(read_static_address_command_complete_prefix)) == 0){
                reverse_48(&packet[7], static_address);
                gap_random_address_set(static_address);
            }
            break;
        default:
            break;
    }
}

static int rt_btstack_main(int argc, const char * argv[]){

	/// GET STARTED with BTstack ///
	btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_rtthread_get_instance());

    const hci_dump_t * hci_dump_impl = hci_dump_rtthread_stdout_get_instance();
    hci_dump_init(hci_dump_impl);

    // init HCI
    const hci_transport_t *transport = hci_transport_h4_instance(NULL);
	hci_init(transport, NULL);
    
    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // setup app
    btstack_main(argc, argv);

    // sm required to setup static random Bluetooth address
    sm_init();

    // go
    btstack_run_loop_execute();    

    return 0;
}

#include <rtthread.h>
int btstack_rtthread_port_init(void)
{
    rt_btstack_main(0, NULL);
    return RT_EOK;
}
