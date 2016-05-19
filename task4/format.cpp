#include "format.h"

namespace Format {
    std::string seq(char c, unsigned n){
        std::string final = "";
        for(unsigned i = 1; i <= n; i++){
            final += c;
        }
        return final;
    }

    std::string spec(const std::string &fmt, unsigned &item, bool notEmpty){
        std::string final = "";
        while(item < fmt.length()){
            while(item < fmt.length() && fmt[item] != '%'){
                final += fmt[item++];
            }
            if(item >= fmt.length()){
                if(notEmpty){
                    throw std::invalid_argument("abundance of symbols");
                }
                return final;
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
                final += '%';
                item += 2;
            }
        }
        return final;
    }

    std::string impl(const std::string &fmt, unsigned item, unsigned output){
        return spec(fmt, item, false);
    }

    std::string printVal(nullptr_t value){
        return "nullptr";
    }
}
