#include <fstream>
#include <iostream>
#include <boost/multi_array.hpp>

using boost::indices;
using boost::extents;
typedef boost::multi_array_types::index_range range;

struct Problem {
    int rows;
    int cols;
    int h; // nombre minimum de jambon par part
    int s; // taille maximum d'une part
    boost::multi_array<char, 2> pizza;

    Problem(int rows, int cols, int h, int s):
        rows(rows), cols(cols), h(h), s(s), pizza(extents[rows][cols])
    {}
};

Problem parse_problem(std::istream& f) {
    int rows, cols, h, s;
    f >> rows >> cols >> h >> s;

    Problem problem(rows, cols, h, s);

    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            f >> problem.pizza[i][j];
        }
    }

    return problem;
}

struct Slice {
    int i0;
    int i1;
    int j0;
    int j1;

    Slice(int i0, int i1, int j0, int j1): i0(i0), i1(i1), j0(j0), j1(j1) {
        assert(i0 <= i1 && j0 <= j1 && "bad slice");
    }

    int size() const {
        return (i1 - i0 + 1) * (j1 - j0 + 1);
    }

    int w() const {
        return j1 - j0 + 1;
    }

    int h() const {
        return i1 - i0 + 1;
    }

    bool is_in(int i, int j) const {
        return i0 <= i && i <= i1 && j0 <= j && j <= j1;
    }

    bool collide_with(const Slice& s) const {
        for(int i = s.i0; i <= s.i1; ++i) {
            for(int j = s.j0; j <= s.j1; ++j) {
                // for all (i, j) in s
                if(i0 <= i && i <= i1 && j0 <= j && j <= j1) {
                    return true;
                }
            }
        }

        return false;
    }
};

std::ostream& operator<<(std::ostream& o, const Slice& s) {
    o << "Slice(" << s.i0 << ", " << s.i1 << ", " << s.j0 << ", " << s.j1 << ")";
    return o;
}

std::ostream& operator<<(std::ostream& o, const std::vector<Slice>& slices) {
    o << slices.size() << std::endl;

    for(const auto& slice : slices) {
        o << slice.i0 << " " << slice.j0 << " " << slice.i1 << " " << slice.j1 << std::endl;
    }

    return o;
}

int count_ham(const Problem& problem, const Slice& slice) {
    int ham = 0;

    for(int i = slice.i0; i <= slice.i1; ++i) {
        for(int j = slice.j0; j <= slice.j1; ++j) {
            if(problem.pizza[i][j] == 'H') {
                ham++;
            }
        }
    }

    return ham;
}

// returne true si le slice peut être contenu dans la repartition
bool slice_fit(const std::vector<Slice>& slices, const Slice& slice) {
    for(const auto& s : slices) {
        if(s.collide_with(slice)) {
            return false;
        }
    }

    return true;
}

