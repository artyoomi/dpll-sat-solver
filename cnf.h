#ifndef CNF_H
#define CNF_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>

#include <cassert>

#include "clause.h"

class CNF {
public:
    explicit CNF(const std::string& filename);

    size_t size() const;
    bool   empty() const;
    int    vars_count() const;

    const Clause& operator[](size_t index) const;
    Clause&       operator[](size_t index);

    std::pair<bool, std::vector<Literal>> apply_literal(const Literal& literal);

    friend std::ostream& operator<<(std::ostream& os, const CNF& obj);

private:
    std::pair<int, std::vector<Clause>> _parse_dimacs(const std::string& filename);

private:
    std::vector<Clause> _data;
    int                 _vars_count;
};

std::ostream& operator<<(std::ostream& os, const CNF& obj);

#endif
