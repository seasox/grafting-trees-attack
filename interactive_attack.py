#!/usr/bin/env python3

import attack
import serial
import sys
import time

MSS_TREE_HEIGHT = 5
SPHINCS_BYTES = 32
SEED_BYTES = 32
SPHINCS_ADDRESS_BYTES = 8
WOTS_L = 67

def open_serial(tty, baudrate=115200, timeout=2):
    ser = serial.Serial(tty, baudrate, timeout=timeout)
    if not ser.is_open:
        return None
    return ser

def main():
    ser = open_serial(sys.argv[1] if len(sys.argv) > 1 else '/dev/cu.usbmodem101')
    if not ser:
        print('could not open serial')
        sys.exit(1)

    time.sleep(3)  # wait for device to boot

    # send \aa, expect \bb
    ser.write(b'\xaa')
    assert ser.read() == b'\xbb'

    # send count
    cnt = 200
    ser.write(int.to_bytes(cnt, 2, "big"))
    cnt_echo = int.from_bytes(ser.read(2), "big")
    assert cnt_echo == cnt

    # send delay
    del_ms = 0
    ser.write(int.to_bytes(del_ms, 2, "big"))
    del_ms_echo = int.from_bytes(ser.read(2), "big")
    assert del_ms_echo == del_ms

    # seed
    #seed = 'a82182f8e56ca9d2d1836c4e5bf9f86a63a74cf67e9a3818acc4ac46b82b250e'  # R1 (?) from full_sphincs_signature.txt
    #seed = bytes.fromhex(seed)
    seed = attack.in_sk1_bytes
    assert len(seed) == SEED_BYTES
    ser.write(seed)

    # addr
    #addr = 'e7c1d4f115363236'  # addr from full_sphincs_signature.txt
    #addr = bytes.fromhex(addr)
    addr = int.to_bytes(attack.subtree_A, SPHINCS_ADDRESS_BYTES, "big")
    assert len(addr) == SPHINCS_ADDRESS_BYTES
    ser.write(addr)

    seed_echo = ser.read(SEED_BYTES)
    assert seed_echo == seed

    addr_echo = ser.read(SPHINCS_ADDRESS_BYTES)
    assert addr_echo == addr

    print(f"(addr) {addr_echo.hex()}")
    print(f"(R1) {seed_echo.hex()}")

    f = open("interactive_faulty_sigmas.txt", "w")

    ign = 0

    sigmas = []
    for i in range(cnt):
        auth_path = ser.read(MSS_TREE_HEIGHT * SPHINCS_BYTES)
        assert len(auth_path) == MSS_TREE_HEIGHT * SPHINCS_BYTES
        #print(f"(path_{i}) {auth_path.hex()}")
        sigma = ser.read(WOTS_L * SPHINCS_BYTES)
        assert len(sigma) == WOTS_L * SPHINCS_BYTES
        sigmas += [sigma]
        #print(f"(sigma_{i}) {sigma.hex()}")
        print(f"{sigma.hex()}")
        if sigma.hex() == attack.exp_sigma:  # skip valid signatures
            ign += 1
        else:
            f.write(sigma.hex() + '\n')
        time.sleep(del_ms / 1000)

    f.close()

    from collections import Counter
    cntr = Counter(sigmas)
    print(cntr)

    print(f"generated {cnt-ign} faulty signatures ({(cnt-ign)/cnt} fault probability)")

    assert not ser.read()




if __name__ == '__main__':
    main()