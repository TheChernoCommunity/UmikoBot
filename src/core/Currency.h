#pragma once
#include "UmikoBot.h"

class Currency {
private:
    int m_Units = 0;
    int m_Cents = 0;

public:
    Currency();
    Currency(double);
    Currency(const Currency&);

    int cents() const;
    int units() const;

    Currency& operator=(const Currency&);

    explicit operator double() const;

    Currency operator+(const Currency&) const;
    Currency operator-(const Currency&) const;
    Currency operator*(const Currency&) const;
    Currency operator/(const Currency&) const;

    void operator+=(const Currency&);
    void operator-=(const Currency&);
    void operator*=(const Currency&);
    void operator/=(const Currency&);

    bool operator>=(const Currency&) const;
    bool operator<=(const Currency&) const;
    bool operator>(const Currency&) const;
    bool operator<(const Currency&) const;
    bool operator==(const Currency&) const;
};