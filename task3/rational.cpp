#include "rational.h"

rational::rational(int temp) {
    num = temp;
    denom = 1;
}

rational::rational(int temp1, int temp2) {
    int v = gcd(temp1, temp2);
    num = temp1 / v;
    denom = temp2 / v;
}

int rational::gcd(int temp1, int temp2) {
    while (temp1 && temp2) {
        if (temp1 > temp2)
            temp1 %= temp2;
        else
            temp2 %= temp1;
    };
    return temp1 + temp2;
}

int rational::getNum() const {
    return num;
}
int rational::getDenom() const {
    return denom;
}

rational rational::operator+(rational const &b) const {
    return rational(num * b.denom + b.num * denom, denom * b.denom);
}

rational rational::operator-(rational const &b) const {
    return rational(num * b.denom - b.num * denom, denom * b.denom);
}

rational rational::operator*(rational const &b) const {
    return rational(num * b.num, denom * b.denom);
}

rational rational::operator/(rational const &b) const {
    return rational(num * b.denom, denom * b.num);
}
