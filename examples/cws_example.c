#include <stddef.h>
#include <stdint.h>
#include <cws.h>

int
    main(int argc, char **argv)
{
    uint8_t handshake[] = "GET /mychat HTTP/1.1\r\n"
                          "Host: server.example.com\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
                          "Sec-WebSocket-Protocol: chat\r\n"
                          "Sec-WebSocket-Version: 13\r\n"
                          "Origin: http://example.com\r\n";
    uint8_t hello_ws_frame[] = {0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f};
    size_t input_len = 7;

    struct handshake hs;
    enum ws_frame_type type;

    //Init handshake struct;

    nullhandshake(&hs);

    //After detecting a request for an upgrade, parse the handshake.

    type = ws_parse_handshake(handshake, input_len, &hs);

    uint8_t *out;
    size_t out_len;
    out = (uint8_t*) malloc(sizeof(uint8_t) * 4096);

    type = ws_get_handshake_answer(&hs, out, &out_len);

    //send to client answer pointed by out
    //parse input frames

    type = ws_parse_input_frame(hello_ws_frame, input_len, out, &out_len);

    uint8_t *out_frame;
    size_t out_frame_len;
    type = ws_make_frame("Hello", 5,
                         &out_frame, &out_frame_len,
                         WS_TEXT_FRAME, FINAL_FRAME);

    printf("%d frame type", type);

    return 0;
}
