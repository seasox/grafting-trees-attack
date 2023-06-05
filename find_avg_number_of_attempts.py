#!/usr/bin/env python3
import attack
import sys 
seed = 0
attempts = 0
for i in range(1,10000):
    _, rc = attack.attack("wotsp_faulty_sigmas.txt", 20, 100, seed)
    if rc == -1:
        print("ERROR: forgery failed!")
    else: 
        attempts += rc
    print(f"{rc};{attempts};{attempts/i};")
    sys.stdout.flush()
    seed += 1
