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

/**
 * Fonctions d'affichage
 */
std::ostream& operator<<(std::ostream& o, const Slice& s) {
    o << "Slice(" << s.i0 << ", " << s.i1 << ", " << s.j0 << ", " << s.j1 << ")";
    return o;
}

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

/* mise en cache des résultats de partial_dp */
boost::multi_array<std::vector<Slice>, 4> cache_partial_dp;

const std::vector<Slice>& partial_dp(const Problem& problem, const Slice& zone) {
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

    if(cache_partial_dp.size() == 0) {
        // initialisation du cache
        cache_partial_dp.resize(extents[problem.rows][problem.cols][problem.rows][problem.cols]);
    }

    if(cache_partial_dp[zone.i0][zone.j0][zone.i1][zone.j1].size() > 0) {
        return cache_partial_dp[zone.i0][zone.j0][zone.i1][zone.j1];
    }

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
                    for(int p = slice.j0 - zone.j0; p <= slice.j1 - zone.j0; ++p) {
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

    cache_partial_dp[zone.i0][zone.j0][zone.i1][zone.j1] = std::move(result);
    return cache_partial_dp[zone.i0][zone.j0][zone.i1][zone.j1];
}

// returne true si le slice peut être contenu dans la solution
bool slice_fit(const std::vector<Slice>& solution, const Slice& slice) {
    for(const auto& s : solution) {
        if(s.collide_with(slice)) {
            return false;
        }
    }

    return true;
}

void optimize_solution(const Problem& problem, std::vector<Slice>& solution) {
    assert(all_slices.size() > 0);

    // on place des slices dans les trous
    for(int i = 0; i < problem.rows; ++i) {
        for(int j = 0; j < problem.cols; ++j) {
            // precalcul des parts possibles terminant en (i, j)
            std::vector<Slice> slices;
            slices.reserve(all_slices.size());

            for(const auto& square : all_slices) {
                if(j - square.first + 1 >= 0 && i - square.second + 1 >= 0) {
                    Slice slice(i - square.second + 1, i, j - square.first + 1, j);
                    if(count_ham(problem, slice) >= problem.h && slice_fit(solution, slice)) {
                        slices.push_back(slice);
                    }
                }
            }

            if(slices.size() > 0) {
                int index = 0;

                for(std::size_t k = 0; k < slices.size(); ++k) {
                    if(slices[k].size() > slices[index].size()) {
                        index = k;
                    }
                }

                std::cerr << "ajout de " << slices[index] << std::endl;
                solution.push_back(slices[index]);
            }
        }
    }

    // on augmente les slices si possibles
    for(auto& slice : solution) {
        while(slice.i0 > 0
                && slice.size() + slice.w() <= problem.s
                && slice_fit(solution, Slice(slice.i0 - 1, slice.i0 - 1, slice.j0, slice.j1))) {
            std::cerr << "augmentation vers le haut de " << slice << std::endl;
            slice.i0--;
        }

        while(slice.i1 < problem.rows - 1
                && slice.size() + slice.w() <= problem.s
                && slice_fit(solution, Slice(slice.i1 + 1, slice.i1 + 1, slice.j0, slice.j1))) {
            std::cerr << "augmentation vers le bas de " << slice << std::endl;
            slice.i1++;
        }

        while(slice.j0 > 0
                && slice.size() + slice.h() <= problem.s
                && slice_fit(solution, Slice(slice.i0, slice.i1, slice.j0 - 1, slice.j0 - 1))) {
            std::cerr << "augmentation vers la gauche de " << slice << std::endl;
            slice.j0--;
        }

        while(slice.j1 < problem.cols - 1
                && slice.size() + slice.h() <= problem.s
                && slice_fit(solution, Slice(slice.i0, slice.i1, slice.j1 + 1, slice.j1 + 1))) {
            std::cerr << "augmentation vers la droite " << slice << std::endl;
            slice.j1++;
        }
    }
}

int score(const std::vector<Slice>& solution) {
    int s = 0;

    for(const auto& slice : solution) {
        s += slice.size();
    }

    return s;
}

void solution_dp(const Problem& problem) {
    /* scores[i][j] = meilleur score possible en utilisant [0..i] * [0..j] */
    boost::multi_array<int, 2> scores(extents[problem.rows][problem.cols]);

    /* moves[i][j] = ((prev_i, prev_j), slices) */
    typedef std::pair<std::pair<int, int>, std::vector<Slice>> move_t;
    boost::multi_array<move_t, 2> moves(extents[problem.rows][problem.cols]);

    const int DP_MAX_WIDTH = 4;
    const int DP_MAX_HEIGHT = 12;

    // programmation dynamique par colonne
    for(int i = 0; i < problem.rows; ++i) {
        for(int j = 0; j < problem.cols; ++j) {
            scores[i][j] = 0;
            moves[i][j].first = std::make_pair(-1, -1);

            // on découpe notre espace en deux avec la colonne prev_j
            for(int prev_j = std::max(0, j - DP_MAX_WIDTH + 1); prev_j <= j; ++prev_j) {
                // programmation dynamique par ligne
                // score = scores[i][prev_j - 1] + meilleur score dans [prev_j..j]

                std::vector<int> scores_bloc(i + 1, 0);
                std::vector<std::pair<int, std::vector<Slice>>> moves_bloc(i + 1);

                for(int k = 0; k <= i; ++k) {
                    // calcul de scores_bloc[k]
                    scores_bloc[k] = 0;
                    moves_bloc[k].first = -1;

                    for(int prev_k = std::max(0, k - DP_MAX_HEIGHT + 1); prev_k <= k; ++prev_k) {
                        // score = score_bloc[prev_k - 1] + meilleur score dans [prev_k..k]
                        const std::vector<Slice>& solution = partial_dp(problem, Slice(prev_k, k, prev_j, j));
                        int s = score(solution);
                        if(prev_k > 0) {
                            s += scores_bloc[prev_k - 1];
                        }

                        if(s > scores_bloc[k]) {
                            scores_bloc[k] = s;
                            moves_bloc[k].first = prev_k - 1;
                            moves_bloc[k].second = solution;
                        }
                    }
                }

                int s = scores_bloc[i];
                if(prev_j > 0) {
                    s += scores[i][prev_j - 1];
                }

                if(s > scores[i][j]) {
                    scores[i][j] = s;
                    moves[i][j].first = std::make_pair(i, prev_j - 1);

                    /* on reconstruit la solution dans [prev_j..j] */
                    auto& slices = moves[i][j].second;
                    slices.clear();

                    int k = i;
                    while(k >= 0) {
                        auto& move = moves_bloc[k];
                        slices.insert(slices.end(),
                                      std::make_move_iterator(move.second.begin()),
                                      std::make_move_iterator(move.second.end()));
                        k = move.first;
                    }
                }
            }

            // on découpe notre espace en deux avec la ligne prev_i
            for(int prev_i = std::max(0, i - DP_MAX_WIDTH + 1); prev_i <= i; ++prev_i) {
                // programmation dynamique par colonne
                // score = scores[prev_i - 1][j] + meilleur score dans [prev_i..i]

                std::vector<int> scores_bloc(j + 1, 0);
                std::vector<std::pair<int, std::vector<Slice>>> moves_bloc(j + 1);

                for(int k = 0; k <= j; ++k) {
                    // calcul de scores_bloc[k]
                    scores_bloc[k] = 0;
                    moves_bloc[k].first = -1;

                    for(int prev_k = std::max(0, k - DP_MAX_HEIGHT + 1); prev_k <= k; ++prev_k) {
                        // score = score_bloc[prev_k - 1] + meilleur score dans [prev_k..k]
                        const std::vector<Slice>& solution = partial_dp(problem, Slice(prev_i, i, prev_k, k));
                        int s = score(solution);
                        if(prev_k > 0) {
                            s += scores_bloc[prev_k - 1];
                        }

                        if(s > scores_bloc[k]) {
                            scores_bloc[k] = s;
                            moves_bloc[k].first = prev_k - 1;
                            moves_bloc[k].second = solution;
                        }
                    }
                }

                int s = scores_bloc[j];
                if(prev_i > 0) {
                    s += scores[prev_i - 1][j];
                }

                if(s > scores[i][j]) {
                    scores[i][j] = s;
                    moves[i][j].first = std::make_pair(prev_i - 1, j);

                    /* on reconstruit la solution dans [prev_i..i] */
                    auto& slices = moves[i][j].second;
                    slices.clear();

                    int k = j;
                    while(k >= 0) {
                        auto& move = moves_bloc[k];
                        slices.insert(slices.end(),
                                      std::make_move_iterator(move.second.begin()),
                                      std::make_move_iterator(move.second.end()));
                        k = move.first;
                    }
                }
            }

            std::cerr << "(" << i << ", " << j << ") score " << scores[i][j] << std::endl;
        }
    }

    std::vector<Slice> solution;
    int i = problem.rows - 1;
    int j = problem.cols - 1;
    while(i >= 0 && j >= 0) {
        auto& move = moves[i][j];
        solution.insert(solution.end(),
                        std::make_move_iterator(move.second.begin()),
                        std::make_move_iterator(move.second.end()));
        i = move.first.first;
        j = move.first.second;
    }

    // dernière optimisation
    optimize_solution(problem, solution);

    std::cerr << "score " << score(solution) << std::endl;
    std::cout << solution;
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
