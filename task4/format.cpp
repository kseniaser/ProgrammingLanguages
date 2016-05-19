#include <stddef.h>
#include "format.h"

namespace Format {
    std::string sequenceOfChar(char c, unsigned n){
        std::string result = "";
        for(unsigned i = 0; i < n; i++){
            result +=c;
        }
        return result;
    }

    std::string specification(const std::string &str, unsigned &item, bool presences){
        std::string final = "";
        while(item < str.length()){
            for(; item < str.length() && str[item] != '%'; final += str[item++]);
            //while (item < str.length() && str[item] != '%'){
            //    final += str[item++];
            //}
            if(item == str.length() - 1){
                throw std::invalid_argument("failure in format");
            }

            if(item == str.length()){
                if(presences){
                    throw std::invalid_argument("abundance of symbols");
                }
                return final;
            }

            if(str[item + 1] != '%'){
                ++item;
                if(!presences){
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

    std::string formatImplementation(const std::string &str, unsigned item, unsigned output){
        return specification(str, item, false);
    }

    std::string print_at(nullptr_t value){
        return "nullptr";
    }
}
  
