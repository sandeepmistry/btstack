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
 
// *****************************************************************************
//
// HFP BTstack Mocks
//
// *****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <btstack/btstack.h>
#include "hci.h"
#include "hci_dump.h"
#include "sdp_query_rfcomm.h"
#include "rfcomm.h"
#include "hfp_hf.h"

#include "mock.h"

static void *registered_sdp_app_context;
static uint8_t sdp_rfcomm_channel_nr = 1;
const char sdp_rfcomm_service_name[] = "BTstackMock";
static uint16_t rfcomm_cid = 1;
static uint8_t rfcomm_payload[200];
static uint16_t rfcomm_payload_len;
void * active_connection;

void (*registered_rfcomm_packet_handler)(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
void (*registered_sdp_app_callback)(sdp_query_event_t * event, void * context);

uint8_t * get_rfcomm_payload(){
	return &rfcomm_payload[0];
}

uint16_t get_rfcomm_payload_len(){
	return rfcomm_payload_len;
}

static void prepare_rfcomm_buffer(uint8_t * data, int len){
	memset(&rfcomm_payload, 0, 200);
	int pos = 0;
	if (memcmp((char*)data, "AT", 2) != 0){
		rfcomm_payload[pos++] = '\r';
		rfcomm_payload[pos++] = '\n';
	}
	memcpy((char*)&rfcomm_payload[pos], data, len);
	pos += len;
	if (memcmp((char*)data, "AT", 2) != 0){
		rfcomm_payload[pos++] = '\r';
		rfcomm_payload[pos++] = '\n';
        rfcomm_payload[pos++] = 'O';
        rfcomm_payload[pos++] = 'K';
    }   
    rfcomm_payload[pos++] = '\r';
    rfcomm_payload[pos] = '\n';
	rfcomm_payload_len = pos;
}	


int  rfcomm_send_internal(uint16_t rfcomm_cid, uint8_t *data, uint16_t len){
	if (memcmp((char*)data, "AT", 2) == 0){
		printf("HF send: %s", data);
	} else {
		printf("AG send: %s", data);
	}
	prepare_rfcomm_buffer(data, len);
	return 0;
}

int hci_send_cmd(const hci_cmd_t *cmd, ...){
	printf("hci_send_cmd opcode 0x%02x\n", cmd->opcode);	
	return 0;
}


void sdp_query_rfcomm_register_callback(void(*sdp_app_callback)(sdp_query_event_t * event, void * context), void * context){
	registered_sdp_app_callback = sdp_app_callback;
	registered_sdp_app_context = context;
}

static void sdp_query_complete_response(uint8_t status){
    sdp_query_complete_event_t complete_event = {
        SDP_QUERY_COMPLETE, 
        status
    };
    (*registered_sdp_app_callback)((sdp_query_event_t*)&complete_event, registered_sdp_app_context);
}

static void sdp_query_rfcomm_service_response(uint8_t status){
    sdp_query_rfcomm_service_event_t service_event = {
        SDP_QUERY_RFCOMM_SERVICE, 
        sdp_rfcomm_channel_nr,
        (uint8_t *)sdp_rfcomm_service_name
    };
    (*registered_sdp_app_callback)((sdp_query_event_t*)&service_event, registered_sdp_app_context);
}

void sdp_query_rfcomm_channel_and_name_for_uuid(bd_addr_t remote, uint16_t uuid){
	// printf("sdp_query_rfcomm_channel_and_name_for_uuid %p\n", registered_sdp_app_callback);
	sdp_query_rfcomm_service_response(0);
	sdp_query_complete_response(0);
}

void rfcomm_create_channel_internal(void * connection, bd_addr_t addr, uint8_t channel){
	// RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE
	// printf("rfcomm_create_channel_internal\n");
	active_connection = connection;
    uint8_t event[16];
    uint8_t pos = 0;
    event[pos++] = RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE;
    event[pos++] = sizeof(event) - 2;
    event[pos++] = 0;
    
    bt_flip_addr(&event[pos], addr); pos += 6;
    bt_store_16(event,  pos, 1);   pos += 2;
	event[pos++] = 0;
	
	bt_store_16(event, pos, rfcomm_cid); pos += 2;       // channel ID
	bt_store_16(event, pos, 200); pos += 2;   // max frame size
    (*registered_rfcomm_packet_handler)(connection, HCI_EVENT_PACKET, 0, (uint8_t *) event, pos);
}

int rfcomm_can_send_packet_now(uint16_t rfcomm_cid){
	return 1;
}

void rfcomm_disconnect_internal(uint16_t rfcomm_cid){
	uint8_t event[4];
	event[0] = RFCOMM_EVENT_CHANNEL_CLOSED;
    event[1] = sizeof(event) - 2;
    bt_store_16(event, 2, rfcomm_cid);
    (*registered_rfcomm_packet_handler)(active_connection, HCI_EVENT_PACKET, 0, (uint8_t *) event, sizeof(event));
}

void rfcomm_register_packet_handler(void (*handler)(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)){
	registered_rfcomm_packet_handler = handler;
}

void rfcomm_register_service_internal(void * connection, uint8_t channel, uint16_t max_frame_size){
	printf("rfcomm_register_service_internal\n");
}


void sdp_query_rfcomm_channel_and_name_for_search_pattern(bd_addr_t remote, uint8_t * des_serviceSearchPattern){
	printf("sdp_query_rfcomm_channel_and_name_for_search_pattern\n");
}


void rfcomm_accept_connection_internal(uint16_t rfcomm_cid){
	printf("rfcomm_accept_connection_internal \n");
}

// HFP Mock API

int expected_rfcomm_command(const char * cmd){
    char * ag_cmd = (char *)get_rfcomm_payload();
    int offset = 0;
    int cmd_size = strlen(cmd);

    if (memcmp(ag_cmd, "OK", 2) == 0){
        int ok_found = memcmp(ag_cmd+offset, "OK", 2) == 0;
        while (!ok_found && get_rfcomm_payload_len() - 2 >= offset){
            offset++;
            ok_found = memcmp(ag_cmd+offset, "OK", 2) == 0;
        }
        return ok_found;
    }

    int cmd_found = memcmp(ag_cmd, cmd, cmd_size) == 0;
    while (!cmd_found && get_rfcomm_payload_len() - cmd_size >= offset){
        offset++;
        cmd_found = strncmp(ag_cmd+offset, cmd, cmd_size) == 0;
    }
    
    if (!cmd_found) return 0;
    offset += strlen(cmd);

    int ok_found = memcmp(ag_cmd+offset, "OK", 2) == 0;
    while (!ok_found && get_rfcomm_payload_len() - 2 >= offset){
        offset++;
        ok_found = memcmp(ag_cmd+offset, "OK", 2) == 0;
    }
  
    return cmd_found && ok_found;
}


void inject_rfcomm_command(uint8_t * data, int len){
    prepare_rfcomm_buffer(data, len);
    (*registered_rfcomm_packet_handler)(active_connection, RFCOMM_DATA_PACKET, rfcomm_cid, (uint8_t *) &rfcomm_payload[0], rfcomm_payload_len);
}





