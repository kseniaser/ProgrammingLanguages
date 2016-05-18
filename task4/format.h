#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstddef>
#include <iomanip>
#include <cstdio>
#include <typeinfo>
#include <stddef.h>

template<typename... Args> std::string format(const std::string& str, const Args&... args);

namespace Format {
    enum length_t {hh,
                   h,
                   standart,
                   l,
                   ll,
                   j,
                   z,
                   t,
                   L,
                   error};

    struct format_t {
        enum length_t length = standart;
        bool name[7] = {false};
        long sz= 0;
        long precision = -1;
        char type;
    };

    template<typename To, typename From>
    double convert(From value){
        return (To) value;
    }

    template<typename To, typename From> typename std::enable_if<!std::is_convertible<From, To>::value, To>::type convert(From value){
        throw std::invalid_argument("Unacceptable type");
    }

    std::string specification(const std::string &str, bool presence, unsigned &item);

    std::string Implementation (const std::string &fmt, unsigned output, unsigned item);

    std::string sequenceOfChar(unsigned u, char c );


    /*
     * if the argument is  nullptr_t – prints nullptr
     * if the argument is pointer and his value is 0 – prints nulltpr<type_name>
     * if the argument is pointer and his value isn't 0 - prints ptr<type_name>(the_same_as%@)
     * if it is an element of array of known capacity – prints all elements of array in [] separated by commas
     * if argument can't be transformed into std::string – prints result of such modification
     * if non of modifications is possible – throws exception
     */

    std::string print_at(nullptr_t value);

    template<typename T> typename std::enable_if<!std::is_integral<T>::value &&
            !std::is_convertible<T, std::string>::value &&
            !std::is_pointer<T>::value, std::string>::type print_at(const T& value){
        throw std::invalid_argument("Unacceptable type");
    }

    template<typename T> typename std::enable_if<std::is_integral<T>::value, std::string>::type ptint_at(T value){
        return std::to_string(value);
    }

    template<typename T, int m> typename std::enable_if<!std::is_convertible<T*, std::string>::value, std::string>::type print_at(const T (&a)[m]) {
        std::string temp = "[";
        for(int i = 0; i < m-1; i++){
            temp += (std::to_string(a[i]) + ", ");
        }
        temp += (std::to_string(a[m-1]) + ']');
        return temp;
    }

    template<typename T> typename std::enable_if<std::is_convertible<T, std::string>::value, std::string>::type ptint_at(const T& value){
        return value;
    }

    template<typename T> typename std::enable_if<!std::is_array<T>::value &&
            !std::is_convertible<T, std::string>::value &&
            std::is_pointer<T>::value, std::string>::type print_at(T& value){

        std::string temp;

        if(value != 0){
            temp += "ptr<";
            temp += (typeid(*value).name());
            temp += ">(";
            temp += (format("%@", *value));
            temp += ")";
        } else {
            temp += "nullptr<";
            temp += (typeid(*value).name());
            temp += ">";
        }
        return temp;
    }

    /*
     * Builds string with the given format specifier and the given argument
     * If the argument does not match its specifier, it throws an exception
     */

    template<typename T> typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type print_num(format_t formStr, T value){

        if (!formStr.name[5] && formStr.precision < 0)
            formStr.precision = 1;
        if (!formStr.name[5] && formStr.precision >= 0 && formStr.name[4])
            formStr.name[4]= false;

        std::string temp = "%";

        if(formStr.name[0])
            temp += '+';
        if(formStr.name[1])
            temp += '-';
        if(formStr.name[2])
            temp += ' ';
        if(formStr.name[3])
            temp += '#';
        if(formStr.name[4])
            temp += '0';
        if(formStr.precision >= 0){
            temp += '.';
            long i = (formStr.precision > 1024 ? 1024 : formStr.precision);
            temp += (std::to_string(i));
        }

        char buf[2048];

        if(formStr.name[5]){
            if(formStr.length == L)
                temp += 'L';
            if(formStr.length == l)
                temp += 'l';
            temp += formStr.type;
        } else {
            temp += 'j';
            temp += formStr.type;
        }

        snprintf(buf, sizeof(buf), temp.c_str(), value);
        std::string templ = buf;

        if(formStr.precision > 1024 && templ.size() > 1024 / 2 && formStr.name[5])
            templ += sequenceOfChar(  formStr.precision - templ.size() + templ.find_first_of('.') + 1, '0');

        if(formStr.precision > 1024 && templ.size() > 1024 / 2 && !formStr.name[5]) {
                long k = (templ[0] == '0' ? 0 : 1);
                templ = templ.substr(0, 2) + sequenceOfChar(formStr.precision - templ.size() + (k), '0') + templ.substr(2);
        }


        if((unsigned) formStr.sz > templ.size() && formStr.name[1])
            templ += sequenceOfChar(formStr.sz - templ.size(),' ');

        if((unsigned) formStr.sz > templ.size() && !formStr.name[1] && formStr.name[4]) {
            std::string s1 = templ[0] + sequenceOfChar(formStr.sz - templ.size(), '0') + templ.substr(1);
            std::string s2 = sequenceOfChar(formStr.sz - templ.size(), '0');
            templ += (templ.find_first_of("+- ") == 0) ?  s1 : s2;
        }
        if((unsigned) formStr.sz > templ.size() && !formStr.name[1] && !formStr.name[4]) {
                    templ += sequenceOfChar(formStr.sz - templ.size(), ' ');
        }

        return templ;
    }

