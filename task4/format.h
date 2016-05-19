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
    enum length_t {len_hh, len_h, len_default, len_l, len_ll, len_j, len_z, len_t, len_L, len_error};

    struct format_t {
        bool force_sign = false,
                left_justify = false,
                space_or_sign = false,
                alt_num_format = false,
                left_pad = false,
                uppercase = false,
                floating = false;
        int width = 0,
                precision = -1;
        char type;
        enum length_t length = len_default;
    };

    template<typename To, typename From> typename std::enable_if<std::is_convertible<From, To>::value, To>::type convert(From value){
        return (To) value;
    }

    template<typename To, typename From> typename std::enable_if<!std::is_convertible<From, To>::value, To>::type convert(From value){
        throw std::invalid_argument("Invalid argument type");
    }

    std::string find_spec(const std::string &fmt, unsigned &pos, bool has_arguments);

    std::string format_impl(const std::string &fmt, unsigned pos, unsigned printed);

    std::string char_seq(char c, unsigned n);

    /*
     * Если аргумент - nullptr_t – выводит nullptr
     * Если аргумент указатель, и его значение равно 0 – выводит nulltpr<имя_типа>
     * Если аргумент указатель, и его значение не равно 0 - выводит ptr<имя_типа>(вывод_значения_как_для_%@)
     * Если аргумент массив известной размерности – выводит элементы массива через запятую в []
     * Если аргумент может быть преобразован к std::string – выводит результат такого преобразования
     * Если ни одно преобразование невозможно – кидается исключение
     */

    std::string print_at(nullptr_t value);

    template<typename T> typename std::enable_if<!std::is_integral<T>::value && !std::is_convertible<T, std::string>::value && !std::is_pointer<T>::value, std::string>::type print_at(const T& value){
        throw std::invalid_argument("Invalid argument type");
    }

    template<typename T> typename std::enable_if<std::is_integral<T>::value, std::string>::type print_at(T value){
        return std::to_string(value);
    }

    template<typename T, int num> typename std::enable_if<!std::is_convertible<T*, std::string>::value, std::string>::type print_at(const T (&a)[num]) {
        std::string r = "[";
        for(int i = 0; i < num - 1; i++){
            r += (std::to_string(a[i]) + ", ");
        }
        r += (std::to_string(a[num - 1]) + ']');
        return r;
    }

    template<typename T> typename std::enable_if<std::is_convertible<T, std::string>::value, std::string>::type print_at(const T& value){
        return value;
    }

    template<typename T> typename std::enable_if<!std::is_array<T>::value && !std::is_convertible<T, std::string>::value && std::is_pointer<T>::value, std::string>::type print_at(T& value){
        std::string r;
        if(value == 0){
            r += "nullptr<";
            r += typeid(*value).name();
            r += ">";
        } else {
            r += "ptr<";
            r += typeid(*value).name();
            r += ">(";
            r+= format("%@", *value);
            r += ")";
        }
        return r;
    }

    template<typename T> typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type print_num(format_t fm, T value){
        // Disclaimer:
        // This template might not comply with *printf standarts but I hope everything is OK
        if(!fm.floating && fm.precision >= 0 && fm.left_pad) {
            fm.left_pad = false;
        }
        if(!fm.floating && fm.precision < 0 ){
            fm.precision = 1;
        }
        
        std::string temp = "%";
        
        if(fm.force_sign){temp += '+';}
        if(fm.left_justify){temp += '-';}
        if(fm.space_or_sign){temp += ' ';}
        if(fm.alt_num_format){temp += '#';}
        if(fm.left_pad){temp += '0';}
        if(fm.precision >= 0){
            temp += '.';
            if (fm.precision > 1024){
                temp += std::to_string(1024);
            } else {
                temp += std::to_string(fm.precision);
            }
        }
        char buffer[2048];
        
        if (fm.floating && fm.length == len_L)
            {temp += 'L';}
        if (fm.floating && fm.length == len_l)
            {temp += 'l';}
        if (fm.floating)
            {temp += fm.type;}
        if(!fm.floating){
            temp += 'j';
            temp += fm.type;
        }
        
        snprintf(buffer, sizeof(buffer), temp.c_str(), value);
        
        std::string r = buffer;
        
        if(fm.precision > 1024 && r.size() > 512 && fm.floating){
                r += char_seq('0', fm.precision - r.size() + r.find_first_of('.') + 1);
        }
        if(fm.precision > 1024 && r.size() > 512 && !fm.floating){
                if (r[0]=='0'){
                    r = r.substr(0, 2) + char_seq('0', fm.precision - r.size()) + r.substr(2);
                } else {
                    r = r.substr(0, 2) + char_seq('0', fm.precision - r.size() + 1) + r.substr(2);
                }
        }
           

        if((unsigned) fm.width > r.size()){
           if(fm.left_justify){
                r +=char_seq(' ', fm.width - r.size());
            } else {
                if(fm.left_pad){
                    if (r.find_first_of("+- ") == 0){
                        r =  r[0] + char_seq('0', fm.width - r.size()) + r.substr(1);
                    } else {
                        r =  char_seq('0', fm.width - r.size()) + r;
                        //r += r;
                    }
                } else {
                    r = char_seq(' ', fm.width - r.size()) + r;
                   //r += r;
                } 
            }
        }

        return r;
    }

    template<typename First, typename... Rest> std::string format_impl(const std::string& fmt, unsigned pos, unsigned printed, const First& value, const Rest&... args){
        std::string result = find_spec(fmt, pos, true);
        format_t fm;
        std::string temp = "";

        while(pos < fmt.length() && (fmt[pos] == '-' || fmt[pos] == '+' || fmt[pos] == ' ' || fmt[pos] == '#' || fmt[pos] == '0')){
            if (fmt[pos] == '-') {
                    fm.left_justify = true;
                    fm.left_pad = false;
            } else if (fmt[pos] == '+') {
                    fm.force_sign = true;
                    fm.space_or_sign = false;
            } else if (fmt[pos] == ' ') {
                    fm.space_or_sign = !fm.force_sign;
            } else if (fmt[pos] == '#') {
                    fm.alt_num_format = true;
            } else if (fmt[pos] == '0') {
                    fm.left_pad = !fm.left_justify;
            }
            ++pos;
        }

        if(pos < fmt.length() && fmt[pos] == '*'){
            fm.width = convert<int>(value);
            if(fm.width < 0){
                fm.width *= -1;
                fm.left_justify = true;
                fm.left_pad = false;
            }
            temp = "%";
            if(fm.force_sign){temp += '+';}
            if(fm.left_justify){temp += '-';}
            if(fm.space_or_sign){temp += ' ';}
            if(fm.alt_num_format){temp += '#';}
            if(fm.left_pad){temp += '0';}
            temp += std::to_string(fm.width);
            return result + format_impl(temp + fmt.substr(pos + 1, std::string::npos), 0, printed + result.length(), args...);
        } else {
            
            while (pos < fmt.length() && isdigit(fmt[pos])){
                temp += fmt[pos++];
            }
            if(!temp.empty()){
                fm.width = stoi(temp);
                temp.clear();
            }
        }

        if(pos < fmt.length() - 1 && fmt[pos] == '.'){
            ++pos;
            if(fmt[pos] == '*'){
                fm.precision = convert<int>(value);
                temp = "%";
                if(fm.force_sign){temp += '+';}
                if(fm.left_justify){temp += '-';}
                if(fm.space_or_sign){temp += ' ';}
                if(fm.alt_num_format){temp += '#';}
                if(fm.left_pad){temp += '0';}
                if(fm.width != 0){temp += std::to_string(fm.width);}
                temp += '.';
                temp += std::to_string(fm.precision);
                return result + format_impl(temp + fmt.substr(pos + 1, std::string::npos), 0, printed + result.length(), args...);
            } else {
                if(fmt[pos] == '-'){
                    fm.precision = -1;
                    ++pos;
                } else {
                    fm.precision = 1;
                }
                while (pos < fmt.length() && isdigit(fmt[pos])){
                    temp += fmt[pos++];
                }
                if(temp.empty()){
                    fm.precision = 0;
                } 
                if (!temp.empty()) {
                    fm.precision *= stoi(temp);
                    temp.clear();
                }
            }
        }

        while(pos < fmt.length() && (fmt[pos] == 'h' || fmt[pos] == 'l' || fmt[pos] == 'j' || fmt[pos] == 'z' || fmt[pos] == 't' || fmt[pos] == 'L')){
            if (fmt[pos]== 'h') {
                if (fm.length == len_h) {
                    fm.length = len_hh;
                } else{
                    if (fm.length == len_default){
                        fm.length = len_h;
                    } else {
                        fm.length = len_error;
                    }
                }
            } else if (fmt[pos]== 'l') {
                if (fm.length == len_l) {
                    fm.length = len_ll;
                } else{
                    if (fm.length == len_default){
                        fm.length = len_l;
                    } else {
                        fm.length = len_error;
                    }
                }
            } else if (fmt[pos]=='j') {
                    if (fm.length == len_default) {
                        fm.length = len_j;
                    } else {
                        fm.length = len_error;
                    }
            } else if (fmt[pos]== 'z') {
                    if (fm.length == len_default) {
                        fm.length = len_z;
                    } else {
                        fm.length = len_error;
                    }
            }else if (fmt[pos]== 't') {
                    if (fm.length == len_default) {
                        fm.length = len_t;
                    } else {
                        fm.length = len_error;
                    }
            } else if (fmt[pos]== 'L') {
                    if (fm.length == len_default) {
                        fm.length = len_L;
                    } else {
                        fm.length = len_error;
                    }
            }
            ++ pos;
        }

        if(fm.length == len_error){
            throw std::invalid_argument("Unknown length specifier");
        }

        if(pos == fmt.length()){
            throw std::invalid_argument("Сonversion lacks type at end of format");
        }

        std::stringstream out;
        if(fm.force_sign){
            out << std::showpos;
        }
        if(fm.left_justify){
            out << std::left;
        }
        if(fm.width != 0){
            out.width(fm.width);
        }
        if(fm.precision >= 0){
            out.precision(fm.precision);
        }
        if(fm.alt_num_format){
            out << std::showbase << std::showpoint;
        }

        intmax_t d;      // Integer
        uintmax_t u;     // Unsigned
        double f;        // Floating point
        char nil_p[6];   // Null pointer fix

        fm.type = fmt[pos++];
        if (fm.type == 'd' || fm.type =='i') {
                if (fm.length == len_hh) {
                        d = convert<signed char>(value);
                } else if (fm.length == len_h) {
                        d = convert<short int>(value);
                } else if (fm.length == len_l) {
                        d = convert<long int>(value);
                } else if (fm.length == len_ll) {
                        d = convert<long long int>(value);
                } else if (fm.length == len_j) {
                        d = convert<intmax_t>(value);
                } else if (fm.length == len_z) {
                        d = convert<size_t>(value);
                } else if (fm.length == len_t) {
                        d = convert<ptrdiff_t>(value);
                } else if (fm.length == len_default) {
                        d = convert<int>(value);
                } else {
                        throw std::invalid_argument("Unsupported length specifier");
                }
                result += print_num(fm, d);
        } else if (fm.type == 'X') {
                fm.uppercase = true;
        } else if (fm.type == 'x' || fm.type == 'o' || fm.type == 'u') {
                if (fm.length == len_hh) {
                        u = convert<unsigned char>(value);
                } else if (fm.length == len_h) {
                        u = convert<unsigned short int>(value);
                } else if (fm.length == len_l) {
                        u = convert<unsigned long int>(value);
                } else if (fm.length == len_ll) {
                        u = convert<unsigned long long int>(value);
                } else if (fm.length == len_j) {
                        u = convert<uintmax_t>(value);
                } else if (fm.length == len_z) {
                        u = convert<size_t>(value);
                } else if (fm.length == len_t) {
                        u = convert<ptrdiff_t>(value);
                } else if (fm.length == len_default) {
                        u = convert<unsigned int>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                result += print_num(fm, u);
        } else if (fm.type == 'E' || fm.type == 'G' || fm.type == 'A') {
                fm.uppercase = true;
        }else if (fm.type == 'e' || fm.type == 'g' ||fm.type == 'a' || fm.type == 'F' || fm.type == 'f') {
                fm.floating = true;
                if (fm.length == len_l){
                } else if (fm.length == len_default) {
                        f = convert<double>(value);
                } else if (fm.length == len_L) {
                        f = convert<long double>(value);
                } else {
                        throw std::invalid_argument("Unsupported length specifier");
                }
                result += print_num(fm, f);
        } else if (fm.type == 'c') {
                if (fm.length == len_l) {
                } else if (fm.length == len_default) {
                        out << convert<unsigned char>(value);
                } else {
                        throw std::invalid_argument("Unsupported length specifier");
                }
                result += out.str();
        } else if (fm.type == 's') {
                std::string str;
                if (fm.length == len_l) {
                } else if (fm.length == len_default) {
                        str = convert<std::string>(value);
                } else {
                        throw std::invalid_argument("Unsupported length specifier");
                }
                if(fm.precision >= 0 && str.length() > (unsigned) fm.precision){
                    str = str.substr(0, fm.precision);
                }
                out << str;
                result += out.str();
        } else if (fm.type == 'p') {
                if(fm.length != len_default){
                    throw std::invalid_argument("Unsupported length specifier");
                }
                out << std::setfill(fm.left_pad ? '0' : ' ');
                snprintf(nil_p, 2, "%p", convert<void*>(value));
                if(nil_p[0] != '(' && convert<void*>(value) != NULL && convert<void*>(value) != nullptr){
                    out << convert<void*>(value);
                } else {
                    out << "(nil)";
                }
                result += out.str();
        } else if (fm.type == 'n') {
                printed += result.length();
                if (fm.length == len_hh) {
                        *(convert<signed char*>(value)) = printed;
                } else if (fm.length == len_h) {
                        *(convert<short int*>(value)) = printed;
                } else if (fm.length == len_l) {
                        *(convert<long int*>(value)) = printed;
                } else if (fm.length == len_ll) {
                        *(convert<long long int*>(value)) = printed;
                } else if (fm.length == len_j) {
                        *(convert<intmax_t*>(value)) = printed;
                } else if (fm.length == len_z) {
                        *(convert<size_t*>(value)) = printed;
                } else if (fm.length == len_t) {
                        *(convert<ptrdiff_t*>(value)) = printed;
                } else if (fm.length == len_default) {
                        *(convert<int*>(value)) = printed;
                } else {
                        throw std::invalid_argument("Unsupported length specifier");
                }
        } else if (fm.type == '@') {
                result += print_at(value);
        } else {
                throw std::invalid_argument("Unknown format specifier: '" + fmt[pos] + '\'');
        }

        return result + format_impl(fmt, pos, printed + result.length(), args...);
    }
}


/**
 * Returns a std::string formatted with *printf syntax
 *
 * @param   fmt
 *          A <a href="http://cplusplus.com/printf">format string</a>
 *
 * @param   args
 *          Arguments required by the format specifiers in the format
 *          string. If there are more arguments than format specifiers, the
 *          extra arguments are ignored. The number of arguments is
 *          variable and may be zero.
 *
 * @throws  std::invalid_format
 *          If a format string contains an unexpected specifier,
 *          an argument can not be converted to required format,
 *          or in other illegal conditions.
 *
 * @throws  std::out_of_range
 *          If there are not enough arguments in args list
 *
 * @return  std::string, formatted using format and args
 */

template<typename... Args> std::string format(const std::string& fmt, const Args&... args){
    return Format::format_impl(fmt, 0, 0, args...);
}

#endif

