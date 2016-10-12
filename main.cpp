#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef uint32_t u32char;

int utf8_strlen(const char *str)
{
    int max = strlen(str);
    int count = 0;

    for (int i = 0; i < max; i++, count++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0 && c <= 127) i += 0;
        else if ((c & 0xE0) == 0xC0) i += 1;
        else if ((c & 0xF0) == 0xE0) i += 2;
        else if ((c & 0xF8) == 0xF0) i += 3;
        else return -1;//invalid utf8
    }

    return count;
}

int utf32_strlen(const u32char *str)
{
    int counter = 0;
    const u32char *pos = str;
    for (; *pos; ++pos, counter++);
    return counter;
}

u32char char8to32(const char *str)
{
    int num_bytes = strlen(str);
    if (num_bytes > sizeof(u32char)) {
        // not wide enough to store the character
        return -1;
    }
    
    u32char result = 0;

    char *result_bytes = reinterpret_cast<char*>(&result);
    for (int i = 0; i < num_bytes; i++) {
        result_bytes[i] = str[i];
    }

    return result;
}

void char32to8(u32char src, char *dst)
{
    char *src_bytes = reinterpret_cast<char*>(&src);

    for (int i = 0; i < sizeof(u32char); i++) {
        if (src_bytes[i] == 0) {
            // stop reading
            break;
        }

        dst[i] = src_bytes[i];
    }
}

void utf8to32(const char *src, u32char *dst, int size)
{
    int max = size * (int)sizeof(u32char);
    char *dst_bytes = reinterpret_cast<char*>(dst);

    const char *pos = src;

    for (int i = 0; *pos && i < max; i += sizeof(u32char)) {
        unsigned char c = (unsigned char)*pos;

        if (c >= 0 && c <= 127) {
            dst_bytes[i] = *(pos++);
        } else if ((c & 0xE0) == 0xC0) {
            dst_bytes[i] = *(pos++);
            dst_bytes[i + 1] = *(pos++);
        } else if ((c & 0xF0) == 0xE0) {
            dst_bytes[i] = *(pos++);
            dst_bytes[i + 1] = *(pos++);
            dst_bytes[i + 2] = *(pos++);
        } else if ((c & 0xF8) == 0xF0) {
            dst_bytes[i] = *(pos++);
            dst_bytes[i + 1] = *(pos++);
            dst_bytes[i + 2] = *(pos++);
            dst_bytes[i + 3] = *(pos++);
        } else {
            // invalid utf-8
            break;
        }
    }
}

u32char utf8_charat(const char *str, int index)
{
    int max = strlen(str);
    int count = 0;

    for (int i = 0; i < max; i++, count++) {
        unsigned char c = (unsigned char)str[i];

        u32char ret = 0;
        char *ret_bytes = reinterpret_cast<char*>(&ret);

        if (c >= 0 && c <= 127) {
            ret_bytes[0] = str[i];
        } else if ((c & 0xE0) == 0xC0) {
            ret_bytes[0] = str[i];
            ret_bytes[1] = str[i + 1];
            i += 1;
        } else if ((c & 0xF0) == 0xE0) {
            ret_bytes[0] = str[i];
            ret_bytes[1] = str[i + 1];
            ret_bytes[2] = str[i + 2];
            i += 2;
        } else if ((c & 0xF8) == 0xF0) {
            ret_bytes[0] = str[i];
            ret_bytes[1] = str[i + 1];
            ret_bytes[2] = str[i + 2];
            ret_bytes[3] = str[i + 3];
            i += 3;
        } else {
            // invalid utf-8
            break;
        }

        if (index == count) {
            // reached index
            return ret;
        }
    }

    // error
    return -1;
}

void utf8_charat(const char *str, char *dst, int index)
{
    char32to8(utf8_charat(str, index), dst);
}

class Utf8String {
public:
    Utf8String();
    Utf8String(const char *str);
    Utf8String(const Utf8String &other);
    ~Utf8String();

    Utf8String &operator=(const char *str);
    Utf8String &operator=(const Utf8String &other);

    inline size_t Length() const
    {
        if (m_data == nullptr) {
            return 0;
        }

        return utf8_strlen(m_data);
    }

