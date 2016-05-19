#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <cstddef>
#include <iomanip>
#include <cstdio>
#include <typeinfo>

template<typename... Args> std::string format(const std::string& fmt, const Args&... args);

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
        bool array[7] = {false};
        int sz = 0, accur = -1;
        char type;

    };

    template<typename To, typename From> typename
    std::enable_if<std::is_convertible<From, To>::value, To>::type convert(From value){

        return (To) value;

    }

    template<typename To, typename From> typename
    std::enable_if<!std::is_convertible<From, To>::value, To>::type convert(From value){

        throw std::invalid_argument("Unacceptable type");

    }

    std::string specification(const std::string &str, unsigned &item, bool presence);

    std::string formatImplementation(const std::string &str, unsigned item, unsigned output);

    std::string sequenceOfChar(char c, unsigned n);

    /*
    * if the argument is  nullptr_t – prints nullptr
    * if the argument is pointer and his value is 0 – prints nulltpr<type_name>
    * if the argument is pointer and his value isn't 0 - prints ptr<type_name>(the_same_as%@)
    * if it is an element of array of known capacity – prints all elements of array in [] separated by commas
    * if argument can't be transformed into std::string – prints result of such modification
    * if non of modifications is possible – throws exception
    */

    std::string print_at(nullptr_t value);

    template<typename T> typename
    std::enable_if<!std::is_integral<T>::value &&
                   !std::is_convertible<T, std::string>::value &&
                   !std::is_pointer<T>::value, std::string>::type print_at(const T& value){

        throw std::invalid_argument("Invalid argument type");

    }

    template<typename T> typename
    std::enable_if<std::is_integral<T>::value,
            std::string>::type print_at(T value){

        return std::to_string(value);

    }

    template<typename T, int m> typename
    std::enable_if<!std::is_convertible<T*,
            std::string>::value,
            std::string>::type print_at(const T (&mas)[m]) {

        std::string finally = "[";

        for(int i = 0; i < m - 1; i++){
            finally += (std::to_string(mas[i]) + ", ");
        }

        finally += (std::to_string(mas[m - 1]) + ']');
        return finally;

    }

    template<typename T> typename
    std::enable_if<std::is_convertible<T,
            std::string>::value,
            std::string>::type print_at(const T& value){

        return value;

    }

    template<typename T> typename
    std::enable_if<!std::is_array<T>::value &&
                   !std::is_convertible<T, std::string>::value &&
                   std::is_pointer<T>::value, std::string>::type print_at(T& value){

        std::string finally;

        if(value == 0){
            finally.append("nullptr<").append(typeid(*value).name()).append(">");
        }

        if(value != 0){
            finally.append("ptr<").append(typeid(*value).name()).append(">(").append(format("%@", *value)).append(")");
        }

        return finally;

    }

    /*
     * Builds string with the given format specifier and the given argument
     * If the argument does not match its specifier, it throws an exception
     */

    template<typename T> typename
    std::enable_if<std::is_arithmetic<T>::value,
            std::string>::type print_num(format_t formStr, T value){

        if(!formStr.array[6] && formStr.accur < 0) {
            formStr.accur = 1;
        }

        if(!formStr.array[6] && formStr.accur >= 0 && formStr.array[4]) {
            formStr.array[4] = false;
        }

        std::string temp = "%";

        if(formStr.array[0]){
            temp += '+';
        }

        if(formStr.array[1]){
            temp += '-';
        }

        if(formStr.array[2]){
            temp += ' ';
        }

        if(formStr.array[3]){
            temp += '#';
        }

        if(formStr.array[4]){
            temp += '0';
        }

        if(formStr.accur >= 0){
            temp += '.';
            if (formStr.accur > 1024)
                temp += (std::to_string(1024));
            else
                temp += (std::to_string(formStr.accur));
        }

        char buf[2048];

        if(formStr.array[6] && formStr.length == L){
            temp += 'L';
        }

        if(formStr.array[6] && formStr.length == l){
            temp += 'l';
        }

        if (formStr.array[6]){
            temp += formStr.type;
        } else {
            temp += 'j';
            temp += formStr.type;
        }

        snprintf(buf, sizeof(buf), temp.c_str(), value);
        std::string templ = buf;

        if(formStr.accur > 1024 && templ.size() > 1024 / 2 && formStr.array[6]){
            templ = templ + sequenceOfChar('0', formStr.accur - templ.size() + templ.find_first_of('.') + 1);
        }

        if(formStr.accur > 1024 && templ.size() > 1024 / 2 && !formStr.array[6]) {

            if (templ[0] == '0') {
                templ = templ.substr(0, 2) + sequenceOfChar('0', formStr.accur - templ.size()) + templ.substr(2);
            } else {
                templ = templ.substr(0, 2) + sequenceOfChar('0', formStr.accur - templ.size() + 1) + templ.substr(2);
            }

        }


        if((unsigned) formStr.sz > templ.size()){

            if(formStr.array[1]){
                templ = templ + sequenceOfChar(' ', formStr.sz - templ.size());
            }

            if(!formStr.array[1] && formStr.array[4]){

                if (templ.find_first_of("+- ") == 0) {
                    templ = templ[0] + sequenceOfChar('0', formStr.sz - templ.size()) + templ.substr(1);
                } else {
                    templ += sequenceOfChar('0', formStr.sz - templ.size()) + templ.substr(1);
                }

            }

            if(!formStr.array[1] && !formStr.array[4]) {
                templ = sequenceOfChar(' ', formStr.sz - templ.size()) + templ;
            }

        }

        return templ;

    }

    template<typename First, typename... Rest>
    std::string formatImplementation(const std::string& strF,
                                     unsigned item,
                                     unsigned output,
                                     const First& value,
                                     const Rest&... args){

        std::string final = specification(strF, item, true);
        std::string temp = "";
        format_t formString;

        while(item < strF.length() &&
              (strF[item] == '-' ||
               strF[item] == '+' ||
               strF[item] == ' ' ||
               strF[item] == '#' ||
               strF[item] == '0')) {

            if (strF[item] == '-') {
                formString.array[1] = true;
                formString.array[4] = false;
            } else if (strF[item] == '+') {
                formString.array[0] = true;
                formString.array[2] = false;
            } else if (strF[item] == ' ') {
                formString.array[2] = !formString.array[0];
            } else if (strF[item] == '#') {
                formString.array[3] = true;
            }else if (strF[item] == '0') {
                formString.array[4] = !formString.array[1];
            }

            ++item;
        }

        if(item < strF.length() && strF[item] == '*'){
            formString.sz = convert<int>(value);
            if(formString.sz < 0){
                formString.sz *= -1;
                formString.array[1] = true;
                formString.array[4] = false;
            }

            temp = "%";

            if(formString.array[0]){
                temp += '+';
            }
            if(formString.array[1]){
                temp += '-';
            }
            if(formString.array[2]){
                temp += ' ';
            }
            if(formString.array[3]){
                temp += '#';
            }
            if(formString.array[4]){
                temp += '0';
            }

            temp += (std::to_string(formString.sz));
            return final + formatImplementation(temp + strF.substr(item + 1, std::string::npos), 0, output + final.length(), args...);
        } else {
            for (; item < strF.length() && isdigit(strF[item]);temp+=strF[item++]);
            //while (item < str.length() && isdigit(str[item])){
            //    temp += str[item++];
            //}

            if(!temp.empty()){
                formString.sz = stoi(temp);
                temp.clear();
            }

        }

        if(item < strF.length() - 1 && strF[item] == '.'){
            item++;

            if(strF[item] == '*'){
                formString.accur = convert<int>(value);
                temp = "%";

                if(formString.array[0]){
                    temp += '+';
                }
                if(formString.array[1]){
                    temp += '-';
                }
                if(formString.array[2]){
                    temp += ' ';
                }
                if(formString.array[3]){
                    temp += '#';
                }
                if(formString.array[4]){
                    temp += '0';
                }
                if(formString.sz != 0){
                    temp += (std::to_string(formString.sz));
                }
                temp += '.';
                temp += (std::to_string(formString.accur));
                return final + formatImplementation(temp + strF.substr(item + 1, std::string::npos), 0, output + final.length(), args...);
            } else {
                if(strF[item] == '-'){
                    formString.accur = -1;
                    item++;
                } else {
                    formString.accur = 1;
                }
                for (;item < strF.length() && isdigit (strF[item]); temp += strF[item++])
                    //while (item < str.length() && isdigit(str[item])){
                    //    temp += str[item++];
                    //}

                    if(!temp.empty()){
                        formString.accur *= stoi(temp);
                        temp.clear();
                    } else {
                        formString.accur = 0;
                    }
            }
        }

        while(item < strF.length() && (strF[item] == 'h' ||
                                      strF[item] == 'l' ||
                                      strF[item] == 'j' ||
                                      strF[item] == 'z' ||
                                      strF[item] == 't' ||
                                      strF[item] == 'L')){

            if (strF[item++] == 'h') {
                if (formString.length == h) {
                    formString.length = hh;
                } else {
                    if (formString.length == standart) {
                        formString.length = h;
                    } else {
                        formString.length = error;
                    }
                }
            } else if (strF[item++] == 'l') {
                if (formString.length == l) {
                    formString.length = ll;
                } else {
                    if (formString.length == standart) {
                        formString.length = l;
                    } else {
                        formString.length = error;
                    }
                }
            } else if (strF[item++] == 'j') {
                if (formString.length == standart) {
                    formString.length = j;
                } else {
                    formString.length = error;
                }
            } else if (strF[item++] == 'z') {
                if (formString.length == standart) {
                    formString.length =z;
                } else {
                    formString.length = error;
                }
            } else if (strF[item++] == 't') {
                if (formString.length == standart) {
                    formString.length = t;
                }else {
                    formString.length = error;
                }
            } else if (strF[item++] == 'L') {
                if (formString.length == standart) {
                    formString.length =  L;
                } else {
                    formString.length = error;
                }
            }
        }

        if(formString.length == error){
            throw std::invalid_argument("Problem with length");
        }

        if(item == strF.length()){
            throw std::invalid_argument("Problem with converting");
        }

        std::stringstream out;
        if(formString.array[0]){
            out << std::showpos;
        }
        if(formString.array[1]){
            out << std::left;
        }
        if(formString.sz != 0){
            out.width(formString.sz);
        }
        if(formString.accur >= 0){
            out.precision(formString.accur);
        }
        if(formString.array[3]){
            out << std::showbase << std::showpoint;
        }

        intmax_t d;
        uintmax_t u;
        double f;
        char nil_p[6];

        formString.type = strF[item++];
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
            final += print_num(formString, d);
        } else if (formString.type == 'X') {
            formString.array[5] = true;
        } else if (formString.type == 'x' || formString.type == 'o' || formString.type ==  'u') {
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
            final += print_num(formString, u);
        } else if (formString.type == 'E' ||
                   formString.type == 'G' ||
                   formString.type == 'A') {
            formString.array[5] = true;
        } else if (formString.type == 'e' ||
                   formString.type == 'g' ||
                   formString.type == 'a' ||
                   formString.type == 'F' ||
                   formString.type == 'f') {
            formString.array[6] = true;
            if (formString.length == l || formString.length == standart) {
                f = convert<double>(value);
            } else if (formString.length == L) {
                f = convert<long double>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }
            final += print_num(formString, f);
        } else if (formString.type == 'c') {
            if (formString.length == l) {
            } else if (formString.length == standart) {
                out << convert<unsigned char>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }
            final += out.str();
        } else if (formString.type == 's') {
            std::string str;
            if (formString.length == l) {
            } else if (formString.length == standart) {
                str = convert<std::string>(value);
            } else {
                throw std::invalid_argument("Problem with length");
            }
            if (formString.accur >= 0 && str.length() > (unsigned) formString.accur) {
                str = str.substr(0, formString.accur);
            }
            out << str;
            final += out.str();
        } else if ( formString.type == 'p') {
            if (formString.length != standart) {
                throw std::invalid_argument("Problem with length");
            }
            out << std::setfill(formString.array[4] ? '0' : ' ');
            snprintf(nil_p, 2, "%p", convert<void *>(value));
            if (nil_p[0] != '(' && convert<void *>(value) != NULL && convert<void *>(value) != nullptr) {
                out << convert<void *>(value);
            } else {
                out << "(nil)";
            }
            final += out.str();
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
            final += print_at(value);
        } else {
            throw std::invalid_argument("Strange format specifier: '" + strF[item] + '\'');
        }

        return final + formatImplementation(strF, item, output + final.length(), args...);
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
    return Format::formatImplementation(str, 0, 0, args...);
}

#endif
