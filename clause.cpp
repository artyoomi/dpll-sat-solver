#include "clause.h"

Clause::Clause(const std::vector<Literal>& literals)
: _data{literals}
{}

size_t Clause::size() const
{
    return this->_data.size();
}

bool Clause::empty() const
{
    return this->_data.empty();
}

const Literal& Clause::operator[](size_t index) const
{
    return this->_data.at(index);
}

Literal& Clause::operator[](size_t index)
{
    return this->_data.at(index);
}

bool Clause::apply_literal(const Literal& literal)
{
    for (size_t i = 0; i < this->_data.size(); ++i) {
        if (this->_data[i].var() == literal.var()) {
            // X = 1 if X in clause => clause satisfied
            if (this->_data[i].neg() == literal.neg()) {
                this->_data = std::vector<Literal>({Literal(true)});
                return true;
            // X = 1 if (not X) in clause => (not X) = 0 => erase (not X) from clause
            } else {
                this->_data.erase(this->_data.begin() + i);
                return false;
            }
        }
    }

    // If literal not found in clause
    return false;
}

std::ostream& operator<<(std::ostream& os, const Clause& obj)
{
    std::string or_delim = "or";

    os << "(";
    for (size_t i = 0, cl_size = obj._data.size(); i < cl_size; ++i) {
        os << obj._data[i];
        if (i != cl_size - 1) { os << " " + or_delim + " "; }
    }
    os << ")";

    return os;
}

