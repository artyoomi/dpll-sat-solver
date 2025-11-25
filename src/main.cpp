#include <iostream>
#include <string>
#include <unordered_map>

#include "cnf.h"


namespace DPLL {

std::vector<Literal> find_pure_literals(CNF cnf)
{
    std::vector<Literal> pure_literals;

    /*  Values are assumed to be:
     *  1. Literal is pure
     *  2. Negotation value of literal
     */
    std::unordered_map<int, std::pair<bool, bool>> detector;

    for (size_t i = 0; i < cnf.size(); ++i) {
        for (size_t j = 0; j < cnf[i].size(); ++j) {
            int  var = cnf[i][j].var();
            bool neg = cnf[i][j].neg();

            // Literal already detected by detector
            if (detector.find(var) != detector.end()) {
                if (detector[var].first && detector[var].second != neg) {
                    detector[var].first = false;
                }
            } else {
                detector[var] = {true, neg};
            }
        }
    }

    for (const auto& detection : detector) {
        // Add only if it's still pure
        if (detection.second.first) {
            pure_literals.emplace_back(detection.first, detection.second.second);
        }
    }

    return pure_literals;
}

/**
 * Recursively solves SAT by DPLL algo and saves assignment in assignment argument.
 * Maybe it is not super optimal, but I like it's simplicity :)
 *
 * @param cnf formula to solve
 * @param assignment reference to vector in which correct literals assignment
 *                   saved during solving
 * @return satisfiable or unsatisfiable
 */
bool DPLL(CNF cnf,
          std::vector<Literal>& assignment)
{
    // Base case
    if (cnf.empty()) {
        return true;
    }

    // Simple heuristic - just use first literal in first clause
    Literal literal = cnf[0][0];

    bool                 still_solvable = true;
    std::vector<Literal> unit_clauses;

    // Standard branch
    CNF                  standard_branch_cnf        = cnf;
    std::vector<Literal> standard_branch_assignment = assignment;

    std::tie(std::ignore, unit_clauses) = standard_branch_cnf.apply(literal);
    standard_branch_assignment.emplace_back(literal);
    std::tie(still_solvable, std::ignore) = standard_branch_cnf.apply(unit_clauses);
    standard_branch_assignment.insert(standard_branch_assignment.end(),
                                      unit_clauses.begin(), unit_clauses.end());
    if (still_solvable && DPLL(standard_branch_cnf, standard_branch_assignment)) {
        assignment = standard_branch_assignment;
        return true;
    }

    // Opposite branch
    CNF                  opposite_branch_cnf        = cnf;
    std::vector<Literal> opposite_branch_assignment = assignment;
    std::tie(std::ignore, unit_clauses) = opposite_branch_cnf.apply(!literal);
    opposite_branch_assignment.emplace_back(!literal);
    std::tie(still_solvable, std::ignore) = opposite_branch_cnf.apply(unit_clauses);
    opposite_branch_assignment.insert(opposite_branch_assignment.end(),
                                      unit_clauses.begin(), unit_clauses.end());
    if (still_solvable && DPLL(opposite_branch_cnf, opposite_branch_assignment)) {
        assignment = opposite_branch_assignment;
        return true;
    }

    return false;
}

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
    if (cnf.empty()) {
        return {true, std::vector<Literal>()};
    }

    const std::vector<Literal>& pure_literals = find_pure_literals(cnf);
    /*  Pure literals are unnecessary anymore, so let's just delete them
     *  from formula.
     *  NOTE: Applying pure literals must not be backtracked, because it
     *  guaranteed that it will not make formula unsatisfiable. If it happened,
     *  then pure literals were found incorrectly.
     */
    for (const auto& literal : pure_literals) {
        /*  For now let's ignore unit clauses, they will be processed in more
         *  centralized manner below in main logic.
         */
        bool still_solvable;
        std::tie(still_solvable, std::ignore) = cnf.apply(literal);
        if (!still_solvable) {
            throw std::logic_error("CNF become unsatisfiable after applying pure literals");
        }
    }

    // Formula solved by pure literals, so just return them
    if (cnf.empty()) {
        return {true, pure_literals};
    }

    std::vector<Literal> assignment;
    if (DPLL(cnf, assignment)) {
        assignment.insert(assignment.end(), pure_literals.begin(), pure_literals.end());
        return {true, assignment};
    } else {
        return {false, std::vector<Literal>()};
    }
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
