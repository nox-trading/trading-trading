#pragma once

static const double const_digits[] = { 1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0 };

namespace tools
{
    inline double int_price_to_double(int price, int digits)
    {
        return price / const_digits[digits];
    }
}