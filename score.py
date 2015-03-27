#!/usr/bin/env python3

import sys


def score(file):
    parts = int(file.readline())

    score = 0
    for _ in range(parts):
        r1, c1, r2, c2 = map(int, file.readline().strip().split())

        score += (r2-r1+1)*(c2-c1+1)

    return score


with open(sys.argv[1], 'r') as f:
    print(score(f))
