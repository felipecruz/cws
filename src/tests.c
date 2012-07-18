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
    const uint8_t small_frame[] = {0x89};
    enum ws_frame_type type;

    type = ws_parse_input_frame(NULL, 0, NULL, 0);
    CU_ASSERT(WS_ERROR_FRAME == type);

    type = ws_parse_input_frame(NULL, 10, NULL, 10);
    CU_ASSERT(WS_ERROR_FRAME == type);

    type = ws_parse_input_frame(small_frame, 1, NULL, 2);
    CU_ASSERT(WS_INCOMPLETE_FRAME == type);

    fd = open("tests/ws_frame.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 10);

    read(fd, frame1, 10, 0);

    type = ws_parse_input_frame(frame1, 10, NULL, 0);

    CU_ASSERT(WS_TEXT_FRAME == type);

    free(frame1);
    close(fd);

    fd = open("tests/ws_frame2.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 10);

    read(fd, frame1, 10, 0);

    type = ws_parse_input_frame(frame1, 10, NULL, 0);

    CU_ASSERT(WS_TEXT_FRAME == type);

    free(frame1);
    close(fd);

    fd = open("tests/ws_frame3.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 961);

    read(fd, frame1, 10, 0);

    type = ws_parse_input_frame(frame1, 961, NULL, 0);

    CU_ASSERT(WS_TEXT_FRAME == type);
    // 961 (total payload size) - 4 (fin, srvs, opcode, payload length) bytes
    CU_ASSERT(961-4 == _payload_length(frame1));
    CU_ASSERT(NULL != extract_payload(frame1));

    free(frame1);
    close(fd);

    fd = open("tests/ws_frame4.txt", O_RDONLY);
    frame1 = malloc(sizeof(uint8_t) * 90402);

    read(fd, frame1, 90402, 0);

    type = ws_parse_input_frame(frame1, 90402, NULL, 0);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(90402-10 == _payload_length(frame1));
    CU_ASSERT(NULL != extract_payload(frame1));

    free(frame1);
    close(fd);

    type = ws_parse_input_frame(client_big_masked_frame, 90405, NULL, 0);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(90404-14 == _payload_length(client_big_masked_frame));
    CU_ASSERT(NULL != _extract_mask_len3(client_big_masked_frame));
    CU_ASSERT(NULL != extract_payload(client_big_masked_frame));

    type = ws_parse_input_frame(client_medium_masked_frame, 90405, NULL, 0);

    CU_ASSERT(WS_TEXT_FRAME == type);
    CU_ASSERT(333-14 == _payload_length(client_medium_masked_frame));
    CU_ASSERT(NULL != _extract_mask_len3(client_medium_masked_frame));
    CU_ASSERT(NULL != extract_payload(client_medium_masked_frame));
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
    CU_ASSERT(NULL == hs.key2);
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
    CU_ASSERT(WS_TEXT_FRAME == type(first_frame));
    CU_ASSERT(WS_TEXT_FRAME != type(second_frame));
    CU_ASSERT(WS_INCOMPLETE_FRAME == type(second_frame));
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
    CU_ASSERT(0 == strncmp((char*) _extract_mask_len1(single_frame_masked),
                           (char*) mask, 4));

    CU_ASSERT(0 == strncmp((char*) _extract_mask_len2(len_256_masked),
                           (char*) mask, 4));

    CU_ASSERT(0 == strncmp((char*) _extract_mask_len3(client_big_masked_frame),
                           (char*) big_data_mask, 4));

    CU_ASSERT(0 == strncmp((char*) _extract_mask_len3(client_medium_masked_frame),
                           (char*) medium_data_mask, 4));
}

void
    test_websocket_extract_payload(void)
{
    CU_ASSERT(0 == strncmp((char*) extract_payload(single_frame),
                            "Hello", 5));

    CU_ASSERT(0 == strncmp((char*) extract_payload(unmasked_ping),
                            "Hello", 5));
}

void
    test_websocket_extract_masked_payload(void)
{
    CU_ASSERT(0 == strncmp((char*) extract_payload(single_frame_masked),
                            "Hello", 5));

    CU_ASSERT(0 == strncmp((char*) extract_payload(masked_pong),
                            "Hello", 5));
}

void
    test_get_upto_linefeed(void)
{
    CU_ASSERT(0 == strcmp((char*)get_upto_linefeed("GET /?encoding=text HTTP/1.1\r\n"),
                          "GET /?encoding=text HTTP/1.1"));

    CU_ASSERT(0 == strcmp((char*)get_upto_linefeed("Upgrade: websocket\r\n"),
                          "Upgrade: websocket"));

    CU_ASSERT(0 == strcmp((char*)get_upto_linefeed("Sec-WebSocket-Key: rRec6RPAbwPWLEsSQpGDKA==\r\n"),
                          "Sec-WebSocket-Key: rRec6RPAbwPWLEsSQpGDKA=="));
}

void
    test_websocket_parse_handshake(void)
{

}

void
    test_websocket_get_handshake_answer(void)
{
    struct handshake hs;
    uint8_t *out;
    size_t len;
    char *key = "rRec6RPAbwPWLEsSQpGDKA==";

    nullhandshake(&hs);

    out = (uint8_t*) malloc(sizeof(uint8_t) * 4096);

    hs.key1 = key;
    ws_get_handshake_answer(&hs, out, &len);
    
    CU_ASSERT(0 == strcmp((char*)out,
                          "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: qVMgwBQ7DRv6Mxw0KMXrjlX6EQQ=\r\n\r\n"));
    CU_ASSERT(129 == len);
}

void test_websocket_make_frame(void)
{

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
