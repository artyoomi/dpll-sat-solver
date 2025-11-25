#ifndef CLAUSE_H
#define CLAUSE_H

#include <vector>

#include "literal.h"

class Clause {
public:
    Clause(const std::vector<Literal>& literals);

    // Getters
    size_t size() const;
    bool   empty() const;

    const Literal& operator[](size_t index) const;
    Literal&       operator[](size_t index);

    bool apply_literal(const Literal& literal);

    friend std::ostream& operator<<(std::ostream& os, const Clause& obj);

private:
    std::vector<Literal> _data;
};

std::ostream& operator<<(std::ostream& os, const Clause& obj);

#endif
