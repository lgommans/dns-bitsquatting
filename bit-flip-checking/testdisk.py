#!/usr/bin/env python3

path = '/dev/sdd'
filesize = 400e9
patternlength = 4096 - 1


import os, binascii

testrunnum = 0

while True:
    if testrunnum % 4 == 0:
        print('Generating new pattern.')
        pattern = os.urandom(patternlength)

    testrunnum += 1

    f = open(path, 'wb')
    wrotebytes = 0
    while wrotebytes < filesize - patternlength:
        f.write(pattern)
        wrotebytes += patternlength
    f.close()

    f = open(path, 'rb')
    readbytes = 0
    while readbytes < filesize - patternlength:
        r = f.read(patternlength)
        readbytes += patternlength
        if r != pattern:
            print('Changed! Original:')
            print(binascii.hexliy(pattern))
            print('Result:')
            print(binascii.hexliy(r))
            print('-------------')

    print('Test run {} complete.'.format(testrunnum))

