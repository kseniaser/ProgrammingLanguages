#include "format.h"

namespace Format {
    std::string seq(char c, unsigned n){
        std::string result = "";
        for(unsigned i = 1; i <= n; i++){
            result += c;
        }
        return result;
    }

    std::string spec(const std::string &fmt, unsigned &item, bool notEmpty){
        std::string result = "";
        while(item < fmt.length()){
            while(item < fmt.length() && fmt[item] != '%'){
                result += fmt[item++];
            }
            if(item >= fmt.length()){
                if(notEmpty){
                    throw std::invalid_argument("abundance of symbols");
                }
                return result;
            }
            if(item == fmt.length() - 1){
                throw std::invalid_argument("failure in format");
            }
            if(fmt[item + 1] != '%'){
                ++item;
                if(!notEmpty){
                    throw std::out_of_range("lack of symbols");
                }
                break;
            } else {
                result += '%';
                item += 2;
            }
        }
        return result;
    }

    std::string impl(const std::string &fmt, unsigned item, unsigned output){
        return spec(fmt, item, false);
    }

    std::string print_at(nullptr_t value){
        return "nullptr";
    }
}