    template<typename First, typename... Rest> std::string Implementation(const std::string& str, unsigned item, unsigned output, const First& value, const Rest&... args) {
        std::string final = specification(str, true, item);
        format_t formString;
        std::string temp = "";

        intmax_t d;      // Integer
        uintmax_t u;     // Unsigned
        double f;        // Floating point
        char nil_p[6];   // Null pointer

        while (item < str.length() &&
               (str[item] == '-' ||
                str[item] == '+' ||
                str[item] == ' ' ||
                str[item] == '#' ||
                str[item] == '0'
               )) {
            if (str[item++] == '-') {
                formString.name[1] = true;
                formString.name[4] = false;
            } else if (str[item++] == '+') {
                formString.name[0] = true;
                formString.name[2] = false;
            } else if (str[item++] == ' ') {
                formString.name[2] = !formString.name[0];
            } else if (str[item++] == '#') {
                formString.name[3] = true;
            } else if (str[item++] == '0') {
                formString.name[4] = !formString.name[1];
            }
        }

        if (item < str.length() && str[item] == '*') {
            formString.sz = convert<int>(value);

            if (formString.sz < 0) {
                formString.sz *= -1;
                formString.name[1] = true;
                formString.name[4] = false;
            }

            temp = "%";

            if (formString.name[0])
                temp += '+';
            if (formString.name[1])
                temp += '-';
            if (formString.name[2])
                temp += ' ';
            if (formString.name[3])
                temp += '#';
            if (formString.name[4])
                temp += '0';

            temp += (std::to_string(formString.sz));

            return final + Implementation(temp + str.substr(item + 1, std::string::npos), 0, output + final.length(), args...);

        } else {

            while (item < str.length() && isdigit(str[item]))
                temp += str[item++];

            if (!temp.empty()) {
                formString.sz = stoi(temp);
                temp.clear();
            }
        }

        if (item < str.length() - 1 && str[item] == '.') {
            ++item;
            if (str[item] == '*') {
                formString.precision = convert<int>(value);
                temp = "%";
                if (formString.name[0])
                    temp += '+';
                if (formString.name[1])
                    temp += '-';
                if (formString.name[2])
                    temp += ' ';
                if (formString.name[3])
                    temp += '#';
                if (formString.name[4])
                    temp += '0';
                if (formString.sz != 0)
                    temp.append(std::to_string(formString.sz));
                temp += '.';
                temp.append(std::to_string(formString.precision));
                return final + Implementation(temp + str.substr(item + 1, std::string::npos), 0, output + final.length(), args...);
            } else {
                if (str[item] == '-') {
                    formString.precision = -1;
                    item++;
                }
                if (str[item] != '-') {
                    formString.precision = 1;
                }
                while (item < str.length() && isdigit(str[item]))
                    temp += str[item++];
                if (!temp.empty()) {
                    formString.precision *= stoi(temp);
                    temp.clear();
                }
                if (temp.empty()) {
                    formString.precision = 0;
                }
            }
        }

        while (item < str.length() &&
               (str[item] == 'h' ||
                str[item] == 'l' ||
                str[item] == 'j' ||
                str[item] == 'z' ||
                str[item] == 't' ||
                str[item] == 'L'
               )) {
            bool nya = (formString.length == standart);
            if (str[item++] == 'h') {
                long hex = (nya) ? h : error;
                formString.length = (formString.length == h) ? hh : hex;
            } else if (str[item++] == 'l') {
                long lex = (nya) ? l : error;
                formString.length = (formString.length == l) ? ll : lex;
            } else if (str[item++] == 'j') {
                formString.length = (nya) ? j : error;
            } else if (str[item++] == 'z') {
                formString.length = (nya) ? z : error;
            } else if (str[item++] == 't') {
                formString.length = (nya) ? t : error;
            } else if (str[item++] == 'L') {
                formString.length = (nya) ? L : error;
            }
        }


        if (formString.length == error) {
            throw std::invalid_argument("Problem with length");
        }

        if (item == str.length()) {
            throw std::invalid_argument("Problem with converting");
        }

        std::stringstream out;

        if (formString.name[0])
            out << std::showpos;

        if (formString.name[1])
            out << std::left;

        if (formString.sz != 0)
            out.width(formString.sz);

        if (formString.precision >= 0)
            out.precision(formString.precision);

        if (formString.name[3])
            out << std::showbase << std::showpoint;


        formString.type = str[item++];

        if (formString.type == 'd' || formString.type == 'i') {

            if (formString.length == hh) {
                d = convert<signed char>(value);
            } else if (formString.length == h) {
                d = convert<short int>(value);
            } else if (formString.length == l) {
                d = convert<long int>(value);
            } else if (formString.length == ll) {
                d = convert<long long int>(value);
            } else if (formString.length == j) {
                d = convert<intmax_t>(value);
            } else if (formString.length == z) {
                d = convert<size_t>(value);
            } else if (formString.length == t) {
                d = convert<ptrdiff_t>(value);
            } else if (formString.length == standart) {
                d = convert<int>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }

            final += (print_num(formString, d));

        } else if (formString.type == 'X') {

            formString.name[5] = true;

        } else if (formString.type == 'x' || formString.type == 'o' || formString.type == 'u') {

            if (formString.length == hh) {
                u = convert<unsigned char>(value);
            } else if (formString.length == h) {
                u = convert<unsigned short int>(value);
            } else if (formString.length == l) {
                u = convert<unsigned long int>(value);
            } else if (formString.length == ll) {
                u = convert<unsigned long long int>(value);
            } else if (formString.length == j) {
                u = convert<uintmax_t>(value);
            } else if (formString.length == z) {
                u = convert<size_t>(value);
            } else if (formString.length == t) {
                u = convert<ptrdiff_t>(value);
            } else if (formString.length == standart) {
                u = convert<unsigned int>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }

            final += (print_num(formString, u));

        } else if (formString.type == 'E' || formString.type == 'G' || formString.type == 'A') {

            formString.name[5] = true;

        } else if (formString.type == 'e' ||
                   formString.type == 'g' ||
                   formString.type == 'a' ||
                   formString.type == 'F' ||
                   formString.type == 'f'
                   ){

            formString.name[5] = true;

            if (formString.length == l || formString.length == standart) {
                f = convert<double>(value);
            } else if (formString.length == L) {
                f = convert<long double>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }

            final += (print_num(formString, f));

        } else if (formString.type == 'c') {

            if (formString.length == l) {
            } else if (formString.length == standart) {
                out << convert<unsigned char>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }

            final += (out.str());

        } else if (formString.type == 's') {

            std::string str;

            if (formString.length == l) {
            } else if (formString.length == standart) {
                str = convert<std::string>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }

            if (formString.precision >= 0 && str.length() > (unsigned) formString.precision) {
                str = str.substr(0, formString.precision);
            }

            out << str;
            final += (out.str());

        } else if (formString.type == 'p') {

            if (formString.length != standart) {
                throw std::invalid_argument("Problem with length");
            }

            out << std::setfill(formString.name[4] ? '0' : ' ');
            snprintf(nil_p, 2, "%p", convert<void *>(value));

            if (nil_p[0] != '(' && convert<void *>(value) != NULL && convert<void *>(value) != nullptr) {
                out << convert<void *>(value);
            } else {
                out << "(nil)";
            }

            final += (out.str());

        } else if (formString.type == 'n') {

            output += final.length();

            if (formString.length == hh) {
                *(convert<signed char *>(value)) = output;
            } else if (formString.length == h) {
                *(convert<short int *>(value)) = output;
            } else if (formString.length == l) {
                *(convert<long int *>(value)) = output;
            } else if (formString.length == ll) {
                *(convert<long long int *>(value)) = output;
            } else if (formString.length == j) {
                *(convert<intmax_t *>(value)) = output;
            } else if (formString.length == z) {
                *(convert<size_t *>(value)) = output;
            } else if (formString.length == t) {
                *(convert<ptrdiff_t *>(value)) = output;
            } else if (formString.length == standart) {
                *(convert<int *>(value)) = output;
            } else {
                throw std::invalid_argument("Problem with length");
            }

        } else if (formString.type == '@') {

            final += (print_at(value));

        } else{

            throw std::invalid_argument("Strange format specifier: '" + str[item] + '\'');

        }

        return final + Implementation(str, item, output + final.length(), args...);
    }
}


/**
 * Transform std::string into *printf syntax
 *
 * @param   str
 *          A format string
 *
 * @param   args
 *          Essential argument for the format string.
 *          In case of abundance of arguments ignores unwanted arguments.
 *          Variable number of arguments.
 *
 * @throws  std::invalid_format
 *          If an argument was converted to unrequired format
 *          or in other illegal conditions.
 *
 * @throws  std::out_of_range
 *          If there lack of arguments in args list
 *
 * @return  std::string, transformed using format and args
 */

template<typename... Args> std::string format(const std::string& str, const Args&... args){
    return Format::Implementation(str, 0, 0, args...);
}
#endif
