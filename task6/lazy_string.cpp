#include <iostream>
#include "lazy_string.h"

using namespace std;

lazy_string::lazy_string() {

    this->kawaii = make_shared<string>();
    (*this).beginning =(*this).sz = 0;
}

lazy_string lazy_string::substr(size_t position, size_t size) {
    if (position > sz)
        throw std::out_of_range("no such position in string");
    return lazy_string(kawaii,
                       beginning + position,
                       position + size > sz ? (sz - position) : size);
}

lazy_string::lazy lazy_string::at(size_t position) {
    if (position >= sz)
        throw out_of_range("no such position in string");
    return lazy(this, position);
}

char lazy_string::at(size_t position) const {
    if (position >= sz)
        throw out_of_range("no such position in string");
    return (*kawaii)[beginning + position];
}

lazy_string::lazy lazy_string::operator[](size_t position) {
    return lazy(this, position);
}

char lazy_string::operator[](size_t position) const {
    return (*kawaii)[beginning + position];
}

istream &operator>>(istream &input, lazy_string &ls) {
    auto str = make_shared<string>();
    input >> *str;
    ls.kawaii = str;
    ls.sz = (*str).length();
    ls.beginning = 0;
    return input;
}

std::ostream &operator<<(std::ostream &output, lazy_string &ls) {
    for (size_t x = ls.beginning, i = 0; i < ls.size(); x++, i++)
        output << ls[x];
    return output;
}

lazy_string::lazy::lazy(lazy_string *lazyStr, size_t position) : lazyStr(lazyStr), position(position) { }

lazy_string::lazy& lazy_string::lazy::operator=(char c) {
    if ((*lazyStr).kawaii.use_count() > 1) {
        (*lazyStr).beginning = 0;
        (*lazyStr).kawaii = std::make_shared<std::string>((*((*lazyStr).kawaii)).substr((*lazyStr).beginning, (*lazyStr).sz));
    }
    (*(*lazyStr).kawaii)[lazyStr->beginning + position] = c;
    return *this;
}

lazy_string::lazy::operator char() const {
    return (*(*lazyStr).kawaii)[(*lazyStr).beginning + position];
}

size_t lazy_string::size() const {
    return sz;
}

size_t lazy_string::length() const {
    return sz;
}

lazy_string::lazy_string(const std::string &str) {
    (*this).beginning = 0;
    (*this).sz = str.size();
    (*this).kawaii = make_shared<string>(str);
}

lazy_string::lazy_string( std::shared_ptr<std::string> kawaii, size_t beginning, size_t sz) {
    (*this).kawaii = kawaii;
    (*this).beginning = beginning;
    (*this).sz = sz;

}

lazy_string::operator std::string() {
    return (*kawaii).substr(beginning, sz);
}





