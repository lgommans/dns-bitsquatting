#!/usr/bin/env pypy

import sys, socket, hashlib, binascii

hashlen_bytes = 32  # The length of the chosen message digest, in bytes
msglen = 1200  # Bytes that should be in each packet
errlimit = 5000  # Die after this many errors

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', int(sys.argv[1])))

lastevo = None
lastdata = None
rounds = 0
errs = 0

def err():
    global errs
    if errs > errlimit:
        print('Exiting due to too many errors.')
        exit(5)
    errs += 1

while True:
    data = sock.recv(msglen)
    if len(data) != msglen:
        print('WARN: data length incorrect.')
        err()

    if data[0] != lastevo:
        print('New evo after {} rounds.'.format(rounds))
        rounds = 0

    if data[0] != lastevo or not data == lastdata:
        if data[-hashlen_bytes:] == hashlib.sha256(data[:-hashlen_bytes]).digest():
            if data != lastdata and data[0] == lastevo:
                print('Note: new data, same evo, but correct hash.')

            lastdata = data
            lastevo = data[0]
        else:
            err()
            print('Hash error in data:')
            print(binascii.hexlify(data))
            if data[0] == lastevo:
                print('Correct was:')
                print(binascii.hexlify(lastdata))
            else:
                lastdata = None

    rounds += 1

