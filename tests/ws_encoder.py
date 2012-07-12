# -*- coding: utf-8 -*-
from struct import pack

def encode_hybi(buf, opcode):
    """ Encode a HyBi style WebSocket frame.
        Optional opcode:
            0x0 - continuation
            0x1 - text frame (base64 encode buf)
            0x2 - binary frame (use raw buf)
            0x8 - connection close
            0x9 - ping
            0xA - pong
    """

    b1 = 0x80 | (opcode & 0x0f) # FIN + opcode
    payload_len = len(buf)
    if payload_len <= 125:
        header = pack('>BB', b1, payload_len)
    elif payload_len > 125 and payload_len < 65536:
        header = pack('>BBH', b1, 126, payload_len)
    elif payload_len >= 65536:
        header = pack('>BBQ', b1, 127, payload_len)

    print("Encoded: %s" % repr(header + buf))

    return header + buf, len(header)

if __name__ == "__main__":
    #f = open('ws_frame2.txt', 'wb')
    f = open('ws_frame3.txt', 'wb')
    v = u'''
Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João vv Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João Felipe João
'''
    f.write(encode_hybi(v.encode('utf-8'), 0x1)[0])
    f.close()