    inline u32char CharAt(int index) const
    {
        u32char result;

        if (m_data == nullptr || ((result = utf8_charat(m_data, index) == ((u32char)-1)))) {
            throw std::out_of_range("index out of range");
        }

        return result;
    }

private:
    char *m_data;
};

Utf8String::Utf8String()
    : m_data(nullptr)
{
}

Utf8String::Utf8String(const char *str)
{
    size_t len = strlen(str) + 1;
    m_data = new char[len];
    strncpy(m_data, str, len);
}

Utf8String::Utf8String(const Utf8String &other)
{
    size_t len = strlen(other.m_data) + 1;
    m_data = new char[len];
    strncpy(m_data, other.m_data, len);
}

Utf8String::~Utf8String()
{
    if (m_data != nullptr) {
        delete[] m_data;
    }
}

Utf8String &Utf8String::operator=(const char *str)
{
    if (m_data != nullptr) {
        delete[] m_data;
    }

    size_t len = strlen(str) + 1;
    m_data = new char[len];
    strncpy(m_data, str, len);

    return *this;
}

Utf8String &Utf8String::operator=(const Utf8String &other)
{
    if (m_data != nullptr) {
        delete[] m_data;
    }

    size_t len = strlen(other.m_data) + 1;
    m_data = new char[len];
    strncpy(m_data, other.m_data, len);

    return *this;
}

int main()
{
    std::cout << "Reading utf-8 file \"utf-8-test.txt\"...\n";
    std::ifstream file("utf-8-test.txt", std::ios::in | std::ios::ate | std::ios::binary);

    if (file.is_open()) {
        size_t len = file.tellg();
        file.seekg(0, std::ios::beg);

        char buffer[2048];
        memset(buffer, 0, 2048);

        file.read(buffer, len);

        std::cout << "strlen(buffer) = " << strlen(buffer) << "\n";
        std::cout << "utf8_strlen(buffer) = " << utf8_strlen(buffer) << "\n";


        u32char utf32_test[500];
        memset(utf32_test, 0, sizeof(utf32_test));
        utf8to32(buffer, utf32_test, 500);

        std::cout << "utf32_strlen(utf32_test) = " << utf32_strlen(utf32_test) << "\n";

        { // convert utf-32 char to utf-8 char array
            char utf8_char_buffer[4] = { 0 };
            utf8_charat(buffer, utf8_char_buffer, 8);

            std::cout << "utf8_charat(buffer, 8) = " << utf8_char_buffer << "\n";
        }

        std::cout << "\n";
        std::cout << "std::cout << buffer = " << buffer << "\n";

        /*std::cout << "\nConverting to UTF-16...\n";
        
        std::u16string utf16str = utf8_to_utf16(buffer);
        std::cout << "length of utf-16 string = " << utf16str.length() << "\n";*/
/*
        std::cout << "\n\n";
        char utf8_test[] = "ʥ";
        int num_bytes = strlen(utf8_test);

        //u16char test16 = char8to16(utf8_test);
        u32char test32 = char8to32(utf8_test);
        
        std::cout << "strlen(utf8_test) = " << num_bytes << "\n";
        std::cout << "utf8_strlen(utf8_test) = " << utf8_strlen(utf8_test) << "\n";
        std::cout << "\n\n";*/

        /*std::cout << "utf-16:\t";
        const char *test_bytes16 = reinterpret_cast<const char*>(&test16);
        for (int i = 0; i < sizeof(test16); i++) {
            printf("0x%02x ", (unsigned char)test_bytes16[i]);
        }
        std::cout << "\n\n";*/

        /*const char *test_bytes32 = reinterpret_cast<const char*>(&test32);
        std::cout << "utf-32:\t";
        for (int i = 0; i < sizeof(test32); i++) {
            printf("0x%02x ", (unsigned char)test_bytes32[i]);
        }
        std::cout << "\n\n";*/

        /*std::cout << "utf-32:\t";
        const char *dst_buffer_bytes = reinterpret_cast<const char *>(dst_buffer);
        for (int i = 0; i < sizeof(dst_buffer); i++) {
            printf("0x%02x ", (unsigned char)dst_buffer_bytes[i]);
        }
        std::cout << "\n\n";*/

    } else {
        std::cout << "Could not open file.\n";
    }
}