#!/usr/bin/env python3
import sys


class Problem:
    def __init__(self, rows, cols, h, s, problem):
        self.rows = rows
        self.cols = cols
        self.h = h
        self.s = s
        self.problem = problem

    def coord_valid(self, x, y):
        return x >= 0 and x < self.rows and y >= 0 and y < self.cols


def parse(file):
        rows, cols, h, s = map(int, file.readline().strip().split())

        problem = []
        for _ in range(rows):
            problem.append(list(file.readline().strip()))

        return Problem(rows, cols, h, s, problem)


def found_slices(p):
    slices = []

    for i in range(p.rows):
        for j in range(p.cols):
            if p.problem[i][j] == 'H':
                # on cherche le jambon le plus proche
                c = 1
                while c < 3:
                    for x in range(j - c, j + c + 1):
                        # ligne du haut
                        if p.coord_valid(i - c, x) and p.problem[i - c][x] == 'H':
                            print('ligne haut i=%d j=%d c=%d jambon en (%d, %d)' % (i, j, c, i - c, x))

                        # ligne du bas
                        if p.coord_valid(i + c, x) and p.problem[i + c][x] == 'H':
                            print('ligne bas i=%d j=%d c=%d jambon en (%d, %d)' % (i, j, c, i + c, x))

                    for y in range(i - c, i + c + 1):
                        # colonne gauche
                        if p.coord_valid(y, j - c) and p.problem[y][j - c] == 'H':
                            print('colonne gauche i=%d j=%d c=%d jambon en (%d, %d)' % (i, j, c, y, j - c))

                        # colonne droite
                        if p.coord_valid(y, j + c) and p.problem[y][j + c] == 'H':
                            print('colonne droite i=%d j=%d c=%d jambon en (%d, %d)' % (i, j, c, y, j + c))
                    c += 1

if __name__ == '__main__':
    with open(sys.argv[1], 'r') as f:
        p = parse(f)
        found_slices(p)
