#include <fstream>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <boost/multi_array.hpp>
#include <boost/functional/hash.hpp>

using boost::indices;
using boost::extents;
typedef boost::multi_array_types::index_range range;

/**
 * Structures
 */
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
};

/**
 * Fonctions d'affichage
 */
std::ostream& operator<<(std::ostream& o, const Slice& s) {
    o << "Slice(" << s.i0 << ", " << s.i1 << ", " << s.j0 << ", " << s.j1 << ")";
    return o;
}

std::ostream& operator<<(std::ostream& o, const std::vector<Slice>& slices) {
    bool first = true;
    o << "[";

    for(const auto& slice : slices) {
        if(!first) {
            o << ", ";
        }

        o << slice;
        first = false;
    }

    o << "]";
    return o;
}

/**
 * Parsage
 */
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

/*
 * Pour std::unordered_map<std::vector<Slice>, int>
 */
bool operator==(const Slice& s1, const Slice& s2) {
    return s1.i0 == s2.i0 && s1.i1 == s2.i1 && s1.j0 == s2.j0 && s1.j1 == s2.j1;
}

std::size_t hash_value(const Slice& slice) {
    std::size_t seed = 0;
    boost::hash_combine(seed, slice.i0);
    boost::hash_combine(seed, slice.i1);
    boost::hash_combine(seed, slice.j0);
    boost::hash_combine(seed, slice.j1);
    return seed;
}

template <typename T>
struct container_hash {
    std::size_t operator()(const T& c) const {
        return boost::hash_range(c.begin(), c.end());
    }
};

/*
 * Solution en programmation dynamique
 */
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

void solution_dp(const Problem& problem) {
    /* scores[i][j] = meilleur score possible en ayant une part finissant en
     *                (i, j) en en utilisant uniquement (y, x) avec
     *                {(y, x) | 0 <= y <= i et 0 <= x <= (j si y == i sinon cols - 1)}
     */
    boost::multi_array<int, 2> scores(extents[problem.rows][problem.cols]);

    /* scores_possibilities[i][j] est une unordered_map avec pour clé
     * les partitionnements possibles entre (i - problem.s + 1, j) et (i, j) et
     * en valeur, le meilleur score possible
     */
    typedef std::unordered_map<std::vector<Slice>, int, container_hash<std::vector<Slice>>> scores_t;
    boost::multi_array<scores_t, 2> scores_possibilities(extents[problem.rows][problem.cols]);

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
            // calcul des parts possibles terminant en (i, j)
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

            // pour chaque part finissant en (i, j)
            for(const auto& slice : slices) {
                // on calcul scores_possibilities[i][j]
                
                // première possibilité : slice tout seul dans la zone de possibilité
                std::vector<Slice> merge = {slice};
                int best_score = slice.size();

                for(int prev_i = 0; prev_i <= i - problem.s + 1; ++prev_i) {
                    int end_prev_j = (prev_i == i - problem.s + 1) ? j - 1 : problem.cols - 1;

                    for(int prev_j = 0; prev_j <= end_prev_j; ++prev_j) {
                        best_score = std::max(best_score, slice.size() + scores[prev_i][prev_j]);
                    }
                }

                scores_possibilities[i][j][merge] = best_score;

                // pour chaque possibilité dans la zone de possibilité
                for(int prev_i = std::max(0, i - problem.s + 1); prev_i <= i; ++prev_i) {
                    int begin_prev_j = 0;
                    int end_prev_j = problem.cols - 1;

                    if(prev_i == i - problem.s + 1)
                        begin_prev_j = j;

                    if(prev_i == i)
                        end_prev_j = j - 1;

                    for(int prev_j = begin_prev_j; prev_j <= end_prev_j; ++prev_j) {
                        if(!slice.is_in(prev_i, prev_j)) {
                            // pour chaque fin de part précédente dans la zone de possibilité
                            for(const auto& item : scores_possibilities[prev_i][prev_j]) {
                                // pour chaque découpage
                                // TODO
                            }
                        }
                    }
                }
            }

            std::cerr << "(" << i << ", " << j << ")" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "error: missing argument" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ifstream::in);
    Problem problem = parse_problem(file);
    solution_dp(problem);
    return 0;
}
