/*
 * Copyright (c) 2010 Putilov Andrey
 * Copyright (c) 2012 Felipe Cruz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
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

#include <openssl/sha.h>

#include "websocket.h"
#include "b64.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 1
#endif

#define _HASHVALUE "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

static char rn[] PROGMEM = "\r\n";

void
    nullhandshake(struct handshake *hs)
{
    hs->host = NULL;
    hs->key1 = NULL;
    hs->key2 = NULL;
    hs->origin = NULL;
    hs->protocol = NULL;
    hs->resource = NULL;
}

static uint8_t*
    get_upto_linefeed(const uint8_t *start_from)
{
    uint8_t *write_to;
    uint8_t new_length = strstr_P(start_from, rn) - start_from;
    
    assert(new_length);
    
    write_to = (char *) malloc(new_length + 1); //+1 for '\x00'
    
    assert(write_to);
    
    memcpy(write_to, start_from, new_length);
    write_to[new_length] = 0;

    return write_to;
}

enum ws_frame_type
    ws_parse_handshake(const uint8_t *input_frame,
                       size_t input_len,
                       struct handshake *hs)
{
    const char *input_ptr = (const char *)input_frame;
    const char *end_ptr = (const char *)input_frame + input_len;

    // measure resource size
    char *first = strchr((const char *)input_frame, ' ');
    if (!first)
        return WS_ERROR_FRAME;
    first++;
    char *second = strchr(first, ' ');
    if (!second)
        return WS_ERROR_FRAME;

    if (hs->resource) {
        free(hs->resource);
        hs->resource = NULL;
    }
    hs->resource = (char *)malloc(second - first + 1); // +1 is for \x00 symbol
    assert(hs->resource);

    if (sscanf_P(input_ptr, PSTR("GET %s HTTP/1.1\r\n"), hs->resource) != 1)
        return WS_ERROR_FRAME;
    input_ptr = strstr_P(input_ptr, rn) + 2;

    /*
        parse next lines
     */
    #define input_ptr_len (input_len - (input_ptr-input_frame))
    #define prepare(x) do {if (x) { free(x); x = NULL; }} while(0)
    uint8_t connection_flag = FALSE;
    uint8_t upgrade_flag = FALSE;
    while (input_ptr < end_ptr && input_ptr[0] != '\r' && input_ptr[1] != '\n') {
        if (memcmp_P(input_ptr, host, strlen_P(host)) == 0) {
            input_ptr += strlen_P(host);
            prepare(hs->host);
            hs->host = get_upto_linefeed(input_ptr);
        } else
            if (memcmp_P(input_ptr, origin, strlen_P(origin)) == 0) {
            input_ptr += strlen_P(origin);
            prepare(hs->origin);
            hs->origin = get_upto_linefeed(input_ptr);
        } else
            if (memcmp_P(input_ptr, protocol, strlen_P(protocol)) == 0) {
            input_ptr += strlen_P(protocol);
            prepare(hs->protocol);
            hs->protocol = get_upto_linefeed(input_ptr);
        } else
            if (memcmp_P(input_ptr, key, strlen_P(key)) == 0) {
            input_ptr += strlen_P(key);
            prepare(hs->key1);
            hs->key1 = get_upto_linefeed(input_ptr);
        } else
            if (memcmp_P(input_ptr, key1, strlen_P(key1)) == 0) {
            input_ptr += strlen_P(key1);
            prepare(hs->key1);
            hs->key1 = get_upto_linefeed(input_ptr);
        } else
            if (memcmp_P(input_ptr, key2, strlen_P(key2)) == 0) {
            input_ptr += strlen_P(key2);
            prepare(hs->key2);
            hs->key2 = get_upto_linefeed(input_ptr);
        } else
            if (memcmp_P(input_ptr, connection, strlen_P(connection)) == 0) {
            connection_flag = TRUE;
        } else
            if (memcmp_P(input_ptr, upgrade, strlen_P(upgrade)) == 0) {
            upgrade_flag = TRUE;
        };

        input_ptr = strstr_P(input_ptr, rn) + 2;
    }

    // we have read all data, so check them
    if (!hs->host || !hs->origin || !connection_flag || !upgrade_flag)
    {
        return WS_ERROR_FRAME;
    }

    return WS_OPENING_FRAME;
}

enum ws_frame_type
    ws_get_handshake_answer(const struct handshake *hs,
                            uint8_t *out_frame,
                            size_t *out_len)
{
    unsigned char accept_key[30];
    unsigned char digest_key[20];
    char *pre_key = strcat(hs->key1, _HASHVALUE);

    SHA_CTX sha;

    assert(out_frame);

    SHA1_Init(&sha);
    SHA1_Update(&sha, pre_key, strlen(pre_key));

    debug_print("BaseKey: %s\n", pre_key);

    SHA1_Final(digest_key, &sha);

    debug_print("DigestKey: %s\n", digest_key);

    lws_b64_encode_string(digest_key ,
                          strlen(digest_key),
                          accept_key,
                          sizeof(accept_key));

    debug_print("AcceptKey: %s\n", accept_key);

    unsigned int written = sprintf_P((char *)out_frame,
            PSTR("HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n\r\n"), accept_key);

    *out_len = written;
    return WS_OPENING_FRAME;
}

