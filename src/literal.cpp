#include "literal.h"

Literal::Literal(int var)
: _var{std::abs(var)},
  _neg{var < 0}
{}

Literal::Literal(bool constant)
: _var{0},
  _neg{!constant}
{}

Literal::Literal(int var_num, bool neg)
: _neg{neg}
{
    if (var_num <= 0)
        throw std::invalid_argument("Var cannot be non-positive");
    _var = var_num;
}

Literal::Literal(const Literal& other)
{
    this->_var = other._var;
    this->_neg = other._neg;
}

int Literal::var() const
{
    return this->_var;
}

bool Literal::neg() const
{
    return this->_neg;
}

bool Literal::operator==(const Literal& other) const
{
    return this->_var == other._var && this->_neg == other._neg;
}

Literal Literal::operator!() const
{
    return Literal(this->_var, !this->_neg);
}

std::ostream& operator<<(std::ostream& os, const Literal& obj)
{
    if (obj._var == 0) {
        os << ((obj._neg) ? ("F") : ("T"));
    } else {
        os << ((obj._neg) ? ("-") : ("")) + std::to_string(obj._var);
    }

    return os;
}
