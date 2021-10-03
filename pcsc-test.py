#!/usr/bin/env python3

import pcsclite
import binascii
import traceback

SELECT = [0x00, 0xA4, 0x04, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x02, 0x47, 0x10, 0x01]

try:
    context = pcsclite.Context()
    readers = context.list_readers()
    print("PCSC readers: " + ''.join(readers))
    reader = readers[0]
    print("Using reader: " + reader)

    card = context.connect(reader)

    data = card.transmit(bytes(SELECT))
    print(binascii.b2a_hex(data))
    print(data)

    card.disconnect()

    del card
    del context

except Exception as message:
    print("Exception: " + str(message))
    print(traceback.format_exc())
