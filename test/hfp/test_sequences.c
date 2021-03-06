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
// HFP Test Sequences
//
// *****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_sequences.h"


#define TEST_SEQUENCE(name) { (char**)name, sizeof(name) / sizeof(char *)}

/* Service Level Connection (slc) test sequences */

const char * slc_test1[] = {
    "AT+BRSF=438",
    "+BRSF:1007", 
    "OK",
    "AT+BAC=1,2", 
    "OK",
    "AT+CIND=?",
    "+CIND:(\"service\",(0,1)),(\"call\",(0,1)),(\"callsetup\",(0,3)),(\"battchg\",(0,5)),(\"signal\",(0,5)),(\"roam\",(0,1)),(\"callheld\",(0,2))",
    "OK",
    "AT+CIND?",
    "+CIND:1,0,0,3,5,0,0",
    "OK",
    "AT+CMER=3,0,0,1",
    "OK",
    "AT+CHLD=?",
    "+CHLD:(1,1x,2,2x,3)",
    "OK"
};

hfp_test_item_t slc_tests[] = {
    TEST_SEQUENCE(slc_test1)
};

/* Service Level Connection (slc) common commands */
const char * slc_cmds_test1[] = {
    "AT+BAC=1,3", 
    "OK"
};

hfp_test_item_t slc_cmds_tests[] = {
    TEST_SEQUENCE(slc_cmds_test1)
};
/* Codecs Connection (cc) test sequences */

const char * cc_test1[] = {
    "AT+BCC", 
    "OK",
    "+BCS:1",
    "AT+BCS=1",
    "OK"
};

const char * cc_test2[] = {
    "AT+BCC", 
    "OK",
    "+BCS:1",
    "AT+BAC=2,3", 
    "OK",
    "+BCS:2",
    "AT+BCS=2",
    "OK"
};

hfp_test_item_t cc_tests[] = {
    TEST_SEQUENCE(cc_test1),
    TEST_SEQUENCE(cc_test2)
};



//////////////

int test_item_size = sizeof(hfp_test_item_t);

// SLC
hfp_test_item_t * hfp_slc_tests(){
    return slc_tests;
}

int slc_tests_size(){
    return sizeof(slc_tests)/test_item_size;
}

char ** default_slc_setup(){
    return (char **)slc_test1;
}

int default_slc_setup_size(){
    return sizeof(slc_test1)/sizeof(char*);
}

// CC
hfp_test_item_t * hfp_cc_tests(){
    return cc_tests;
}
int cc_tests_size(){
    return sizeof(cc_tests) /test_item_size;
}
char ** default_cc_setup(){
    return (char **)cc_test1;
}
int default_cc_setup_size(){
    return sizeof(cc_test1)/sizeof(char*);  
}
 