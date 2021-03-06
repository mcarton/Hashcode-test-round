#!/usr/bin/env python3

import sys


class Problem:
    def __init__(self, rows, cols, h, s, pizza):
        self.rows = rows
        self.cols = cols
        self.h = h # nombre minimum de jambon par part
        self.s = s # taille maximum d'une part
        self.pizza = pizza


def parse(file):
    rows, cols, h, s = map(int, file.readline().strip().split())

    pizza = []
    for _ in range(rows):
        pizza.append(list(file.readline().strip()))

    return Problem(rows, cols, h, s, pizza)


def generate_output(solution):
    result = '%d\n' % len(solution)
    for i0, i1, j0, j1 in solution:
        result += '%d %d %d %d\n' % (i0, j0, i1, j1)

    return result


def write_output(solution, filename):
    with open(filename, 'w') as f:
        f.write(generate_output(solution))


def count_ham(problem, i0, i1, j0, j1):
    # retourne le nombre de jambon dans la part (i0, j0) → (i1, j1)
    assert(i0 <= i1 and j0 <= j1)
    ham = 0

    for i in range(i0, i1 + 1):
        for j in range(j0, j1 + 1):
            if problem.pizza[i][j] == 'H':
                ham += 1

    return ham


def solution_dp(problem):
    # scores[i][j][l] = meilleur score possible en ayant une part finissant en
    #                   (i, j) et en utilisant la partie de la pizza:
    #                   ____________________________________________
    #                   |                                           |
    #                   |                                           |
    #                   |                                           |
    #                   |              _____________________________|
    #               ↑   |              |                            |
    #               |   |              |                            |
    #             l |   |              |                            |
    #               ↓   |______________| (i, j)                     |
    #
    scores = [[[0 for _ in range(problem.s + 1)] for _ in range(problem.cols)] for _ in range(problem.rows)]

    # moves[i][j][l] = (w, h), (prev_i, prev_j, prev_l)
    #                  pour le meilleur score possible (voir `scores`),
    #                  w * h est la taille de la part et [prev_i][prev_j][prev_l]
    #                  la position de la précédente part
    moves = [[[None for _ in range(problem.s + 1)] for _ in range(problem.cols)] for _ in range(problem.rows)]

    # ensembles des couples (w, h) des parts possibles
    all_slices = []
    for width in range(1, problem.s + 1):
        for height in range(1, problem.s + 1):
            if width * height <= problem.s:
                all_slices.append((width, height))

    for i in range(problem.rows):
        for j in range(problem.cols):
            # precalcul des parts possibles terminant en (i, j)
            slices = []
            for w, h in all_slices:
                if j - w + 1 >= 0 and i - h + 1 >= 0: # valid slice
                    if count_ham(problem, i - h + 1, i, j - w + 1, j) >= problem.h:
                        slices.append((w, h))

            for l in range(1, min(problem.s, i + 1) + 1):
                # on va calculer scores[i][j][l]
                best_score = 0
                best_move = None

                for w, h in slices:
                    i0, i1 = i - h + 1, i
                    j0, j1 = j - w + 1, j

                    if w * h > best_score:
                        best_score = w * h
                        best_move = (w, h), None

                    # pour chaque position de la part précédente
                    for prev_i in range(i, -1, -1): # [i, i-1, …, 0]
                        start_prev_j = problem.cols - 1 # all cols
                        if i0 <= prev_i:
                            start_prev_j = j0 - 1
                        elif i - l + 1 <= prev_i:
                            start_prev_j = j1

                        for prev_j in range(start_prev_j, -1, -1):
                            prev_l = max(1, max(l, h) - (i - prev_i))
                            if w * h + scores[prev_i][prev_j][prev_l] > best_score:
                                best_score = w * h + scores[prev_i][prev_j][prev_l]
                                best_move = (w, h), (prev_i, prev_j, prev_l)

                scores[i][j][l] = best_score
                moves[i][j][l] = best_move
                assert(scores[i][j][l] <= problem.cols * (i - l + 1) + l * (j + 1))

            print('(%d, %d) max score %d' % (i, j, max(scores[i][j])))

    # recherche du meilleur score
    best_score = 0
    best_end = None
    for i in range(problem.rows):
        for j in range(problem.cols):
            for l in range(1, problem.s + 1):
                if scores[i][j][l] > best_score:
                    best_score = scores[i][j][l]
                    best_end = (i, j, l)

    print('best_score: %d position: %s' % (best_score, best_end))

    # construction de la solution
    slices = []
    pos = best_end
    while pos is not None:
        i, j, l = pos
        move = moves[i][j][l]
        if move is not None:
            (w, h), pos = move
            slices.append((i - h + 1, i, j - w + 1, j))
        else:
            break

    write_output(slices, '%d.out' % best_score)


if __name__ == '__main__':
    with open(sys.argv[1], 'r') as f:
        problem = parse(f)
        solution_dp(problem)
