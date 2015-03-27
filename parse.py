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

    def found_closest_ham(self, i, j):
        ''' Retourne la liste des jambons les plus proches de (i, j) '''
        c = 1
        while True:
            positions = []

            for x in range(j - c, j + c + 1):
                # ligne du haut
                if self.coord_valid(i - c, x) and self.problem[i - c][x] == 'H':
                    positions.append((i - c, x))

                # ligne du bas
                if self.coord_valid(i + c, x) and self.problem[i + c][x] == 'H':
                    positions.append((i + c, x))

            for y in range(i - c, i + c + 1):
                # colonne gauche
                if self.coord_valid(y, j - c) and self.problem[y][j - c] == 'H':
                    positions.append((y, j - c))

                # colonne droite
                if self.coord_valid(y, j + c) and self.problem[y][j + c] == 'H':
                    positions.append((y, j + c))

            if positions:
                return positions

            c += 1

    def count_ham(self, topleft_x, topleft_y, bottomright_x, bottomright_y):
        c = 0
        for x in range(topleft_x, bottomright_x + 1):
            for y in range(topleft_y, bottomright_y + 1):
                if self.problem[x][y] == 'H':
                    c += 1

        return c

def parse(file):
        rows, cols, h, s = map(int, file.readline().strip().split())

        problem = []
        for _ in range(rows):
            problem.append(list(file.readline().strip()))

        return Problem(rows, cols, h, s, problem)


if __name__ == '__main__':
    with open(sys.argv[1], 'r') as f:
        p = parse(f)
