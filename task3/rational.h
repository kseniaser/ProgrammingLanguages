#ifndef TASK3_RATIONAL_H
#define TASK3_RATIONAL_H

class rational
{

public:

    rational(int temp);
    rational(int temp1, int temp2);

    int getNum() const;
    int getDenom() const;

    rational operator -(rational const & temp2) const;
    rational operator +(rational const & temp2) const;
    rational operator *(rational const & temp2) const;
    rational operator /(rational const & temp2) const;

private:

    int num;
    int denom;
    int gcd(int, int);

};

#endif
