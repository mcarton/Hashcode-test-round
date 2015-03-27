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


def solutions_row_rec(p, i, slices, slices_row):
    for j1 in range(p.cols):
        if p.problem[i][j1] != 'H':
            continue

        c = 0
        j2 = j1

        while c <= p.h and j2 < p.cols and j2 - j1 + 1 <= p.s:
            if any(s.is_in(j2) for s in slices):
                break

            if p.problem[i][j2] == 'H':
                c += 1

                if c == p.h:
                    new_slices = copy.deepcopy(slices)
                    new_slices.append(Slice(j1, j2))

                    if slices_row[i] is None:
                        slices_row[i] = new_slices
                    else:
                        if sum(s.size() for s in new_slices) > sum(s.size() for s in slices_row[i]):
                            slices_row[i] = new_slices

                    solutions_row_rec(p, i, new_slices, slices_row)

            j2 += 1


def solution(p):
    slices_row = [None for _ in range(p.rows)]

    for i in range(p.rows):
        solutions_row_rec(p, i, [], slices_row)
        print('row %i slices %s' % (i, slices_row[i]))

    nb = 0
    result = ''
    for i in range(p.rows):
        slices = slices_row[i]
        if slices is not None:
            for s in slices:
                nb += 1
                result += '%d %d %d %d\n' % (i, s.j1, i, s.j1)

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
