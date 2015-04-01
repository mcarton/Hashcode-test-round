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

    Slice() {}

    Slice(int i0, int i1, int j0, int j1): i0(i0), i1(i1), j0(j0), j1(j1) {
        assert(0 <= i0 && i0 <= i1 && 0 <= j0 && j0 <= j1 && "bad slice");
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
};

/**
 * Fonctions d'affichage
 */
template<typename T>
std::ostream& operator<<(std::ostream& o, const std::vector<T>& v) {
    bool first = true;
    o << "[";

    for(const auto& value : v) {
        if(!first) o << ", ";

        o << value;
        first = false;
    }

    o << "]";
    return o;
}

template<>
std::ostream& operator<<(std::ostream& o, const std::vector<Slice>& slices) {
    o << slices.size() << std::endl;

    for(const auto& slice : slices) {
        o << slice.i0 << " " << slice.j0 << " " << slice.i1 << " " << slice.j1 << std::endl;
    }

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
 * Pour std::unordered_map<std::vector<int>, …>
 */
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

/* mise en cache des ensemble de couple (w, h) des parts possibles */
std::vector<std::pair<int, int>> all_slices;

std::vector<Slice> partial_dp(const Problem& problem, const Slice& zone) {
    /* scores est une unordered_map avec pour clé un vecteur d'entier v de
     * taille zone.w() représentant un partitionnement, et en valeur le
     * meilleur score possible pour ce partitionnement.
     *
     * Pour un vecteur v représentant un partitionnement, v[j] est l'indice de
     * la première ligne disponible pour la colonne j + zone.j0
     */
    std::unordered_map<std::vector<int>, int, container_hash<std::vector<int>>> scores;
    std::unordered_map<std::vector<int>, std::pair<Slice, std::vector<int>>, container_hash<std::vector<int>>> moves;

    // init
    std::vector<int> position(zone.w(), zone.i0);
    scores[position] = 0;

    // ensemble des couples (w, h) des parts possibles
    if(all_slices.size() == 0) { // pas encore calculé
        for(int w = 1; w <= problem.s; ++w) {
            for(int h = 1; h <= problem.s; ++h) {
                if(w * h <= problem.s) {
                    all_slices.emplace_back(w, h);
                }
            }
        }
    }

    // programmation dynamique
    for(int i = zone.i0; i <= zone.i1; ++i) {
        for(int j = zone.j0; j <= zone.j1; ++j) {
            // calcul des parts possibles terminant en (i, j)
            std::vector<Slice> slices;
            slices.reserve(all_slices.size());

            for(const auto& square : all_slices) {
                if(j - square.first + 1 >= zone.j0 && i - square.second + 1 >= zone.i0) {
                    Slice slice(i - square.second + 1, i, j - square.first + 1, j);
                    if(count_ham(problem, slice) >= problem.h) {
                        slices.push_back(slice);
                    }
                }
            }

            // pour chaque part finissant en (i, j)
            for(const auto& slice : slices) {
                // première possibilité : slice tout seul
                for(int p = 0; p < zone.w(); ++p) {
                    if(slice.j0 <= zone.j0 + p && zone.j0 + p <= slice.j1) {
                        position[p] = slice.i1 + 1;
                    }
                    else {
                        position[p] = zone.i0;
                    }
                }

                if(scores.find(position) == scores.end()) {
                    scores[position] = slice.size();
                    moves[position] = std::make_pair(slice, std::vector<int>());
                }
                else if(slice.size() > scores[position]) {
                    scores[position] = slice.size();
                    moves[position] = std::make_pair(slice, std::vector<int>());
                }

                // les autres possibilités
                for(const auto& item : scores) {
                    const std::vector<int>& last_position = item.first;

                    // peut-on insérer slice dans la position ?
                    bool valid = true;
                    for(int p = slice.j0 - zone.j0; valid && p <= slice.j1 - zone.j0; ++p) {
                        if(slice.i0 < last_position[p]) {
                            valid = false;
                            break;
                        }
                    }

                    if(valid) {
                        for(int p = 0; p < zone.w(); ++p) {
                            if(slice.j0 <= p + zone.j0 && p + zone.j0 <= slice.j1) {
                                position[p] = slice.i1 + 1;
                            }
                            else {
                                position[p] = last_position[p];
                            }
                        }

                        if(scores.find(position) == scores.end()) {
                            scores[position] = item.second + slice.size();
                            moves[position] = std::make_pair(slice, last_position);
                        }
                        else if(item.second + slice.size() > scores[position]) {
                            scores[position] = item.second + slice.size();
                            moves[position] = std::make_pair(slice, last_position);
                        }
                    }
                }
            }
        }
    }

    // recherche du meilleur score
    int best_score = 0;
    position.clear();

    for(const auto& item : scores) {
        if(item.second > best_score) {
            best_score = item.second;
            position = item.first;
        }
    }

    std::vector<Slice> result;
    while(moves.find(position) != moves.end()) {
        const auto& move = moves[position];
        result.push_back(move.first);
        position = move.second;
    }

    return result;
}

void solution_dp(const Problem& problem) {
    /*
    const int square_max_size = 20; // square of size 20 * 20 max
    std::vector<int> max_size(square_max_size + 1);

    max_size[rows] = cols indique que pour `rows` ligne, on peut faire du partiel dp sur `cols` colonne au maximum
    max_size[1] = 46;
    max_size[2] = 29;
    max_size[3] = 20;
    max_size[4] = 16;
    max_size[5] = 15;
    max_size[6] = 12;
    max_size[7] = 11;
    max_size[8] = 11;
    max_size[9] = 9;
    max_size[10] = 9;
    max_size[11] = 8;
    max_size[12] = 8;
    max_size[13] = 8;
    max_size[14] = 8;
    max_size[15] = 7;
    max_size[16] = 7;
    max_size[17] = 7;
    max_size[18] = 7;
    max_size[19] = 7;
    max_size[20] = 6;
    */

    const int HEIGHT = 5;
    const int WIDTH = 15;

    std::vector<Slice> result;
    for(int i = 0; i < problem.rows; i += HEIGHT) {
        for(int j = 0; j < problem.cols; j += WIDTH) {
            const Slice slice(i, std::min(i + HEIGHT - 1, problem.rows - 1),
                              j, std::min(j + WIDTH - 1, problem.cols - 1));
            const std::vector<Slice> partial_solution = partial_dp(problem, slice);
            for(const auto& s : partial_solution) {
                result.push_back(s);
            }

            std::cerr << "(" << i << ", " << j << ")" << std::endl;
        }
    }

    std::cout << result;
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
