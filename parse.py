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


def found_closest_ham(p, i, j):
    ''' Retourne la liste des jambons les plus proches de (i, j) '''
    c = 1
    while True:
        positions = []

        for x in range(j - c, j + c + 1):
            # ligne du haut
            if p.coord_valid(i - c, x) and p.problem[i - c][x] == 'H':
                positions.append((i - c, x))

            # ligne du bas
            if p.coord_valid(i + c, x) and p.problem[i + c][x] == 'H':
                positions.append((i + c, x))

        for y in range(i - c, i + c + 1):
            # colonne gauche
            if p.coord_valid(y, j - c) and p.problem[y][j - c] == 'H':
                positions.append((y, j - c))

            # colonne droite
            if p.coord_valid(y, j + c) and p.problem[y][j + c] == 'H':
                positions.append((y, j + c))

        if positions:
            return positions

        c += 1


def found_slices(p):
    slices = []

    for i in range(p.rows):
        for j in range(p.cols):
            if p.problem[i][j] == 'H':
                print('(%d, %d) closests = %s' % (i, j, found_closest_ham(p, i, j)))


if __name__ == '__main__':
    with open(sys.argv[1], 'r') as f:
        p = parse(f)
        found_slices(p)
