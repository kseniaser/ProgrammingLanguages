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
			r.append(std::to_string(a[i]) + ", ");
		}
		r.append(std::to_string(a[num - 1]) + ']');
        return r;
    }

    template<typename T> typename std::enable_if<std::is_convertible<T, std::string>::value, std::string>::type print_at(const T& value){
        return value;
	}
    
    template<typename T> typename std::enable_if<!std::is_array<T>::value && !std::is_convertible<T, std::string>::value && std::is_pointer<T>::value, std::string>::type print_at(T& value){
        std::string r;
		if(value == 0){
			r.append("nullptr<").append(typeid(*value).name()).append(">");
		} else {
		    r.append("ptr<").append(typeid(*value).name()).append(">(").append(format("%@", *value)).append(")");
		}
		return r;
	}

    template<typename T> typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type print_num(format_t fm, T value){
        // Disclaimer:
        // This template might not comply with *printf standarts but I hope everything is OK
        if(!fm.floating){
            if(fm.precision < 0){
                fm.precision = 1;
            } else if(fm.left_pad) {
                fm.left_pad = false;
            }
        }

        std::string temp = "%";
        if(fm.force_sign){temp.push_back('+');}
        if(fm.left_justify){temp.push_back('-');}
        if(fm.space_or_sign){temp.push_back(' ');}
        if(fm.alt_num_format){temp.push_back('#');}
        if(fm.left_pad){temp.push_back('0');}
        //if(fm.width != 0){temp.append(std::to_string(fm.width > 2048 ? 2048 : fm.width));}
        if(fm.precision >= 0){
            temp.push_back('.');
            temp.append(std::to_string(fm.precision > 1024 ? 1024 : fm.precision));
        }
        char buffer[2048];
        if(fm.floating){
            if(fm.length == len_L){temp.push_back('L');}
            if(fm.length == len_l){temp.push_back('l');}
            temp.push_back(fm.type);
        } else { 
            temp.push_back('j');
            temp.push_back(fm.type);
        }
        snprintf(buffer, sizeof(buffer), temp.c_str(), value);
        std::string r = buffer;
        if(fm.precision > 1024 && r.size() > 1024 / 2){
            if(fm.floating){
                r = r + char_seq('0', fm.precision - r.size() + r.find_first_of('.') + 1);
            } else {
                r = r.substr(0, 2) + char_seq('0', fm.precision - r.size() + (r[0] == '0' ? 0 : 1)) + r.substr(2);
            }
        }

        if((unsigned) fm.width > r.size()){
            if(fm.left_justify){
                r = r + char_seq(' ', fm.width - r.size());
            } else {
                if(fm.left_pad){
                    r = (r.find_first_of("+- ") == 0) ? r[0] + char_seq('0', fm.width - r.size()) + r.substr(1) : char_seq('0', fm.width - r.size()) + r;
                } else {
                    r = char_seq(' ', fm.width - r.size()) + r;
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
            switch(fmt[pos++]){
                case '-':
                    fm.left_justify = true;
                    fm.left_pad = false;
                    break;
                case '+':
                    fm.force_sign = true;
                    fm.space_or_sign = false;
                    break;
                case ' ':
                    fm.space_or_sign = !fm.force_sign;
                    break;
                case '#':
                    fm.alt_num_format = true;
                    break;
                case '0':
                    fm.left_pad = !fm.left_justify;
                    break;
            }
        }

        if(pos < fmt.length() && fmt[pos] == '*'){
            fm.width = convert<int>(value);
            if(fm.width < 0){
                fm.width *= -1;
                fm.left_justify = true;
                fm.left_pad = false;
            }
            temp = "%";
            if(fm.force_sign){temp.push_back('+');}
            if(fm.left_justify){temp.push_back('-');}
            if(fm.space_or_sign){temp.push_back(' ');}
            if(fm.alt_num_format){temp.push_back('#');}
            if(fm.left_pad){temp.push_back('0');}
            temp.append(std::to_string(fm.width));
            return result + format_impl(temp + fmt.substr(pos + 1, std::string::npos), 0, printed + result.length(), args...);
        } else {
            for(; pos < fmt.length() && isdigit(fmt[pos]); temp.push_back(fmt[pos++]));
            if(!temp.empty()){
                fm.width = stoi(temp);
                temp.clear();
            }
        }

        if(pos < fmt.length() - 1 && fmt[pos] == '.'){
            pos++;
            if(fmt[pos] == '*'){
                fm.precision = convert<int>(value);
                temp = "%";
                if(fm.force_sign){temp.push_back('+');}
                if(fm.left_justify){temp.push_back('-');}
                if(fm.space_or_sign){temp.push_back(' ');}
                if(fm.alt_num_format){temp.push_back('#');}
                if(fm.left_pad){temp.push_back('0');}
                if(fm.width != 0){temp.append(std::to_string(fm.width));}
                temp.push_back('.');
                temp.append(std::to_string(fm.precision));
                return result + format_impl(temp + fmt.substr(pos + 1, std::string::npos), 0, printed + result.length(), args...);
            } else {
                if(fmt[pos] == '-'){
                    fm.precision = -1;
                    pos++;
                } else {
                    fm.precision = 1;
                }
                for(; pos < fmt.length() && isdigit(fmt[pos]); temp.push_back(fmt[pos++]));
                if(!temp.empty()){
                    fm.precision *= stoi(temp);
                    temp.clear();
                } else {
                    fm.precision = 0;
                }
            }
        }

        while(pos < fmt.length() && (fmt[pos] == 'h' || fmt[pos] == 'l' || fmt[pos] == 'j' || fmt[pos] == 'z' || fmt[pos] == 't' || fmt[pos] == 'L')){
            switch(fmt[pos++]){
                case 'h':
                    fm.length = (fm.length == len_h) ? len_hh : ((fm.length == len_default) ? len_h : len_error);
                    break;
                case 'l':
                    fm.length = (fm.length == len_l) ? len_ll : ((fm.length == len_default) ? len_l : len_error);
                    break;
                case 'j':
                    fm.length = (fm.length == len_default) ? len_j : len_error;
                    break;
                case 'z':
                    fm.length = (fm.length == len_default) ? len_z : len_error;
                    break;
                case 't':
                    fm.length = (fm.length == len_default) ? len_t : len_error;
                    break;
                case 'L':
                    fm.length = (fm.length == len_default) ? len_L : len_error;
                    break;
            }
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
        // if(fm.space_or_sign){temp.push_back(' ');}
        
        intmax_t d;      // Integer
        uintmax_t u;     // Unsigned
        double f;        // Floating point
        char nil_p[6];   // Null pointer fix
        
        fm.type = fmt[pos++];
        switch(fm.type){
            case 'd':
            case 'i':
                switch (fm.length){
                    case len_hh:
                        d = convert<signed char>(value);
                        break;
                    case len_h:
                        d = convert<short int>(value);
                        break;
                    case len_l:
                        d = convert<long int>(value);
                        break;
                    case len_ll:
                        d = convert<long long int>(value);
                        break;
                    case len_j:
                        d = convert<intmax_t>(value);
                        break;
                    case len_z:
                        d = convert<size_t>(value);
                        break;
                    case len_t:
                        d = convert<ptrdiff_t>(value);
                        break;
                    case len_default:
                        d = convert<int>(value);
                        break;
                    default:
                        throw std::invalid_argument("Unsupported length specifier");
                }
                result.append(print_num(fm, d));
                break;
            case 'X':
                fm.uppercase = true;
            case 'x':
            case 'o':
            case 'u':
                switch (fm.length){
                    case len_hh:
                        u = convert<unsigned char>(value);
                        break;
                    case len_h:
                        u = convert<unsigned short int>(value);
                        break;
                    case len_l:
                        u = convert<unsigned long int>(value);
                        break;
                    case len_ll:
                        u = convert<unsigned long long int>(value);
                        break;
                    case len_j:
                        u = convert<uintmax_t>(value);
                        break;
                    case len_z:
                        u = convert<size_t>(value);
                        break;
                    case len_t:
                        u = convert<ptrdiff_t>(value);
                        break;
                    case len_default:
                        u = convert<unsigned int>(value);
                        break;
                    default:
                        throw std::invalid_argument("Unsupported length specifier");
                }
                result.append(print_num(fm, u));
                break;
            case 'E':
            case 'G':
            case 'A':
                fm.uppercase = true;
            case 'e':
            case 'g':
            case 'a':
            case 'F':
            case 'f':
                fm.floating = true;
                switch (fm.length){
                    case len_l:
                    case len_default:
                        f = convert<double>(value);
                        break;
                    case len_L:
                        f = convert<long double>(value);
                        break;
                    default:
                        throw std::invalid_argument("Unsupported length specifier");
                }
                result.append(print_num(fm, f));
                break;
            case 'c':
                switch (fm.length){
                    case len_l:
                        // out << convert<wchar_t>(value); // FIXME Wide chars
                        break;
                    case len_default:
                        out << convert<unsigned char>(value);
                        break;
                    default:
                        throw std::invalid_argument("Unsupported length specifier");
                } 
                result.append(out.str());
                break;
            case 's': {
                std::string str;
                switch (fm.length){
                    case len_l:
                        // str = convert<std::wstring>(value); // FIXME Wide strings
                        break;
                    case len_default:
                        str = convert<std::string>(value);
                        break;
                    default:
                        throw std::invalid_argument("Unsupported length specifier");
                }
                if(fm.precision >= 0 && str.length() > (unsigned) fm.precision){
                    str = str.substr(0, fm.precision);
                }
                out << str;
                result.append(out.str());
                }
                break;
            case 'p':
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
                result.append(out.str());
                break;
            case 'n':
                printed += result.length();
                switch (fm.length){
                    case len_hh:
                        *(convert<signed char*>(value)) = printed;
                        break;
                    case len_h:
                        *(convert<short int*>(value)) = printed;
                        break;
                    case len_l:
                        *(convert<long int*>(value)) = printed;
                        break;
                    case len_ll:
                        *(convert<long long int*>(value)) = printed;
                        break;
                    case len_j:
                        *(convert<intmax_t*>(value)) = printed;
                        break;
                    case len_z:
                        *(convert<size_t*>(value)) = printed;
                        break;
                    case len_t:
                        *(convert<ptrdiff_t*>(value)) = printed;
                        break;
                    case len_default:
                        *(convert<int*>(value)) = printed;
                        break;
                    default:
                        throw std::invalid_argument("Unsupported length specifier");
                }
                break;
            case '@':
                result.append(print_at(value));
                break;
            default:
                throw std::invalid_argument("Unknown format specifier: '" + fmt[pos] + '\'');
                break;
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
