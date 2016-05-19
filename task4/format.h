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
        bool array[8] = {false};
        int sz = 0, accur = -1;
        char type;

    };

    template<typename To, typename From> typename
            std::enable_if<std::is_convertible<From, To>::value, To>::type convert(From value){
        return (To) value;
    }

    template<typename To, typename From> typename
            std::enable_if<!std::is_convertible<From, To>::value, To>::type convert(From value){
        throw std::invalid_argument("Invalid argument type");
    }

    std::string spec(const std::string &fmt, unsigned &item, bool notEmpty);

    std::string impl(const std::string &fmt, unsigned item, unsigned output);

    std::string seq(char c, unsigned n);

    /*
    * if the argument is  nullptr_t – prints nullptr
    * if the argument is pointer and his value is 0 – prints nulltpr<type_name>
    * if the argument is pointer and his value isn't 0 - prints ptr<type_name>(the_same_as%@)
    * if it is an element of array of known capacity – prints all elements of array in [] separated by commas
    * if argument can't be transformed into std::string – prints result of such modification
    * if non of modifications is possible – throws exception
    */

    std::string printVal (nullptr_t value);

    template<typename T> typename
                    std::enable_if<!std::is_integral<T>::value && !std::is_convertible<T,
                    std::string>::value && !std::is_pointer<T>::value, std::string>::type printVal(const T& value){
        throw std::invalid_argument("Invalid argument type");
    }

    template<typename T> typename std::enable_if<std::is_integral<T>::value, std::string>::type printVal(T value){
        return std::to_string(value);
    }

    template<typename T, int m> typename
                    std::enable_if<!std::is_convertible<T*,
                    std::string>::value,
                    std::string>::type printVal(const T (&a)[m]) {

        std::string final = "[";
        for(int i = 0; i < m - 1; i++){
            final += (std::to_string(a[i]) + ", ");
        }
        final += (std::to_string(a[m - 1]) + ']');
        return final;
    }

    template<typename T> typename
                  std::enable_if<std::is_convertible<T,
                  std::string>::value,
                  std::string>::type ptintVal(const T& value){

        return value;

    }

    template<typename T> typename
            std::enable_if<!std::is_array<T>::value && !std::is_convertible<T,
            std::string>::value && std::is_pointer<T>::value,
            std::string>::type printVal(T& value){

        std::string final;
        if(value == 0){
            final += "nullptr<";
            final += typeid(*value).name();
            final += ">";
        } else {
            final += "ptr<";
            final += typeid(*value).name();
            final += ">(";
            final+= format("%@", *value);
            final += ")";
        }
        return final;
    }

    /*
     * Builds string with the given format specifier and the given argument
     * If the argument does not match its specifier, it throws an exception
     */

    template<typename T> typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type printNum(format_t strF, T value){

        if(!strF.array[6] && strF.accur >= 0 && strF.array[4]) {
            strF.array[4] = false;
        }
        if(!strF.array[6] && strF.accur < 0 ){
            strF.accur = 1;
        }

        std::string temp = "%";

        if(strF.array[0]){
            temp += '+';
        }
        if(strF.array[1]){
            temp += '-';
        }
        if(strF.array[2]){
            temp += ' ';}
        if(strF.array[3]){temp += '#';
        }
        if(strF.array[4]){
            temp += '0';
        }
        if(strF.accur >= 0){
            temp += '.';
            if (strF.accur > 1024){
                temp += std::to_string(1024);
            } else {
                temp += std::to_string(strF.accur);
            }
        }
        char buffer[2048];

        if (strF.array[6] && strF.length == L) {
            temp += 'L';
        }
        if (strF.array[6] && strF.length == l) {
            temp += 'l';
        }
        if (strF.array[6]) {
            temp += strF.type;
        }
        if(!strF.array[6]){
            temp += 'j';
            temp += strF.type;
        }

        snprintf(buffer, sizeof(buffer), temp.c_str(), value);

        std::string r = buffer;

        if(strF.accur > 1024 && r.size() > 512 && strF.array[6]){
            r += seq('0', strF.accur - r.size() + r.find_first_of('.') + 1);
        }
        if(strF.accur > 1024 && r.size() > 512 && !strF.array[6]){
            if (r[0]=='0'){
                r = r.substr(0, 2) + seq('0', strF.accur - r.size()) + r.substr(2);
            } else {
                r = r.substr(0, 2) + seq('0', strF.accur - r.size() + 1) + r.substr(2);
            }
        }


        if((unsigned) strF.sz > r.size()){
            if(strF.array[1]){
                r +=seq(' ', strF.sz - r.size());
            } else {
                if(strF.array[4]){
                    if (r.find_first_of("+- ") == 0){
                        r =  r[0] + seq('0', strF.sz - r.size()) + r.substr(1);
                    } else {
                        r =  seq('0', strF.sz - r.size()) + r;
                        //r += r;
                    }
                } else {
                    r = seq(' ', strF.sz - r.size()) + r;
                    //r += r;
                }
            }
        }

        return r;
    }

    template<typename First, typename... Rest>
    std::string impl(const std::string& stringF,
                            unsigned item,
                            unsigned output,
                            const First& value,
                            const Rest&... args){

        std::string final = spec(stringF, item, true);
        format_t formtH;
        std::string param = "";
        intmax_t maxInt;
        uintmax_t maxUi;
        double frfr;
        char masNil[6];

        while(item < stringF.length() &&
                (stringF[item] == '-' ||
                        stringF[item] == '+' ||
                        stringF[item] == ' ' ||
                        stringF[item] == '#' ||
                        stringF[item] == '0')){

            if (stringF[item] == '-') {
                formtH.array[1] = true;
                formtH.array[4] = false;
            } else if (stringF[item] == '+') {
                formtH.array[0] = true;
                formtH.array[2] = false;
            } else if (stringF[item] == ' ') {
                formtH.array[2] = !formtH.array[0];
            } else if (stringF[item] == '#') {
                formtH.array[3] = true;
            } else if (stringF[item] == '0') {
                formtH.array[4] = !formtH.array[1];
            }
            ++item;
        }

        if(item < stringF.length() && stringF[item] == '*'){
            formtH.sz = convert<int>(value);
            if(formtH.sz < 0){
                formtH.sz *= -1;
                formtH.array[1] = true;
                formtH.array[4] = false;
            }
            param = "%";
            if(formtH.array[0]){param += '+';}
            if(formtH.array[1]){param += '-';}
            if(formtH.array[2]){param += ' ';}
            if(formtH.array[3]){param += '#';}
            if(formtH.array[4]){param += '0';}
            param += std::to_string(formtH.sz);
            return final + impl(param + stringF.substr(item + 1, std::string::npos), 0, output + final.length(), args...);
        } else {

            while (item < stringF.length() && isdigit(stringF[item])){
                param += stringF[item++];
            }
            if(!param.empty()){
                formtH.sz = stoi(param);
                param.clear();
            }
        }

        if(item < stringF.length() - 1 && stringF[item] == '.'){
            ++item;
            if(stringF[item] == '*'){
                formtH.accur = convert<int>(value);
                param = "%";
                if(formtH.array[0]){param += '+';}
                if(formtH.array[1]){param += '-';}
                if(formtH.array[2]){param += ' ';}
                if(formtH.array[3]){param += '#';}
                if(formtH.array[4]){param += '0';}
                if(formtH.sz != 0){param += std::to_string(formtH.sz);}
                param += '.';
                param += std::to_string(formtH.accur);
                return final + impl(param + stringF.substr(item + 1, std::string::npos), 0, output + final.length(), args...);
            } else {
                if(stringF[item] == '-'){
                    formtH.accur = -1;
                    ++item;
                } else {
                    formtH.accur = 1;
                }
                while (item < stringF.length() && isdigit(stringF[item])){
                    param += stringF[item++];
                }
                if(param.empty()){
                    formtH.accur = 0;
                }
                if (!param.empty()) {
                    formtH.accur *= stoi(param);
                    param.clear();
                }
            }
        }

        while(item < stringF.length()
              && (stringF[item] == 'h' ||
                stringF[item] == 'l' ||
                stringF[item] == 'j' ||
                stringF[item] == 'z' ||
                stringF[item] == 't' ||
                stringF[item] == 'L')){

            if (stringF[item]== 'h') {
                if (formtH.length == h) {
                    formtH.length = hh;
                } else{
                    if (formtH.length ==  standart){
                        formtH.length = h;
                    } else {
                        formtH.length = error;
                    }
                }
            } else if (stringF[item]== 'l') {
                if (formtH.length == l) {
                    formtH.length = ll;
                } else{
                    if (formtH.length == standart){
                        formtH.length = l;
                    } else {
                        formtH.length = error;
                    }
                }
            } else if (stringF[item]=='j') {
                if (formtH.length == standart) {
                    formtH.length = j;
                } else {
                    formtH.length = error;
                }
            } else if (stringF[item]== 'z') {
                if (formtH.length == standart) {
                    formtH.length = z;
                } else {
                    formtH.length = error;
                }
            }else if (stringF[item]== 't') {
                if (formtH.length == standart) {
                    formtH.length = t;
                } else {
                    formtH.length = error;
                }
            } else if (stringF[item]== 'L') {
                if (formtH.length == standart) {
                    formtH.length = L;
                } else {
                    formtH.length = error;
                }
            }
            ++ item;
        }

        if(formtH.length == error){
            throw std::invalid_argument("Unknown length specifier");
        }

        if(item == stringF.length()){
            throw std::invalid_argument("Сonversion lacks type at end of format");
        }

        std::stringstream out;

        if(formtH.array[0]){
            out << std::showpos;
        }
        if(formtH.array[1]){
            out << std::left;
        }
        if(formtH.sz != 0){
            out.width(formtH.sz);
        }
        if(formtH.accur >= 0){
            out.precision(formtH.accur);
        }
        if(formtH.array[3]){
            out << std::showbase << std::showpoint;
        }

        formtH.type = stringF[item++];
        switch(formtH.type){
            case 'd':
            case 'i':
                if (formtH.length == hh) {
                    maxInt = convert<signed char>(value);
                } else if (formtH.length == h) {
                    maxInt = convert<short int>(value);
                } else if (formtH.length == l) {
                    maxInt = convert<long int>(value);
                } else if (formtH.length == ll) {
                    maxInt = convert<long long int>(value);
                } else if (formtH.length == j) {
                    maxInt = convert<intmax_t>(value);
                } else if (formtH.length == z) {
                    maxInt = convert<size_t>(value);
                } else if (formtH.length == t) {
                    maxInt = convert<ptrdiff_t>(value);
                } else if (formtH.length == standart) {
                    maxInt = convert<int>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                final += printNum(formtH, maxInt);
                break;
            case 'X':
                formtH.array[5] = true;
            case 'x':
            case 'o':
            case 'u':
                if (formtH.length == hh) {
                    maxUi = convert<unsigned char>(value);
                } else if (formtH.length == h) {
                    maxUi = convert<unsigned short int>(value);
                } else if (formtH.length == l) {
                    maxUi = convert<unsigned long int>(value);
                } else if (formtH.length == ll) {
                    maxUi = convert<unsigned long long int>(value);
                } else if (formtH.length == j) {
                    maxUi = convert<uintmax_t>(value);
                } else if (formtH.length == z) {
                    maxUi = convert<size_t>(value);
                } else if (formtH.length == t) {
                    maxUi = convert<ptrdiff_t>(value);
                } else if (formtH.length == standart) {
                    maxUi = convert<unsigned int>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                final += printNum(formtH, maxUi);
                break;
            case 'E':
            case 'G':
            case 'A':
                formtH.array[5] = true;
            case 'e':
            case 'g':
            case 'a':
            case 'F':
            case 'f':
                formtH.array[6] = true;
                if (formtH.length == l){
                } else if (formtH.length == standart) {
                    frfr = convert<double>(value);
                } else if (formtH.length == L) {
                    frfr = convert<long double>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                final += printNum(formtH, frfr);
                break;
            case 'c':
                if (formtH.length == l) {
                } else if (formtH.length == standart) {
                    out << convert<unsigned char>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                final += out.str();
                break;
            case 's': {
                std::string str;
                if (formtH.length == l) {
                } else if (formtH.length == standart) {
                    str = convert<std::string>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                if(formtH.accur >= 0 && str.length() > (unsigned) formtH.accur){
                    str = str.substr(0, formtH.accur);
                }
                out << str;
                final += out.str();
            }
                break;
            case 'p':
                if(formtH.length != standart){
                    throw std::invalid_argument("Unsupported length specifier");
                }
                out << std::setfill(formtH.array[4] ? '0' : ' ');
                snprintf(masNil, 2, "%p", convert<void*>(value));
                if(masNil[0] != '(' && convert<void*>(value) != NULL && convert<void*>(value) != nullptr){
                    out << convert<void*>(value);
                } else {
                    out << "(nil)";
                }
                final += out.str();
                break;
            case 'n':
                output += final.length();
                if (formtH.length == hh) {
                    *(convert<signed char*>(value)) = output;
                } else if (formtH.length == h) {
                    *(convert<short int*>(value)) = output;
                } else if (formtH.length == l) {
                    *(convert<long int*>(value)) = output;
                } else if (formtH.length == ll) {
                    *(convert<long long int*>(value)) = output;
                } else if (formtH.length == j) {
                    *(convert<intmax_t*>(value)) = output;
                } else if (formtH.length == z) {
                    *(convert<size_t*>(value)) = output;
                } else if (formtH.length == t) {
                    *(convert<ptrdiff_t*>(value)) = output;
                } else if (formtH.length == standart) {
                    *(convert<int*>(value)) = output;
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                break;
            case '@':
                final += printVal(value);
                break;
            default:
                throw std::invalid_argument("Unknown format specifier: '" + stringF[item] + '\'');
                break;
        }

        return final + impl(stringF, item, output + final.length(), args...);
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

template<typename... Args> std::string format(const std::string& fmt, const Args&... args){
    return Format::impl(fmt, 0, 0, args...);
}

#endif
