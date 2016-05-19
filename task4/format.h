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
        bool array[8] = {false};
        int width = 0,
                precision = -1;
        char type;
        enum length_t length = standart;
    };

    template<typename To, typename From> typename
            std::enable_if<std::is_convertible<From, To>::value, To>::type convert(From value){
        return (To) value;
    }

    template<typename To, typename From> typename
            std::enable_if<!std::is_convertible<From, To>::value, To>::type convert(From value){
        throw std::invalid_argument("Invalid argument type");
    }

    std::string find_spec(const std::string &fmt, unsigned &pos, bool has_arguments);

    std::string format_impl(const std::string &fmt, unsigned pos, unsigned printed);

    std::string char_seq(char c, unsigned n);

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
                    std::enable_if<!std::is_integral<T>::value && !std::is_convertible<T,
                    std::string>::value && !std::is_pointer<T>::value, std::string>::type print_at(const T& value){
        throw std::invalid_argument("Invalid argument type");
    }

    template<typename T> typename std::enable_if<std::is_integral<T>::value, std::string>::type print_at(T value){
        return std::to_string(value);
    }

    template<typename T, int num> typename
                    std::enable_if<!std::is_convertible<T*,
                    std::string>::value,
                    std::string>::type print_at(const T (&a)[num]) {

        std::string r = "[";
        for(int i = 0; i < num - 1; i++){
            r += (std::to_string(a[i]) + ", ");
        }
        r += (std::to_string(a[num - 1]) + ']');
        return r;
    }

    template<typename T> typename
                  std::enable_if<std::is_convertible<T,
                  std::string>::value,
                  std::string>::type print_at(const T& value){

        return value;

    }

    template<typename T> typename
            std::enable_if<!std::is_array<T>::value && !std::is_convertible<T,
            std::string>::value && std::is_pointer<T>::value,
            std::string>::type print_at(T& value){

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

    /*
     * Builds string with the given format specifier and the given argument
     * If the argument does not match its specifier, it throws an exception
     */

    template<typename T> typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type print_num(format_t fm, T value){

        if(!fm.array[6] && fm.precision >= 0 && fm.array[4]) {
            fm.array[4] = false;
        }
        if(!fm.array[6] && fm.precision < 0 ){
            fm.precision = 1;
        }

        std::string temp = "%";

        if(fm.array[0]){temp += '+';}
        if(fm.array[1]){temp += '-';}
        if(fm.array[2]){temp += ' ';}
        if(fm.array[3]){temp += '#';}
        if(fm.array[4]){temp += '0';}
        if(fm.precision >= 0){
            temp += '.';
            if (fm.precision > 1024){
                temp += std::to_string(1024);
            } else {
                temp += std::to_string(fm.precision);
            }
        }
        char buffer[2048];

        if (fm.array[6] && fm.length == L)
        {temp += 'L';}
        if (fm.array[6] && fm.length == l)
        {temp += 'l';}
        if (fm.array[6])
        {temp += fm.type;}
        if(!fm.array[6]){
            temp += 'j';
            temp += fm.type;
        }

        snprintf(buffer, sizeof(buffer), temp.c_str(), value);

        std::string r = buffer;

        if(fm.precision > 1024 && r.size() > 512 && fm.array[6]){
            r += char_seq('0', fm.precision - r.size() + r.find_first_of('.') + 1);
        }
        if(fm.precision > 1024 && r.size() > 512 && !fm.array[6]){
            if (r[0]=='0'){
                r = r.substr(0, 2) + char_seq('0', fm.precision - r.size()) + r.substr(2);
            } else {
                r = r.substr(0, 2) + char_seq('0', fm.precision - r.size() + 1) + r.substr(2);
            }
        }


        if((unsigned) fm.width > r.size()){
            if(fm.array[1]){
                r +=char_seq(' ', fm.width - r.size());
            } else {
                if(fm.array[4]){
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

        intmax_t d;
        uintmax_t u;
        double f;
        char nil_p[6];

        while(pos < fmt.length() && (fmt[pos] == '-' || fmt[pos] == '+' || fmt[pos] == ' ' || fmt[pos] == '#' || fmt[pos] == '0')){
            if (fmt[pos] == '-') {
                fm.array[1] = true;
                fm.array[4] = false;
            } else if (fmt[pos] == '+') {
                fm.array[0] = true;
                fm.array[2] = false;
            } else if (fmt[pos] == ' ') {
                fm.array[2] = !fm.array[0];
            } else if (fmt[pos] == '#') {
                fm.array[3] = true;
            } else if (fmt[pos] == '0') {
                fm.array[4] = !fm.array[1];
            }
            ++pos;
        }

        if(pos < fmt.length() && fmt[pos] == '*'){
            fm.width = convert<int>(value);
            if(fm.width < 0){
                fm.width *= -1;
                fm.array[1] = true;
                fm.array[4] = false;
            }
            temp = "%";
            if(fm.array[0]){temp += '+';}
            if(fm.array[1]){temp += '-';}
            if(fm.array[2]){temp += ' ';}
            if(fm.array[3]){temp += '#';}
            if(fm.array[4]){temp += '0';}
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
                if(fm.array[0]){temp += '+';}
                if(fm.array[1]){temp += '-';}
                if(fm.array[2]){temp += ' ';}
                if(fm.array[3]){temp += '#';}
                if(fm.array[4]){temp += '0';}
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
                if (fm.length == h) {
                    fm.length = hh;
                } else{
                    if (fm.length ==  standart){
                        fm.length = h;
                    } else {
                        fm.length = error;
                    }
                }
            } else if (fmt[pos]== 'l') {
                if (fm.length == l) {
                    fm.length = ll;
                } else{
                    if (fm.length == standart){
                        fm.length = l;
                    } else {
                        fm.length = error;
                    }
                }
            } else if (fmt[pos]=='j') {
                if (fm.length == standart) {
                    fm.length = j;
                } else {
                    fm.length = error;
                }
            } else if (fmt[pos]== 'z') {
                if (fm.length == standart) {
                    fm.length = z;
                } else {
                    fm.length = error;
                }
            }else if (fmt[pos]== 't') {
                if (fm.length == standart) {
                    fm.length = t;
                } else {
                    fm.length = error;
                }
            } else if (fmt[pos]== 'L') {
                if (fm.length == standart) {
                    fm.length = L;
                } else {
                    fm.length = error;
                }
            }
            ++ pos;
        }

        if(fm.length == error){
            throw std::invalid_argument("Unknown length specifier");
        }

        if(pos == fmt.length()){
            throw std::invalid_argument("Сonversion lacks type at end of format");
        }

        std::stringstream out;
        if(fm.array[0]){
            out << std::showpos;
        }
        if(fm.array[1]){
            out << std::left;
        }
        if(fm.width != 0){
            out.width(fm.width);
        }
        if(fm.precision >= 0){
            out.precision(fm.precision);
        }
        if(fm.array[3]){
            out << std::showbase << std::showpoint;
        }

        fm.type = fmt[pos++];
        switch(fm.type){
            case 'd':
            case 'i':
                if (fm.length == hh) {
                    d = convert<signed char>(value);
                } else if (fm.length == h) {
                    d = convert<short int>(value);
                } else if (fm.length == l) {
                    d = convert<long int>(value);
                } else if (fm.length == ll) {
                    d = convert<long long int>(value);
                } else if (fm.length == j) {
                    d = convert<intmax_t>(value);
                } else if (fm.length == z) {
                    d = convert<size_t>(value);
                } else if (fm.length == t) {
                    d = convert<ptrdiff_t>(value);
                } else if (fm.length == standart) {
                    d = convert<int>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                result += print_num(fm, d);
                break;
            case 'X':
                fm.array[5] = true;
            case 'x':
            case 'o':
            case 'u':
                if (fm.length == hh) {
                    u = convert<unsigned char>(value);
                } else if (fm.length == h) {
                    u = convert<unsigned short int>(value);
                } else if (fm.length == l) {
                    u = convert<unsigned long int>(value);
                } else if (fm.length == ll) {
                    u = convert<unsigned long long int>(value);
                } else if (fm.length == j) {
                    u = convert<uintmax_t>(value);
                } else if (fm.length == z) {
                    u = convert<size_t>(value);
                } else if (fm.length == t) {
                    u = convert<ptrdiff_t>(value);
                } else if (fm.length == standart) {
                    u = convert<unsigned int>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                result += print_num(fm, u);
                break;
            case 'E':
            case 'G':
            case 'A':
                fm.array[5] = true;
            case 'e':
            case 'g':
            case 'a':
            case 'F':
            case 'f':
                fm.array[6] = true;
                if (fm.length == l){
                } else if (fm.length == standart) {
                    f = convert<double>(value);
                } else if (fm.length == L) {
                    f = convert<long double>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                result += print_num(fm, f);
                break;
            case 'c':
                if (fm.length == l) {
                } else if (fm.length == standart) {
                    out << convert<unsigned char>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                result += out.str();
                break;
            case 's': {
                std::string str;
                if (fm.length == l) {
                } else if (fm.length == standart) {
                    str = convert<std::string>(value);
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                if(fm.precision >= 0 && str.length() > (unsigned) fm.precision){
                    str = str.substr(0, fm.precision);
                }
                out << str;
                result += out.str();
            }
                break;
            case 'p':
                if(fm.length != standart){
                    throw std::invalid_argument("Unsupported length specifier");
                }
                out << std::setfill(fm.array[4] ? '0' : ' ');
                snprintf(nil_p, 2, "%p", convert<void*>(value));
                if(nil_p[0] != '(' && convert<void*>(value) != NULL && convert<void*>(value) != nullptr){
                    out << convert<void*>(value);
                } else {
                    out << "(nil)";
                }
                result += out.str();
                break;
            case 'n':
                printed += result.length();
                if (fm.length == hh) {
                    *(convert<signed char*>(value)) = printed;
                } else if (fm.length == h) {
                    *(convert<short int*>(value)) = printed;
                } else if (fm.length == l) {
                    *(convert<long int*>(value)) = printed;
                } else if (fm.length == ll) {
                    *(convert<long long int*>(value)) = printed;
                } else if (fm.length == j) {
                    *(convert<intmax_t*>(value)) = printed;
                } else if (fm.length == z) {
                    *(convert<size_t*>(value)) = printed;
                } else if (fm.length == t) {
                    *(convert<ptrdiff_t*>(value)) = printed;
                } else if (fm.length == standart) {
                    *(convert<int*>(value)) = printed;
                } else {
                    throw std::invalid_argument("Unsupported length specifier");
                }
                break;
            case '@':
                result += print_at(value);
                break;
            default:
                throw std::invalid_argument("Unknown format specifier: '" + fmt[pos] + '\'');
                break;
        }

        return result + format_impl(fmt, pos, printed + result.length(), args...);
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
    return Format::format_impl(fmt, 0, 0, args...);
}

#endif

