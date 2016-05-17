#include <stdexcept>
#include "lazy_string.h"
#include <iostream>

using namespace std;

lazy_string::lazy_string() {
    (*this).kawaii = string();
    (*this).beginning = (*this).sz = 0;
}

lazy_string lazy_string::substr(size_t position, size_t size) {
    if (position > sz)
        throw std::out_of_range("lazy_string");
    return lazy_string(
            kawaii,
            position + size > sz ? (sz - position) : size,
            beginning + position
    );
}

const char& lazy_string::at(size_t position) const {
    if (position>=sz)
        throw std::out_of_range("lazy_string");
    return (*this).kawaii[(*this).beginning + position];
}

const char& lazy_string::operator[](size_t position) const {
    return at(position);
}

istream& operator>>(istream &input, lazy_string &lazy) {
    input >> lazy.kawaii;
    lazy.beginning = 0;
    lazy.sz = lazy.kawaii.size();
    return input;
}

ostream& operator<<(ostream &output, lazy_string &lazy) {
    for(size_t x = lazy.beginning, i = 0; i < lazy.sz; x++, i++)
        output << lazy[x];
    return output;
}

size_t lazy_string::size() const {
    return (*this).sz;
}

size_t lazy_string::length() const {
    return (*this).sz;
}

lazy_string::lazy_string(const string &str) {
    (*this).kawaii = str;
    (*this).beginning = 0;
    (*this).sz = str.size();
}

lazy_string::lazy_string(const lazy_string &lazy, size_t beginning, size_t size) {
    (*this).kawaii = lazy.kawaii;
    (*this).beginning = beginning;
    (*this).sz = size;
}



