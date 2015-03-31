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

void solution_dp(const Problem& problem) {
    /* scores[i][j] = meilleur score possible en ayant une part finissant en
     *                (i, j) en en utilisant uniquement les (y, x) avec
     *                0 <= y <= i et 0 <= x <= j
     */
    boost::multi_array<int, 2> scores(extents[problem.rows][problem.cols]);

    /* scores_possibilities[i][j] est une unordered_map avec pour clé
     * un vecteur d'entier v de taille maximum problem.s - 1, représentant un
     * partitionnement, et en valeur le meilleur score possible pour ce
     * partitionnement.
     *
     * Pour un vecteur v représentant un partitionnement, v[di] est l'indice de
     * la première colonne disponible dans la ligne i - 1 - di.
     */
    typedef std::unordered_map<std::vector<int>, int, container_hash<std::vector<int>>> scores_t;
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
