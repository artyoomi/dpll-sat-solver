#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>

#include "cnf.h"


namespace DPLL {

struct DPLL_node {
    std::vector<Literal> literals;
    CNF                  cnf;
};

std::vector<Literal> find_pure_literals(CNF cnf)
{
    std::vector<Literal> pure_literals;

    std::unordered_map<int, bool> detector;

    for (size_t i = 0; i < cnf.size(); ++i) {
        for (size_t j = 0; j < cnf[i].size(); ++j) {
            int  var = cnf[i][j].var();
            bool neg = cnf[i][j].neg();

            // Literal already detected by detector
            if (detector.find(var) != detector.end()) {
                // Found same literals, but with different negotiation values
                if (detector[var] != neg) {
                    detector.erase(var);
                }
            } else {
                detector[var] = neg;
            }
        }
    }

    for (const auto& detection : detector) {
        pure_literals.emplace_back(detection.first, detection.second);
    }

    return pure_literals;
}

// void unit_propogation(const CNF& cnf, const std::vector<Literal>& literals)
// {
//     std::vector<Literal> unit_literals = literals;

//     while (!unit_literals.empty()) {
//         ;
//     }
// }

/**
 * Solve CNF using DPLL algorithm.
 *
 * @param cnf CNF to solve
 * @return pair of calculation result and appropriate assignment
 *         All possible results:
 *         1. <true, non-empty assignment> - satisfiable only for given
 *            assignment (other correct assignments may exist)
 *         2. <true, empty assignment> - satisfiable for any assignment
 *         3. <false, any assignment> - not satisfiable
 */
std::pair<bool, std::vector<Literal>> solve(CNF cnf)
{
    bool                 satisfiable;
    std::vector<Literal> assignment;

    // If CNF is empty - it cannot be solved
    if (cnf.empty()) {
        return {false, assignment};
    }

    bool still_solvable;

    const std::vector<Literal>& pure_literals = find_pure_literals(cnf);
    /*  Pure literals are unnecessary anymore, so let's just delete them
     *  from formula.
     *  NOTE: Applying pure literals must not be backtracked, because it
     *  guaranteed that it will not make formula unsatisfiable. If it happened,
     *  then pure literals were found incorrectly.
     */
    for (const auto& literal : pure_literals) {
        std::vector<Literal> _;
        /*  For now let's ignore unit clauses, they will be processed in more
         *  centralized manner below in main logic.
         */
        std::tie(still_solvable, _) = cnf.apply_literal(literal);
        if (!still_solvable) {
            throw std::logic_error("CNF become unsatisfiable after applying pure literals");
        }
    }

    // Formula solved by pure literals, so just return them
    if (cnf.empty()) {
        return {true, pure_literals};
    }

    std::queue<DPLL_node> q;
    q.push({{cnf[0][0]}, cnf});

    while (true) {
        DPLL_node curr_state = q.front();

        q.push({{cnf[0][0]}, cnf});
        CNF curr_cnf         = curr_state.cnf;
        // Literal curr_literal = curr_state.literal;

        bool                 still_solvable;
        std::vector<Literal> unit_clauses;
        // std::tie(still_solvable, unit_clauses) = curr_cnf.apply_literal(curr_literal);

        if (!still_solvable) {
            q.pop();
            // <TODO> add logic to choose next literal
        }
            // for (const auto& unit_clause : unit_clauses) {
            //     curr_cnf.apply_literal(unit_clause);
            // }
        // Need to simplify formula with unit_clauses until they will not left
        while (!unit_clauses.empty()) {
            curr_cnf.apply_literal(unit_clauses.back());
            unit_clauses.pop_back();
        }
    }

    /*  Assignment should be extended here because if it will be extended
     *  above it will be needed to copy it multiple times to form
     *  DPLL_node's.
     */
    assignment.insert(assignment.end(), pure_literals.begin(), pure_literals.end());

    return {satisfiable, assignment};
}

}  // DPLL namespace

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string filename(argv[1]);

    CNF cnf(filename);
    std::cout << cnf << '\n';

    bool                 satisfiable;
    std::vector<Literal> assignment;
    std::tie(satisfiable, assignment) = DPLL::solve(cnf);

    if (satisfiable) {
        std::cout << "SAT\n";
        for (size_t i = 0; i < assignment.size(); ++i) {
            std::cout << assignment[i] << ' ';
        }
        putchar('\n');
    } else {
        std::cout << "UNSAT\n";
    }

    return 0;
}
