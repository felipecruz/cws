/*
 * Copyright (c) 2010 Putilov Andrey
 * Copyright (c) 2012 Felipe Cruz
 *
 * Permission is hereby granted, free of uint8_tge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H


#ifdef  __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdint.h> /* uint8_t */
#include <stdlib.h> /* strtoul */
#include <string.h>
#include <stdio.h> /* sscanf */
#include <stddef.h> /* size_t */
#ifdef __AVR__
    #include <avr/pgmspace.h>
#else
    #define PROGMEM
    #define PSTR
    #define strstr_P strstr
    #define sscanf_P sscanf
    #define sprintf_P sprintf
    #define strlen_P strlen
    #define memcmp_P memcmp
#endif

#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define FINAL_FRAME 0x1
#define NEXT_FRAME 0x2
#define MASKED_FRAME 0x4

static const char connection[] PROGMEM = "Connection: Upgrade";
static const char upgrade[] PROGMEM = "Upgrade: WebSocket";
static const char host[] PROGMEM = "Host: ";
static const char origin[] PROGMEM = "Origin: ";
static const char protocol[] PROGMEM = "Sec-WebSocket-Protocol: ";
static const char key[] PROGMEM = "Sec-WebSocket-Key: ";

enum ws_frame_type {
    WS_ERROR_FRAME = 0,
    WS_INCOMPLETE_FRAME = 0x08,
    WS_TEXT_FRAME = 0x01,
    WS_BINARY_FRAME = 0x02,
    WS_OPENING_FRAME = 5,
    WS_CLOSING_FRAME = 6,
    WS_PING_FRAME = 0x09,
    WS_PONG_FRAME = 0x0A
};

struct handshake {
    char *resource;
    char *host;
    char *origin;
    char *protocol;
    char *key1;
};

void
    nullhandshake(struct handshake *hs);

enum ws_frame_type
    ws_parse_handshake(const uint8_t *input_frame,
                       size_t input_len,
                       struct handshake *hs);

enum ws_frame_type
    ws_get_handshake_answer(const struct handshake *hs,
                            uint8_t *out_frame,
                            size_t *out_len);

enum ws_frame_type
    ws_make_frame(uint8_t *data,
                  size_t data_len,
                  uint8_t *out_frame,
                  size_t *out_len,
                  enum ws_frame_type frame_type);

enum ws_frame_type
    ws_parse_input_frame(uint8_t *input_frame,
                         size_t input_len,
                         uint8_t *out_data_ptr,
                         uint64_t *out_len);

enum ws_frame_type
    type(uint8_t* frame);

uint8_t*
    extract_payload(uint8_t* frame, uint64_t *length);

#ifdef  __cplusplus
}
#endif

#if TEST
uint8_t*
    get_upto_linefeed(const char *start_from);

uint8_t*
    _make_header(size_t data_len,
                 enum ws_frame_type frame_type,
                 int options);

int
    _end_frame(uint8_t *packet);

int
    _masked(uint8_t *packet);

uint64_t
    f_uint64(uint8_t *value);

uint16_t
    f_uint16(uint8_t *value);

uint64_t
    _payload_length(uint8_t *packet);

uint8_t*
    _extract_mask_len1(uint8_t *packet);

uint8_t*
    _extract_mask_len2(uint8_t *packet);

uint8_t*
    _extract_mask_len3(uint8_t *packet);

uint8_t*
    unmask(uint8_t *packet, uint64_t length, uint8_t *mask);
#endif
#endif  /* WEBSOCKET_H */
