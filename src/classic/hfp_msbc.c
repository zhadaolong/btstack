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
// HFP mSBC encoder wrapper
//
// *****************************************************************************

#include "btstack_config.h"

#include <string.h>

#include "btstack_debug.h"
#include "btstack_sbc_encoder.h"
#include "hfp_msbc.h"

#define MSBC_FRAME_SIZE 57

static btstack_sbc_encoder_state_t state;
static uint8_t msbc_padding[] = {0,0,0};

static uint8_t msbc_buffer[2*(MSBC_FRAME_SIZE + sizeof(msbc_padding))];
static int msbc_buffer_offset = 0; 

void hfp_msbc_init(void){
    btstack_sbc_encoder_init(&state, SBC_MODE_mSBC, 16, 8, 0, 16000, 26);
    msbc_buffer_offset = 0;
}

int hfp_msbc_can_encode_audio_frame_now(void){
    return sizeof(msbc_buffer) - msbc_buffer_offset >= MSBC_FRAME_SIZE + sizeof(msbc_padding); 
}

void hfp_msbc_encode_audio_frame(int16_t * pcm_samples){
    if (!hfp_msbc_can_encode_audio_frame_now()) return;

    btstack_sbc_encoder_process_data(pcm_samples);
    
    memcpy(msbc_buffer + msbc_buffer_offset, msbc_padding, sizeof(msbc_padding));
    msbc_buffer_offset += sizeof(msbc_padding);

    memcpy(msbc_buffer + msbc_buffer_offset, btstack_sbc_encoder_sbc_buffer(), MSBC_FRAME_SIZE);
    msbc_buffer_offset += MSBC_FRAME_SIZE;
}

void hfp_msbc_read_from_stream(uint8_t * buf, int size){
    int bytes_to_copy = size;
    if (size > msbc_buffer_offset){
        bytes_to_copy = msbc_buffer_offset;
        log_error("sbc frame storage is smaller then the output buffer");
        return;
    }

    memcpy(buf, msbc_buffer, bytes_to_copy);
    memmove(msbc_buffer, msbc_buffer + bytes_to_copy, sizeof(msbc_buffer) - bytes_to_copy);
    msbc_buffer_offset -= bytes_to_copy;
}

int hfp_msbc_num_bytes_in_stream(void){
    return msbc_buffer_offset;
}

int hfp_msbc_num_audio_samples_per_frame(void){
    return btstack_sbc_encoder_num_audio_samples();
}

