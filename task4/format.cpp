#include "format.h"

namespace Format {
    std::string char_seq(char c, unsigned n){
        std::string result = "";
        for(unsigned i = 1; i <= n; i++){
            result += c;
	    }
	    return result;
    }

    std::string find_spec(const std::string &fmt, unsigned &pos, bool has_arguments){
        std::string result = "";
        while(pos < fmt.length()){
        	while(pos < fmt.length() && fmt[pos] != '%'){
        		result += fmt[pos++];	
        	}
            if(pos >= fmt.length()){
                if(has_arguments){
                    throw std::invalid_argument("abundance of symbols");
                }
                return result;
            }
            if(pos == fmt.length() - 1){
                throw std::invalid_argument("failure in format");
            }
            if(fmt[pos + 1] != '%'){
                ++pos;
                if(!has_arguments){
                    throw std::out_of_range("lack of symbols");
                }
                break;
            } else {
                result += '%';
                pos += 2;
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
