#include <stddef.h>
#include "format.h"

namespace Format {
    std::string sequenceOfChar(unsigned u, char c) {
        std::string final = "";
        for (unsigned i = 1; i <= u; i++){
            final += c;
        }
        return final;
    }

    std::string specification(const std::string &str, bool presence, unsigned &item) {
        std::string final = "";
        while (item < str.length()) {

            long len = str.length() - 1;

            while (item <= len && str[item] != '%') {
                final += str[item];
                ++item;
            }

            if (item == len) {
                throw std::invalid_argument("failure in format");
            }

            if (item > len) {
                if (presence) {
                    throw std::invalid_argument("abundance of symbols");
                }
                return final;
            }

            if (str[item + 1] != '%') {
                ++item;
                if (!presence) {
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

    std::string formatImplementation (const std::string &str, unsigned output, unsigned item) {
        return specification(str, false, item);
    }

    std::string ptint_at (nullptr_t value) {
        return "nullptr";
    }
}

