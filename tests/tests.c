/*
 * =====================================================================================
 *
 *       Filename:  tests.c
 *
 *    Description:
 *
 *        Version:  0.1
 *        Created:  03/06/2012 10:09:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Felipe J Cruz (felipecruz@loogica.net),
 *   Organization:  loogica
 *
 * =====================================================================================
 */

#if TEST

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "websocket.h"
#include "CUnit/Basic.h"
#include "test_packages.h"

int
    init_websocket_test_suite(void)
{
    return 0;
}

void
    test_websocket_frames(void)
{
    int fd;
    uint8_t *frame1;
    uint8_t *mask;
    uint8_t *target;
    uint8_t small_frame[] = {0x89};
    enum ws_frame_type type;
    size_t length;

    type = ws_parse_input_frame(NULL, 0, target, &length);

    CU_ASSERT(WS_ERROR_FRAME == type);
    CU_ASSERT(0 == length);

    type = ws_parse_input_frame(NULL, 10, target, &length);

    CU_ASSERT(WS_ERROR_FRAME == type);
    CU_ASSERT(0 == length);

    type = ws_parse_input_frame(small_frame, 1, target, &length);

    CU_ASSERT(WS_INCOMPLETE_FRAME == type);
    CU_ASSERT(0 == length);

    fd = open("tests/ws_frame.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 10);

    read(fd, frame1, 10);

    type = ws_parse_input_frame(frame1, 10, target, &length);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(10-6 == length);

    free(frame1);
    close(fd);

    fd = open("tests/ws_frame2.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 18);

    read(fd, frame1, 18);

    type = ws_parse_input_frame(frame1, 18, target, &length);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(18-2 == length);

    free(frame1);
    close(fd);

    fd = open("tests/ws_frame3.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 961);

    read(fd, frame1, 10);

    type = ws_parse_input_frame(frame1, 961, target, &length);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(961-4 == length);
    // 961 (total payload size) - 4 (fin, srvs, opcode, payload length) bytes
    CU_ASSERT(961-4 == _payload_length(frame1));
    CU_ASSERT(NULL != extract_payload(frame1, (uint64_t*) &length));

    free(frame1);
    close(fd);

    fd = open("tests/ws_frame4.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 90402);

    read(fd, frame1, 90402);

    type = ws_parse_input_frame(frame1, 90402, target, &length);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(90402-10 == length);
    CU_ASSERT(90402-10 == _payload_length(frame1));
    CU_ASSERT(NULL != extract_payload(frame1, (uint64_t*) &length));

    free(frame1);
    close(fd);

    type = ws_parse_input_frame(client_big_masked_frame, 90405, target, &length);
    mask = _extract_mask_len3(client_big_masked_frame);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(90404-14 == length);
    CU_ASSERT(90404-14 == _payload_length(client_big_masked_frame));
    CU_ASSERT(NULL != mask);
    CU_ASSERT(NULL != extract_payload(client_big_masked_frame, (uint64_t*) &length));
    free(mask);

    type = ws_parse_input_frame(client_medium_masked_frame, 90405, target, &length);
    mask = _extract_mask_len3(client_medium_masked_frame);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(333-14 == length);
    CU_ASSERT(333-14 == _payload_length(client_medium_masked_frame));
    CU_ASSERT(NULL != mask);
    CU_ASSERT(NULL != extract_payload(client_medium_masked_frame, (uint64_t*) &length));
    free(mask);
}

void
    test_nullhandshake(void)
{
    struct handshake hs;

    nullhandshake(&hs);

    CU_ASSERT(NULL == hs.host);
    CU_ASSERT(NULL == hs.origin);
    CU_ASSERT(NULL == hs.protocol);
    CU_ASSERT(NULL == hs.resource);
    CU_ASSERT(NULL == hs.key1);
}

void
    test_websocket_check_end_frame(void)
{
    CU_ASSERT(1 == _end_frame(single_frame));
    CU_ASSERT(0 == _end_frame(first_frame));
    CU_ASSERT(1 == _end_frame(second_frame));
    CU_ASSERT(1 == _end_frame(single_frame_masked));
}

void
    test_websocket_get_frame_type(void)
{
    CU_ASSERT(WS_TEXT_FRAME == type(single_frame));
    CU_ASSERT(WS_INCOMPLETE_FRAME != type(single_frame));
    CU_ASSERT(WS_TEXT_FRAME == type(first_frame));
    CU_ASSERT(WS_TEXT_FRAME == type(single_frame_masked));
    CU_ASSERT(WS_BINARY_FRAME == type(len_256));
    CU_ASSERT(WS_BINARY_FRAME == type(len_64k));
    CU_ASSERT(WS_PING_FRAME == type(unmasked_ping));
    CU_ASSERT(WS_PONG_FRAME == type(masked_pong));
    CU_ASSERT(WS_CLOSING_FRAME == type(closing_frame));
    CU_ASSERT(WS_TEXT_FRAME == type(client_big_masked_frame));
    CU_ASSERT(WS_TEXT_FRAME == type(client_medium_masked_frame));
}

void
    test_websocket_check_masked(void)
{
    CU_ASSERT(0 == _masked(single_frame));
    CU_ASSERT(0 == _masked(first_frame));
    CU_ASSERT(0 == _masked(second_frame));
    CU_ASSERT(1 == _masked(single_frame_masked));
    CU_ASSERT(0 == _masked(len_256));
    CU_ASSERT(0 == _masked(len_64k));
    CU_ASSERT(1 == _masked(client_big_masked_frame));
    CU_ASSERT(1 == _masked(client_medium_masked_frame));
}

void
    test_websocket_get_payload_length(void)
{
    CU_ASSERT(5 == _payload_length(single_frame));
    CU_ASSERT(3 == _payload_length(first_frame));
    CU_ASSERT(2 == _payload_length(second_frame));
    CU_ASSERT(5 == _payload_length(single_frame_masked));

    CU_ASSERT(256 == _payload_length(len_256));
    CU_ASSERT(65536 == _payload_length(len_64k));
}

void
    test_websocket_extract_mask(void)
{
    uint8_t *mask;

    mask = _extract_mask_len1(single_frame_masked);
    CU_ASSERT(0 == strncmp((char*) mask,
                           (char*) mask, 4));
    free(mask);

    mask = _extract_mask_len2(len_256_masked);
    CU_ASSERT(0 == strncmp((char*) mask,
                           (char*) mask, 4));
    free(mask);

    mask = _extract_mask_len3(client_big_masked_frame);
    CU_ASSERT(0 == strncmp((char*) mask,
                           (char*) big_data_mask, 4));
    free(mask);

    mask = _extract_mask_len3(client_medium_masked_frame);
    CU_ASSERT(0 == strncmp((char*) mask,
                           (char*) medium_data_mask, 4));
    free(mask);
}

void
    test_websocket_extract_payload(void)
{
    uint64_t length;

    CU_ASSERT(0 == strncmp((char*) extract_payload(single_frame, &length),
                            "Hello", 5));
    CU_ASSERT(5 == length);

    CU_ASSERT(0 == strncmp((char*) extract_payload(unmasked_ping, &length),
                            "Hello", 5));
    CU_ASSERT(5 == length);
}

void
    test_websocket_extract_masked_payload(void)
{
    uint64_t length;

    CU_ASSERT(0 == strncmp((char*) extract_payload(single_frame_masked, &length),
                            "Hello", 5));
    CU_ASSERT(5 == length);

    CU_ASSERT(0 == strncmp((char*) extract_payload(masked_pong, &length),
                            "Hello", 5));
    CU_ASSERT(5 == length);
}

void
    test_get_upto_linefeed(void)
{
    uint8_t *data;

    data = get_upto_linefeed("GET /?encoding=text HTTP/1.1\r\n");
    CU_ASSERT(0 == strcmp((char*) data,
                          "GET /?encoding=text HTTP/1.1"));
    free(data);

    data = get_upto_linefeed("Upgrade: websocket\r\n");
    CU_ASSERT(0 == strcmp((char*) data,
                          "Upgrade: websocket"));
    free(data);

    data = get_upto_linefeed("Sec-WebSocket-Key: rRec6RPAbwPWLEsSQpGDKA==\r\n");
    CU_ASSERT(0 == strcmp((char*) data,
                          "Sec-WebSocket-Key: rRec6RPAbwPWLEsSQpGDKA=="));
    free(data);
}

void
    test_websocket_parse_handshake(void)
{
    struct handshake hs;
    enum ws_frame_type type;
    uint8_t handshake[] = "GET /mychat HTTP/1.1\r\n"
                          "Host: server.example.com\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
                          "Sec-WebSocket-Protocol: chat\r\n"
                          "Sec-WebSocket-Version: 13\r\n"
                          "Origin: http://example.com\r\n";

    nullhandshake(&hs);

    type = ws_parse_handshake(handshake, strlen(handshake), &hs);
    CU_ASSERT(type == WS_OPENING_FRAME);

    CU_ASSERT(0 == strcmp("http://example.com", hs.origin));
    CU_ASSERT(0 == strcmp("chat", hs.protocol));
    CU_ASSERT(0 == strcmp("x3JJHMbDL1EzLkh9GBhXDw==", hs.key1));
    CU_ASSERT(0 == strcmp("server.example.com", hs.host));
}

void
    test_websocket_get_handshake_answer(void)
{
    char *key = "rRec6RPAbwPWLEsSQpGDKA==";
    size_t len;
    uint8_t *out;

    enum ws_frame_type type;
    struct handshake hs;

    nullhandshake(&hs);

    out = (uint8_t*) malloc(sizeof(uint8_t) * 4096);

    hs.key1 = key;
    type = ws_get_handshake_answer(&hs, out, &len);

    CU_ASSERT(0 == strcmp((char*)out,
                          "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: qVMgwBQ7DRv6Mxw0KMXrjlX6EQQ=\r\n\r\n"));
    CU_ASSERT(129 == len);
    CU_ASSERT(type == WS_OPENING_FRAME);

    free(out);

    out = NULL;

    type = ws_get_handshake_answer(&hs, out, &len);

    CU_ASSERT(type == WS_ERROR_FRAME);
    CU_ASSERT(0 == len);

    hs.key1 = NULL;

    type = ws_get_handshake_answer(&hs, out, &len);

    CU_ASSERT(type == WS_ERROR_FRAME);
    CU_ASSERT(0 == len);
}

void test_websocket_make_header(void)
{
    uint8_t *header;
    int header_len = 0;

    //0 length data
    header = make_header(0, WS_TEXT_FRAME, &header_len, 0);
    CU_ASSERT(NULL == header);
    CU_ASSERT(0 == header_len);

    //invalid end_frame int (must be 1 or 0)
    header = make_header(0, WS_TEXT_FRAME, &header_len, 3);
    CU_ASSERT(NULL == header);
    CU_ASSERT(0 == header_len);

    uint8_t single_text_len5[] = {0x81, 0x05};
    header = make_header(5, WS_TEXT_FRAME, &header_len, FINAL_FRAME);
    CU_ASSERT(0 == strncmp((char*) header, (char*) single_text_len5, 2));
    CU_ASSERT(2 == header_len);
    free(header);

    uint8_t unmasked_ping_len125[] = {0x89, 0x7d};
    header = make_header(125, WS_PING_FRAME, &header_len, FINAL_FRAME);
    CU_ASSERT(0 == strncmp((char*) header, (char*) unmasked_ping_len125, 2));
    CU_ASSERT(2 == header_len);
    free(header);

    uint8_t unmasked_pong_len1[] = {0x8A, 0x01};
    header = make_header(1, WS_PONG_FRAME, &header_len, FINAL_FRAME);
    CU_ASSERT(0 == strncmp((char*) header, (char*) unmasked_pong_len1, 2));
    CU_ASSERT(2 == header_len);
    free(header);

    uint8_t single_text_len4096[] = {0x81, 0x7e, 0x10, 0x00};
    header = make_header(4096, WS_TEXT_FRAME, &header_len, FINAL_FRAME);
    CU_ASSERT(0 == strncmp((char*) header, (char*) single_text_len4096, 4));
    CU_ASSERT(4 == header_len);
    free(header);

    uint8_t single_text_len256[] = {0x02, 0x7e, 0x01, 0x00};
    header = make_header(256, WS_BINARY_FRAME, &header_len, NEXT_FRAME);
    CU_ASSERT(0 == strncmp((char*) header, (char*) single_text_len256, 4));
    CU_ASSERT(4 == header_len);
    free(header);

    uint8_t single_text_len319[] = {0x81, 0x7e, 0x01, 0x3f};
    header = make_header(319, WS_TEXT_FRAME, &header_len, FINAL_FRAME);
    CU_ASSERT(0 == strncmp((char*) header, (char*) single_text_len319, 4));
    CU_ASSERT(4 == header_len);
    free(header);

    uint8_t single_text_len64k[] = {0x82, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x01, 0x00, 0x00};
    header = make_header(65536, WS_BINARY_FRAME, &header_len, FINAL_FRAME);
    CU_ASSERT(0 == memcmp((char*) header, (char*) single_text_len64k, 10));
    CU_ASSERT(10 == header_len);
    free(header);
}

void test_websocket_make_frame(void)
{
    size_t length;
    uint8_t *frame;
    enum ws_frame_type type;

    type = ws_make_frame("Hello", 5, &frame, &length, WS_TEXT_FRAME, FINAL_FRAME);
    CU_ASSERT(0 == memcmp(single_frame, frame, 7));
    CU_ASSERT(7 == length);

    type = ws_make_frame("Hel", 3, &frame, &length, WS_TEXT_FRAME, NEXT_FRAME);
    CU_ASSERT(0 == memcmp(first_frame, frame, 5));
    CU_ASSERT(5 == length);

    type = ws_make_frame("lo", 2, &frame, &length, WS_FRAME, FINAL_FRAME);
    CU_ASSERT(0 == memcmp(second_frame, frame, 4));
    CU_ASSERT(4 == length);
}

int main()
{
    CU_pSuite websocket_suite = NULL;

   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   websocket_suite = CU_add_suite("Websocket Suite", init_websocket_test_suite, NULL);
   if (NULL == websocket_suite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if ((NULL == CU_add_test(websocket_suite, "test nullhandshake",
                            test_nullhandshake)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket get frame type",
                            test_websocket_get_frame_type)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket check end frame",
                            test_websocket_check_end_frame)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket check masked",
                            test_websocket_check_masked)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket get payload length",
                            test_websocket_get_payload_length)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket extract mask",
                            test_websocket_extract_mask)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket extract payload",
                            test_websocket_extract_payload)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket extract masked payload",
                            test_websocket_extract_masked_payload)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket frames",
                            test_websocket_frames)) ||
       (NULL == CU_add_test(websocket_suite, "test get up to linefeed",
                            test_get_upto_linefeed)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket parse handshake",
                            test_websocket_parse_handshake)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket parse get handshake answer",
                            test_websocket_get_handshake_answer)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket make header",
                            test_websocket_make_header)) ||
       (NULL == CU_add_test(websocket_suite, "test websocket make frame",
                            test_websocket_make_frame)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }


   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
#endif
