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

#ifndef CWS_H
#define CWS_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdint.h> /* uint8_t */
#include <stdlib.h> /* strtoul */
#include <string.h>
#include <stdio.h> /* sscanf */
#include <stddef.h> /* size_t */

#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define FINAL_FRAME 0x1
#define NEXT_FRAME 0x2
#define MASKED_FRAME 0x4
#define WS_FRAME 0x0

static const char connection[] = "Connection: ";
static const char upgrade_str[] = "upgrade";
static const char upgrade[] = "Upgrade: ";
static const char websocket_str[] = "websocket";
static const char host[] = "Host: ";
static const char origin[] = "Origin: ";
static const char protocol[] = "Sec-WebSocket-Protocol: ";
static const char key[] = "Sec-WebSocket-Key: ";

enum ws_frame_type {
    WS_ERROR_FRAME = 0,
    WS_INCOMPLETE_FRAME = 0x08,
    WS_TEXT_FRAME = 0x01,
    WS_BINARY_FRAME = 0x02,
    WS_OPENING_FRAME = 0x05,
    WS_CLOSING_FRAME = 0x06,
    WS_PING_FRAME = 0x09,
    WS_PONG_FRAME = 0x0A
};

struct handshake {
    char *resource;
    char *host;
    char *origin;
    char *protocol;
    char *key;
};

/*
 * Create an empty handshake structure.
 */
void
    nullhandshake(struct handshake *hs);

/*
 * Parse a websocket handshake
 *
 * const uint8_t *input_frame - request data pointer
 * size_t input_len - request data length
 * struct handshake - handshake pointer that will be filled with data extracted
 *                    from the request data.
 *                    struct handshake members filled:
 *                    handshake->host
 *                    handshake->origin
 *                    handshake->protocol
 *                    key1
 *
 * Return frame type
 * WS_ERROR_FRAME if an error ocurred pasring the handshake
 * WS_OPENING_FRAME if sucessfull parsed and filled handshake structure
 */
enum ws_frame_type
    ws_parse_handshake(const uint8_t *input_frame,
                       size_t input_len,
                       struct handshake *hs);

/* Given a valid handshake, generate handshake response answer on the
 * out pointer and set out_len to response length.
 *
 * return enum ws_frame_type
 * WS_ERROR_FRAME if handshake is invalid
 * WS_OPENING_FRAME if response was succesfully generated
 */
enum ws_frame_type
    ws_get_handshake_answer(const struct handshake *hs,
                            uint8_t *out_frame,
                            size_t *out_len);

/* Parse a websocket input frame where data is in input_frame and data length in
 * input_len and point out_data_ptr to payload data and change out_len to
 * payload data length.
 *
 * return enum ws_frame_type
 * WS_ERROR_FRAME if input_frame is NULL
 * WS_INCOMPLETE_FRAME if input has length < 2
 * WS_TEXT_FRAME - frame data is text (UTF-8)
 * WS_BINARY_FRAME - frame data is binary
 * WS_PING_FRAME - Ping Frame
 * WS_PONG_FRAME - Pong Frame
 * WS_CLOSING_FRAME - Colsing frame
 */
enum ws_frame_type
    ws_parse_input_frame(uint8_t *input_frame,
                         size_t input_len,
                         uint8_t *out_data_ptr,
                         size_t *out_len);

/*
 * Given some data and it's length, create a websocket frame of type
 * frame_type and make out_frame point to this frame setting it's length on
 * out_len.
 *
 * Values for frame_type:
 *
 * WS_TEXT_FRAME - frame data is text (UTF-8)
 * WS_BINARY_FRAME - frame data is binary
 * WS_PING_FRAME - Ping Frame
 * WS_PONG_FRAME - Pong Frame
 * WS_CLOSING_FRAME - Colsing frame
 * WS_FRAME - this is a special case when the farame is a continuation
 *            frame from another frame. In this case, the frame type is
 *            already specified on the first frame thus, the correct way
 *            to create this second for N frame is to set it's type
 *            to WS_FRAME
 *
 * Values for option
 *
 * WS_FINAL_FRAME - message is completed.
 * WS_NEXT_FRAME - inform that you'll send more frames
 *
 */
enum ws_frame_type
    ws_make_frame(uint8_t *data,
                  size_t data_len,
                  uint8_t **out_frame,
                  size_t *out_len,
                  enum ws_frame_type frame_type,
                  int options);

/* Get Frame Type
 *
 * return enum ws_frame_type
 * WS_TEXT_FRAME - frame data is text (UTF-8)
 * WS_BINARY_FRAME - frame data is binary
 * WS_PING_FRAME - Ping Frame
 * WS_PONG_FRAME - Pong Frame
 * WS_CLOSING_FRAME - Colsing frame
 */
enum ws_frame_type
    type(uint8_t* frame);

/*
 * Given a frame, return a pointer to it's payload data and
 * set it's length on length parameter
 *
 * return uint8_t*
 * Return a pointer to payload data
 */
uint8_t*
    extract_payload(uint8_t* frame, uint64_t *length);

/* Create a websocket header.
 * This function will allocate memory necessary to contain a websocket frame
 * with size payload length data_len, frame type frame_type and options. The
 * header length is set on header_len integer pointer.
 *
 * return uint8_t*
 * Return a pointer to frame header
 */
uint8_t*
    make_header(size_t data_len,
                enum ws_frame_type frame_type,
                int *header_len,
                int options);

#ifdef  __cplusplus
}
#endif

#if TEST
uint8_t*
    get_upto_linefeed(const char *start_from);

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
#endif  /* CWS_H */
