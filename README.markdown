cws
===

c websocket library

[![Build Status](https://secure.travis-ci.org/felipecruz/cws.png)](http://travis-ci.org/felipecruz/cws)

How to Use it
-------------

You can embed cws into your project.

API
---

This is the API exposed by cws:

```c
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

```

Examples
--------

Parsing handshakes:

```c
struct handshake hs;
enum ws_frame_type type;

nullhandshake(&hs);
type = ws_parse_handshake(handshake, input_len, &hs);
```

Get handshake answer:

```c
uint8_t *out;
size_t out_len;
out = (uint8_t*) malloc(sizeof(uint8_t) * 4096);

type = ws_get_handshake_answer(&hs, out, &out_len);
```

Parse Websocket frames:

```c
uint8_t *out;
size_t out_len;

type = ws_parse_input_frame(input_frame, input_len, out, &out_len);
```

Make frames:

```c
uint8_t *out_frame;
size_t out_frame_len;

type = ws_make_frame("Hello", 5, &out_frame, &out_frame_len,
                                 WS_TEXT_FRAME, FINAL_FRAME);
```

Testing
-------

In order to test, you'll have to install CUnit (http://cunit.sourceforge.net/).

`make test`

Coverage Report
---------------

In order to run coverage reports you'll have to install "lcov" package.

`make coverage`

Then, open `coverage/index.html`

Contact
-------
send me an email: felipecruz@loogica.net
