#ifndef LITERAL_H
#define LITERAL_H

#include <iostream>
#include <string>

class Literal {
public:
    explicit Literal(int var);
    explicit Literal(bool constant);
    Literal(int var_num, bool neg);

    Literal(const Literal& other);

    int  var() const;
    bool neg() const;

    bool    operator==(const Literal& other) const;
    Literal operator!() const;

    friend std::ostream& operator<<(std::ostream& os, const Literal& obj);

private:
    int _var;
    bool _neg;
};

#endif
