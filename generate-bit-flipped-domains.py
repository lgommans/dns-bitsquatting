#!/usr/bin/env python3

import sys, string

s = sys.argv[1]

for i, c in enumerate(s):
    for bit in range(8):
        flippedchar = chr(ord(s[i]) ^ int(2**bit))
        if flippedchar not in string.ascii_lowercase + string.digits + '-.':
            continue

        #print('parts={}, {}, {}  flipped={}'.format((s[:i] if i > 0 else ''), s[i], (s[i + 1:] if i < len(s) - 1 else ''), flippedchar))
        flipped = (s[:i] if i > 0 else '') + flippedchar + (s[i + 1:] if i < len(s) - 1 else '')
        #sys.stdout.write(' '.join(format(ord(x), 'b').zfill(8) for x in flipped))
        #sys.stdout.write(' ')
        print(flipped)

