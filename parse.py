import sys


class Problem:
    def __init__(self, rows, cols, h, s, problem):
        self.rows = rows
        self.cols = cols
        self.h = h
        self.s = s
        self.problem = problem


def parse(file):
        rows, cols, h, s = map(int, file.readline().strip().split())

        problem = []
        for _ in range(rows):
            problem.append(list(file.readline().strip()))

        return Problem(rows, cols, h, s, problem)

with open(sys.argv[1], 'r') as f:
    print(parse(f).problem)
