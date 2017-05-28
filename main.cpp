#include <iostream>
#include <fstream>

#include "simple-utf-8.hpp"

using namespace utf;

int main()
{
    utf::init();

    utf::cout << "Reading utf-8 file \"utf-8-test.txt\"...\n";
    std::ifstream file("utf-8-test.txt", std::ios::in | std::ios::ate | std::ios::binary);

    if (file.is_open()) {
        // read file with utf8 text into buffer
        size_t len = file.tellg();
        file.seekg(0, std::ios::beg);

        char *buffer = new char[len + 1];
        buffer[len] = '\0';
        file.read(buffer, len);

        // create utf8 string from buffer
        utf::String file_contents(buffer);
        // contents now copied, delete buffer
        delete[] buffer;

        utf::cout << "file_contents: " << file_contents << "\n";

        utf::String str("Hello");
        utf::cout << "str: " << str << "\n";
        // overloaded operators (also includes <, <=, >, >=, ==, etc.)
        str += " World";
        utf::cout << "str: " << str << "\n";

        utf::cout << "str == \"Hello World\": " << (str == "Hello World") << "\n";
    } else {
        utf::cout << "Could not open file.\n";
    }

    return 0;
}