#include "format.h"

namespace Format {
    std::string char_seq(char c, unsigned n){
        std::string result = "";
        for(unsigned i = 0; i < n; i++){
            result.push_back(c);
	    }
	    return result;
    }

    std::string find_spec(const std::string &fmt, unsigned &pos, bool has_arguments){
        std::string result = "";
        while(pos < fmt.length()){
            for(; pos < fmt.length() && fmt[pos] != '%'; result.push_back(fmt[pos++]));
            if(pos == fmt.length()){
                if(has_arguments){
                    throw std::invalid_argument("Too many arguments for format");
                }
                return result;
            }
            if(pos == fmt.length() - 1){
                throw std::invalid_argument("Spurious trailing '%%' in format");
            }
            if(fmt[pos + 1] == '%'){
                result.push_back('%');
                pos += 2;
            } else {
                pos++;
                if(!has_arguments){
                    throw std::out_of_range("Need more arguments");
                }
                break;
            }
        }
        return result;
    }

    std::string format_impl(const std::string &fmt, unsigned pos, unsigned printed){
        return find_spec(fmt, pos, false);
    }

    std::string print_at(nullptr_t value){
        return "nullptr";
	}
}