std::vector<Slice> solution_dp(const Problem& problem) {
    boost::multi_array<int, 3> scores(extents[problem.rows][problem.cols][problem.s + 1]);

    typedef std::pair<std::pair<int, int>, std::tuple<int, int, int>> move_t;
    const move_t invalid_move = std::make_pair(
                                    std::make_pair(-1, -1), // w, h
                                    std::make_tuple(-1, -1, -1) // i, j, l
                                    );
    boost::multi_array<move_t, 3> moves(extents[problem.rows][problem.cols][problem.s + 1]);

    for(int i = 0; i < problem.rows; ++i) {
        for(int j = 0; j < problem.cols; ++j) {
            for(int l = 0; l <= problem.s; ++l) {
                moves[i][j][l] = invalid_move;
            }
        }
    }

    // ensemble des couples (w, h) des parts possibles
    std::vector<std::pair<int, int>> all_slices;
    for(int w = 1; w <= problem.s; ++w) {
        for(int h = 1; h <= problem.s; ++h) {
            if(w * h <= problem.s) {
                all_slices.emplace_back(w, h);
            }
        }
    }

    // programmation dynamique
    for(int i = 0; i < problem.rows; ++i) {
        for(int j = 0; j < problem.cols; ++j) {
            // precalcul des parts possibles terminant en (i, j)
            std::vector<Slice> slices;
            slices.reserve(all_slices.size());

            for(const auto& square : all_slices) {
                if(j - square.first + 1 >= 0 && i - square.second + 1 >= 0) {
                    Slice slice(i - square.second + 1, i, j - square.first + 1, j);
                    if(count_ham(problem, slice) >= problem.h) {
                        slices.push_back(slice);
                    }
                }
            }

            for(int l = 1; l <= std::min(problem.s, i + 1); ++l) {
                // on va calculer scores[i][j][l]
                int best_score = 0;
                move_t best_move = invalid_move;

                for(const auto& slice : slices) {
                    if(slice.size() > best_score) {
                        best_score = slice.size();
                        best_move = std::make_pair(
                                        std::make_pair(slice.w(), slice.h()),
                                        std::make_tuple(-1, -1, -1)
                                        );
                    }

                    for(int prev_i = i; prev_i >= 0; --prev_i) {
                        int start_prev_j = problem.cols - 1;
                        if(slice.i0 <= prev_i) {
                            start_prev_j = slice.j0 - 1;
                        }
                        else if(i - l + 1 <= prev_i) {
                            start_prev_j = slice.j1;
                        }

                        for(int prev_j = start_prev_j; prev_j >= 0; --prev_j) {
                            int prev_l = std::max(1, std::max(l, slice.h()) - (i - prev_i));

                            if(slice.size() + scores[prev_i][prev_j][prev_l] > best_score) {
                                best_score = slice.size() + scores[prev_i][prev_j][prev_l];
                                best_move = std::make_pair(
                                                std::make_pair(slice.w(), slice.h()),
                                                std::make_tuple(prev_i, prev_j, prev_l)
                                                );
                            }
                        }
                    }
                }

                scores[i][j][l] = best_score;
                moves[i][j][l] = best_move;
                assert(scores[i][j][l] <= problem.cols * (i - l + 1) + l * (j + 1));
            }

            /* pour le debug */
            int s = 0;
            for(int l = 0; l <= problem.s; ++l) {
                s = std::max(s, scores[i][j][l]);
            }
            std::cerr << "(" << i << ", " << j << ") max score " << s << std::endl;
        }
    }

    // recherche du meilleur score
    int best_score = 0;
    std::tuple<int, int, int> best_end;
    for(int i = 0; i < problem.rows; ++i) {
        for(int j = 0; j < problem.cols; ++j) {
            for(int l = 1; l <= problem.s; ++l) {
                if(scores[i][j][l] > best_score) {
                    best_score = scores[i][j][l];
                    best_end = std::make_tuple(i, j, l);
                }
            }
        }
    }

    std::cerr << "best score " << best_score << " position ("
              << std::get<0>(best_end) << ", "
              << std::get<1>(best_end) << ", "
              << std::get<2>(best_end) << ")" << std::endl;

    // construction de la solution
    std::vector<Slice> result;
    std::tuple<int, int, int> pos = best_end;

    while(pos != std::make_tuple(-1, -1, -1)) {
        int i = std::get<0>(pos);
        int j = std::get<1>(pos);
        int l = std::get<2>(pos);
        move_t move = moves[i][j][l];

        if(move.first != std::make_pair(-1, -1)) {
            int w = move.first.first;
            int h = move.first.second;
            result.emplace_back(i - h + 1, i, j - w + 1, j);
        }

        pos = move.second;
    }

    // on place la ou on peut encore placer
    for(int i = 0; i < problem.rows; ++i) {
        for(int j = 0; j < problem.cols; ++j) {
            // precalcul des parts possibles terminant en (i, j)
            std::vector<Slice> slices;
            slices.reserve(all_slices.size());

            for(const auto& square : all_slices) {
                if(j - square.first + 1 >= 0 && i - square.second + 1 >= 0) {
                    Slice slice(i - square.second + 1, i, j - square.first + 1, j);
                    if(count_ham(problem, slice) >= problem.h && slice_fit(result, slice)) {
                        slices.push_back(slice);
                    }
                }
            }

            if(slices.size() > 0) {
                int index = 0;

                for(std::size_t p = 0; p < slices.size(); ++p) {
                    if(slices[p].size() > slices[index].size()) {
                        index = p;
                    }
                }

                std::cerr << "ajout de " << slices[index] << std::endl;
                result.push_back(slices[index]);
            }
        }
    }

    // on augmente les slices si possibles
    for(auto& slice : result) {
        while(slice.i0 > 0
                && slice.size() + slice.w() <= problem.s
                && slice_fit(result, Slice(slice.i0 - 1, slice.i0 - 1, slice.j0, slice.j1))) {
            std::cerr << "augmentation vers le haut de " << slice << std::endl;
            slice.i0--;
        }

        while(slice.i1 < problem.rows - 1
                && slice.size() + slice.w() <= problem.s
                && slice_fit(result, Slice(slice.i1 + 1, slice.i1 + 1, slice.j0, slice.j1))) {
            std::cerr << "augmentation vers le bas de " << slice << std::endl;
            slice.i1++;
        }

        while(slice.j0 > 0
                && slice.size() + slice.h() <= problem.s
                && slice_fit(result, Slice(slice.i0, slice.i1, slice.j0 - 1, slice.j0 - 1))) {
            std::cerr << "augmentation vers la gauche de " << slice << std::endl;
            slice.j0--;
        }

        while(slice.j1 < problem.cols - 1
                && slice.size() + slice.h() <= problem.s
                && slice_fit(result, Slice(slice.i0, slice.i1, slice.j1 + 1, slice.j1 + 1))) {
            std::cerr << "augmentation vers la droite " << slice << std::endl;
            slice.j1++;
        }
    }

    return result;
}


int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "error: missing argument" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ifstream::in);
    Problem problem = parse_problem(file);
    std::vector<Slice> solution = solution_dp(problem);
    std::cout << solution;

    return 0;
}
