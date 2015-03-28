#!/usr/bin/env python3
import sys
import copy


class Problem:
    def __init__(self, rows, cols, h, s, problem):
        self.rows = rows
        self.cols = cols
        self.h = h
        self.s = s
        self.problem = problem

    def coord_valid(self, x, y):
        return x >= 0 and x < self.rows and y >= 0 and y < self.cols


class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y


class Slice:
    def __init__(self, j1, j2):
        self.j1 = min(j1, j2)
        self.j2 = max(j1, j2)

    def size(self):
        return self.j2 - self.j1 + 1

    def is_in(self, j):
        return self.j1 <= j and j <= self.j2

    def __repr__(self):
        return 'Slice(%d, %d)' % (self.j1, self.j2)


class Rect:
    def __init__(self, p1, p2):
        self.p1 = p1
        self.p2 = p2

    def size(self):
        return (self.p2.x - self.p1.x + 1) * (self.p2.y - self.p1.y + 1)


def solutions_row_dynprog(problem, i):
    values = [0 for _ in range(problem.cols)]
    parent = [None for _ in range(problem.cols)]

    for j2 in range(problem.cols):
        j1 = j2

        while j1 >= 0 and j2 - j1 + 1 <= problem.s:
            hams = sum(1 if problem.problem[i][j] == 'H' else 0 for j in range(j1, j2 + 1))
            if hams >= problem.h:
                if j1 == 0:
                    v = j2 - j1 + 1
                    p = Slice(j1, j2), -1

                    if v >= values[j2]:
                        values[j2] = v
                        parent[j2] = p
                else:
                    v = 0
                    p = None

                    for j in range(j1):
                        if values[j] + (j2 - j1 + 1) >= v:
                            v = values[j] + (j2 - j1 + 1)
                            p = Slice(j1, j2), j

                    if v > values[j2]:
                        values[j2] = v
                        parent[j2] = p

            j1 -= 1

    slices = []
    j = values.index(max(values))
    while j >= 0 and parent[j] is not None:
        slices.append(parent[j][0])
        j = parent[j][1]

    return slices


def solution(p):
    slices_row = [None for _ in range(p.rows)]

    for i in range(p.rows):
        slices_row[i] = solutions_row_dynprog(p, i)

    nb = 0
    result = ''
    for i in range(p.rows):
        slices = slices_row[i]
        if slices is not None:
            for s in slices:
                nb += 1
                result += '%d %d %d %d\n' % (i, s.j1, i, s.j2)

    with open('out', 'w') as f:
        f.write('%d\n' % nb)
        f.write(result)


def parse(file):
    rows, cols, h, s = map(int, file.readline().strip().split())

    problem = []
    for _ in range(rows):
        problem.append(list(file.readline().strip()))

    return Problem(rows, cols, h, s, problem)


if __name__ == '__main__':
    with open(sys.argv[1], 'r') as f:
        p = parse(f)
        solution(p)
