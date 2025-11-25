#include "cnf.h"

CNF::CNF(const std::string& filename)
{
    std::tie(this->_vars_count, this->_data) = this->_parse_dimacs(filename);
}

std::pair<int, std::vector<Clause>> CNF::_parse_dimacs(const std::string& filename)
{
    std::ifstream f_in(filename);
    if (!f_in.is_open()) {
        throw std::runtime_error("Cannot open DIMACS file " + filename);
    }

    std::vector<Clause> clauses;
    bool                config_appeared      = false;
    int                 config_vars_count    = 0;
    int                 config_clauses_count = 0;

    // Count of appeared clauses
    int         count = 0;
    std::string line;
    while (std::getline(f_in, line)) {
        if (line.empty() || line[0] == 'c') {
            continue;
        }

        if (line[0] == 'p') {
            std::istringstream iss(line);

            /*  Line starting with p must have following layout:
                "p cnf 3 2", where "cnf" - type of logical formula, "3" - amount
                of variables and "2" - amount of clauses.
            */
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

            config_appeared = true;

            std::string formula_type;
            iss >> formula_type;
            assert(formula_type == "cnf" &&
                   "Unsupported formula type, supported: cnf");

            iss >> config_vars_count;
            assert(config_vars_count > 0 &&
                   "Variables count must be positive");

            iss >> config_clauses_count;
            assert(config_clauses_count > 0 &&
                   "Clauses count must be positive");

            continue;
        }

        std::vector<Literal> literals;

        // Parse line
        int var = -1;
        std::istringstream iss(line);
        while (iss >> var && config_appeared && count < config_clauses_count) {
            ++count;

            if (var == 0) {
                assert(!literals.empty() && "Empty clause in input file");
                break;
            }

            assert(std::abs(var) <= config_vars_count &&
                   "Unknown variable appeared in input file");

            literals.emplace_back(Literal(var));

        }

        clauses.emplace_back(literals);
    }

    return {config_vars_count, clauses};
}

size_t CNF::size() const
{
    return this->_data.size();
}

bool CNF::empty() const
{
    return this->_data.empty();
}

int CNF::vars_count() const
{
    return this->_vars_count;
}

const Clause& CNF::operator[](size_t index) const
{
    return this->_data.at(index);
}

Clause& CNF::operator[](size_t index)
{
    return this->_data.at(index);
}

/**
 * Apply literal to formula and simplify if available. It also searches
 * for unit and empty clauses.
 *
 * @param literal literal to apply
 * @return returns true and unit clauses vector if formula still solvable, else
 *         false and empty unit clauses vector
 */
std::pair<bool, std::vector<Literal>> CNF::apply(const Literal& literal)
{
    if (literal.var() < 0 || literal.var() > this->_vars_count) {
        throw std::invalid_argument("Trying to apply unknown literal");
    }

    // To detect opposite literals in unit clauses
    std::unordered_map<int, bool> detector;

    bool                 still_solvable = true;
    std::vector<Literal> unit_clauses;

    for (size_t i = 0; i < this->_data.size(); ++i) {
        bool curr_result = this->_data[i].apply_literal(literal);

        // If current literal solved clause -> this clause can be deleted from formula
        if (curr_result) {
            this->_data.erase(this->_data.begin() + i);
        // Empty clause detected -> need to backtrack
        } else if (this->_data[i].empty()) {
            still_solvable = false;
            unit_clauses.clear();
            goto finalize;
        // Capture unit clause literal to return it from function
        } else if (this->_data[i].size() == 1) {
            // If such unit clause not detected yet -> just add it
            if (detector.find(this->_data[i][0].var()) == detector.end()) {
                detector[this->_data[i][0].var()] = this->_data[i][0].neg();
                unit_clauses.emplace_back(this->_data[i][0]);
            // If such literal already was detected, but with different
            // negotiation -> it is not solvable anymore
            } else if (detector[this->_data[i][0].var()] != this->_data[i][0].neg()) {
                still_solvable = false;
                unit_clauses.clear();
                goto finalize;
            }
        }
    }
finalize:
    return {still_solvable, unit_clauses};
}

std::pair<bool, std::vector<Literal>> CNF::apply(const std::vector<Literal>& literals)
{
    // Detects if within unit clauses exists opposite
    std::unordered_map<int, bool> detector;

    bool still_solvable = true;
    std::vector<Literal> unit_clauses;

    /*  Apply function for single literal detects all unit clauses. So we need
     *  to capture only the last unit clauses returned by this function.
     */
    for (const auto& literal : literals) {
        std::tie(still_solvable, unit_clauses) = apply(literal);
        if (!still_solvable) {
            break;
        }
    }

    return {still_solvable, unit_clauses};
}

std::ostream& operator<<(std::ostream& os, const CNF& obj)
{
    std::string and_delim = "^";

    for (size_t i = 0, cnf_size = obj._data.size(); i < cnf_size; ++i) {
        os << obj._data[i];
        if (i != cnf_size - 1) { os << " " + and_delim + " "; }
    }

    return os;
}
