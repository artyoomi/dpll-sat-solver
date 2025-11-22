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
    int                 config_vars_count;
    int                 config_clauses_count;

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

            std::string formula_type;
            iss >> formula_type;
            if (formula_type != "cnf") {
                throw std::invalid_argument(
                    "Unsupported formula type, supported: cnf"
                );
            }
            iss >> config_vars_count;
            iss >> config_clauses_count;

            continue;
        }

        std::vector<Literal> literals;

        // Parse line
        int var = -1;
        std::istringstream iss(line);
        while (iss >> var) {
            if (var == 0) {
                assert(!literals.empty() && "Empty clause in input file");
                break;
            }

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
 * @return true if CNF still can be solved, else false (need to backtrack)
 */
std::pair<bool, std::vector<Literal>> CNF::apply_literal(const Literal& literal)
{
    if (literal.var() < 0 || literal.var() > this->_vars_count) {
        throw std::invalid_argument("Trying to apply unknown literal");
    }

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
            goto finalize;
        // Capture unit clause literal
        } else if (this->_data[i].size() == 1) {
            unit_clauses.emplace_back(this->_data[i][0]);
        }
    }
finalize:
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