enum ws_frame_type
    ws_make_frame(uint8_t *data,
                  size_t data_len,
                  uint8_t *out_frame,
                  size_t *out_len,
                  enum ws_frame_type frame_type)
{
    assert(out_frame);
    assert(data);

    return frame_type;
}

int
    _end_frame(uint8_t *packet)
{
    return (packet[0] & 0x80) == 0x80;
}

enum ws_frame_type
    type(uint8_t *packet)
{
    int opcode = packet[0] & 0xf;

    if (opcode == 0x01) {
        return WS_TEXT_FRAME;
    } else if (opcode == 0x00) {
        return WS_INCOMPLETE_FRAME;
    } else if (opcode == 0x02) {
        return WS_BINARY_FRAME;
    } else if (opcode == 0x08) {
        return WS_CLOSING_FRAME;
    } else if (opcode == 0x09) {
        return WS_PING_FRAME;
    } else if (opcode == 0x0A) {
        return WS_PONG_FRAME;
    }

}

int
    _masked(uint8_t *packet)
{
    return (packet[1] >> 7) & 0x1;
}

uint64_t
    f_uint64(uint8_t *value)
{
    uint64_t length = 0;

    for (int i = 0; i < 8; i++) {
        length = (length << 8) | value[i];
    }

    return length;
}

uint16_t
    f_uint16(uint8_t *value)
{
    uint16_t length = 0;

    for (int i = 0; i < 2; i++) {
        length = (length << 8) | value[i];
    }

    return length;
}

uint64_t
    _payload_length(uint8_t *packet)
{
    int length = -1;
    uint8_t temp = 0;

    if (_masked(packet)) {
        temp = packet[1];
        length = (temp &= ~(1 << 7));
    } else {
        length = packet[1];
    }

    if (length < 125) {
        return length;
    } else if (length == 126) {
        return f_uint16(&packet[2]);
    } else if (length == 127) {
        return f_uint64(&packet[2]);
    } else {
        return length;
    }
}

uint8_t*
    _extract_mask_len1(uint8_t *packet)
{
    uint8_t *mask;
    int j = 0;

    mask = malloc(sizeof(uint8_t) * 4);

    for(int i = 2; i < 6; i++) {
        mask[j] = packet[i];
        j++;
    }

    return mask;
}

uint8_t*
    _extract_mask_len2(uint8_t *packet)
{
    uint8_t *mask;
    int j = 0;

    mask = malloc(sizeof(uint8_t) * 4);

    for(int i = 4; i < 8; i++) {
        mask[j] = packet[i];
        j++;
    }

    return mask;
}

uint8_t*
    _extract_mask_len3(uint8_t *packet)
{
    uint8_t *mask;
    int j = 0;

    mask = malloc(sizeof(uint8_t) * 4);

    for(int i = 10; i < 14; i++) {
        mask[j] = packet[i];
        j++;
    }

    return mask;
}

uint8_t*
    unmask(uint8_t *packet, uint64_t length, uint8_t *mask)
{
    for (int i = 0; i < length; i++) {
        packet[i] ^= mask[i % 4];
    }

    free(mask);
    return packet;
}

uint8_t*
    extract_payload(uint8_t *packet)
{
    uint8_t *mask;
    int m = _masked(packet);
    uint64_t length = _payload_length(packet);

    if (m == 1) {
        if (length < 126) {
            mask = _extract_mask_len1(packet);
            return unmask(&packet[6], length, mask);
        } else if (length > 126 && length < 65536) {
            mask = _extract_mask_len2(packet);
            return unmask(&packet[8], length, mask);
        } else if (length >= 65536) {
            mask = _extract_mask_len3(packet);
            return unmask(&packet[14], length, mask);
        }
    } else {
        if (length < 126) {
            return &packet[2];
        } else if (length > 126 && length < 65536) {
            return &packet[4];
        } else if (length >= 65536) {
            return &packet[4];
        }
    }
    return NULL;
}

enum ws_frame_type
    ws_parse_input_frame(uint8_t *input_frame,
                         size_t input_len,
                         uint8_t **out_data_ptr,
                         size_t *out_len)
{
    enum ws_frame_type frame_type;

    if (input_frame == NULL)
        return WS_ERROR_FRAME;

    if (input_len < 2)
        return WS_INCOMPLETE_FRAME;

    debug_print("(ws) %d is end frame\n", _end_frame(input_frame));
    debug_print("(ws) %d frame type\n", type(input_frame));
    debug_print("(ws) %s content\n", (char*) extract_payload(input_frame));

    frame_type = type(input_frame);

    return frame_type;
}
