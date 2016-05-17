#ifndef LAZY_STRING_H
#define	LAZY_STRING_H

#include <string>
#include <istream>
#include <ostream>
#include <bits/shared_ptr.h>

using namespace std;


class lazy_string {

private:
    size_t beginning, sz;
    string kawaii;
    lazy_string(const lazy_string &general, size_t beginning, size_t size);

public:

    /**
     * Creates empty lazy_string (zero size and undefined capacity).
     */
    lazy_string();

    /**
     * Returns a newly constructed string object with its value initialized to a copy of a substring of this object.
     * @param character's position
     * @param number of characters
     * @return the object that starts at character position pos and spans size characters
     *         (or until the end of the string, whichever comes first).
     */

    lazy_string substr (size_t position = 0, size_t size = std::string::npos);

    /**
     * The function automatically checks whether position is the valid position of a character in the string
     * (i.e., whether position is less than the string length), throwing an out_of_range exception if it is not.
     * @param character's index.
     * @return reference to the character at position pos in the string.
     */
    const char& at(size_t position) const;

    /**
    * Returns at(size_t position).
    * @param character's position.
    * @return character at specified position.
    */
    const char& operator[](size_t position) const;

    /**
    * Extracts a lazy_string from the input stream input, storing the sequence in lazy,
    * which is overwritten (the previous value of ls is replaced).
    * @param input stream.
    * @param lazy_string.
    * @return the same as parameter input.
    */
    friend istream& operator>>(istream& input, lazy_string& lazy);

    /**
    * Inserts the sequence of characters that conforms value of lazy into output.
    * @param output stream.
    * @param lazy_string.
    * @return the same as parameter output.
    */
    friend ostream& operator<<(ostream& output, lazy_string& lazy);

    /**
     *  Returns the number of characters in the string.
     * @return string's length (size).
     */
    size_t size() const;

    /**
     * Returns the number of characters in the string.
     * @return string's length (size).
     */
    size_t length() const;

    /**
     * Creates new lazy_string from the given std::string.
     * @return lazy_string instance.
     */
    lazy_string(const string &str);

    /**
     * Creates std::string using COW method from this lazy_string.
     * @return std::string instance.
     */
    operator std::string();

};

#endif	/* LAZY_STRING_H */
