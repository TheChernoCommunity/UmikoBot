#include "Currency.h"
#include <QtMath>

Currency::Currency() : m_Cents(0), m_Units(0)
{
}

Currency::Currency(double cents)
    : m_Cents(qFloor(cents * 100.0) % 100), m_Units(qFloor(cents))
{
}

Currency::Currency(const Currency& other)
    : m_Cents(other.m_Cents), m_Units(other.m_Units)
{   
}

int Currency::cents() const
{
    return m_Cents;
}

int Currency::units() const
{
    return m_Units;
}

Currency& Currency::operator=(const Currency& other)
{
    m_Units = other.m_Units;
    m_Cents = other.m_Cents;
    return *this;
}

Currency::operator double() const
{
    return (double)m_Cents / 100.0 + (double)m_Units;
}

Currency Currency::operator+(const Currency& other) const
{
    return (m_Cents + other.m_Cents) / 100 + m_Units + other.m_Units;
}
Currency Currency::operator-(const Currency& other) const
{
    return (m_Cents - other.m_Cents) / 100 + m_Units - other.m_Units;
}

Currency Currency::operator*(const Currency& other) const
{
    double tDouble = (double)*this;
    double oDouble = (double)other;
    return tDouble * oDouble;
}
Currency Currency::operator/(const Currency& other) const 
{
    double tDouble = (double)*this;
    double oDouble = (double)other;
    return tDouble / oDouble;
}

void Currency::operator+=(const Currency& other)
{
    m_Cents += other.m_Cents;
    m_Units += other.m_Units;
}

void Currency::operator-=(const Currency& other)
{
    m_Cents -= other.m_Cents;
    m_Units -= other.m_Units;
}

void Currency::operator*=(const Currency& other)
{
    double tDouble = (double)*this;
    double oDouble = (double)other;
    *(this) = tDouble * oDouble;
}

void Currency::operator/=(const Currency& other)
{
    double tDouble = (double)*this;
    double oDouble = (double)other;
    *(this) = tDouble / oDouble;
}

bool Currency::operator<(const Currency& other) const
{
    return (m_Cents + m_Units) < (other.m_Cents + other.m_Units);
}

bool Currency::operator>(const Currency& other) const
{
    return (m_Cents + m_Units) > (other.m_Cents + other.m_Units);
}

bool Currency::operator<=(const Currency& other) const
{
    return (m_Cents + m_Units) <= (other.m_Cents + other.m_Units);
}

bool Currency::operator>=(const Currency& other) const
{
    return (m_Cents + m_Units) >= (other.m_Cents + other.m_Units);
}

bool Currency::operator==(const Currency& other) const
{
    return (m_Cents + m_Units) == (other.m_Cents + other.m_Units);
}