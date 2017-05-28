#ifndef SIMPLE_UTF_8_HPP
#define SIMPLE_UTF_8_HPP

#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>

#ifdef __MINGW32__
#undef _WIN32
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // do not allow windows.h to define 'max' and 'min'
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cwchar>
#endif

namespace utf {

#ifdef _WIN32
// define typedefs to streams
typedef std::wostream utf8_ostream;
typedef std::wofstream utf8_ofstream;
typedef std::wistream utf8_istream;
typedef std::wifstream utf8_ifstream;

static utf8_ostream &cout = std::wcout;
static utf8_istream &cin = std::wcin;
static auto &printf = std::wprintf;
static auto &sprintf = wsprintf;
static auto &fputs = std::fputws;

#define PRIutf8s "ls"
#define UTF8_CSTR(str) L##str

inline std::vector<wchar_t> ToWide(const char *str)
{
    std::vector<wchar_t> buffer;
    buffer.resize(MultiByteToWideChar(CP_UTF8, 0, str, -1, 0, 0));
    MultiByteToWideChar(
        CP_UTF8,
        0, str,
        -1,
        &buffer[0],
        buffer.size()
    );
    return buffer;
}

#define UTF8_TOWIDE(str) utf::ToWide(str).data()

#else

// define typedefs to streams
typedef std::ostream utf8_ostream;
typedef std::ofstream utf8_ofstream;
typedef std::istream utf8_istream;
typedef std::wifstream utf8_ifstream;
static utf8_ostream &cout = std::cout;
static utf8_istream &cin = std::cin;
static auto &printf = std::printf;
static auto &sprintf = std::sprintf;
static auto &fputs = std::fputs;
#define PRIutf8s "s"
#define UTF8_CSTR(str) str
#define UTF8_TOWIDE(str) str

#endif

typedef uint32_t u32char;

inline void init()
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_U16TEXT);
#endif
}

inline int utf8_strlen(const char *str)
{
    const int max = std::strlen(str);

    int i;
    int count;

    for (i = 0, count = 0; i < max; i++, count++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0 && c <= 127) {
            i += 0;
        } else if ((c & 0xE0) == 0xC0) {
            i += 1;
        } else if ((c & 0xF0) == 0xE0) {
            i += 2;
        } else if ((c & 0xF8) == 0xF0) {
            i += 3;
        } else {
            return -1; //invalid
        }
    }

    return count;
}

inline int utf8_strcmp(const char *s1, const char *s2)
{
    while (*s1 || *s2) {
        unsigned char c;

        u32char c1 = 0;
        char *c1_bytes = reinterpret_cast<char*>(&c1);

        u32char c2 = 0;
        char *c2_bytes = reinterpret_cast<char*>(&c2);

        // get the character for s1
        c = (unsigned char)*s1;

        if (c >= 0 && c <= 127) {
            c1_bytes[0] = *(s1++);
        } else if ((c & 0xE0) == 0xC0) {
            c1_bytes[0] = *(s1++);
            c1_bytes[1] = *(s1++);
        } else if ((c & 0xF0) == 0xE0) {
            c1_bytes[0] = *(s1++);
            c1_bytes[1] = *(s1++);
            c1_bytes[2] = *(s1++);
        } else if ((c & 0xF8) == 0xF0) {
            c1_bytes[0] = *(s1++);
            c1_bytes[1] = *(s1++);
            c1_bytes[2] = *(s1++);
            c1_bytes[3] = *(s1++);
        }

        // get the character for s2
        c = (unsigned char)*s2;

        if (c >= 0 && c <= 127) {
            c2_bytes[0] = *(s2++);
        } else if ((c & 0xE0) == 0xC0) {
            c2_bytes[0] = *(s2++);
            c2_bytes[1] = *(s2++);
        } else if ((c & 0xF0) == 0xE0) {
            c2_bytes[0] = *(s2++);
            c2_bytes[1] = *(s2++);
            c2_bytes[2] = *(s2++);
        } else if ((c & 0xF8) == 0xF0) {
            c2_bytes[0] = *(s2++);
            c2_bytes[1] = *(s2++);
            c2_bytes[2] = *(s2++);
            c2_bytes[3] = *(s2++);
        }

        if (c1 < c2) {
            return -1;
        } else if (c1 > c2) {
            return 1;
        }
    }

    return 0;
}

inline char *utf8_strcpy(char *dst, const char *src)
{
    // copy all raw bytes
    for (int i = 0; (dst[i] = src[i]) != '\0'; i++);
    return dst;
}

inline char *utf8_strncpy(char *dst, const char *src, size_t n)
{
    const size_t max = std::strlen(dst) + 1;

    size_t i;
    size_t count;

    for (i = 0, count = 0; src[i] != '\0'; i++, count++) {

        if (count == n) {
            // finished copying, jump to end
            break;
        }

        unsigned char c = (unsigned char)src[i];

        if (c >= 0 && c <= 127) {
            dst[i] = src[i];
        } else if ((c & 0xE0) == 0xC0) {
            dst[i] = src[i];
            dst[i + 1] = src[i + 1];
            i += 1;
        } else if ((c & 0xF0) == 0xE0) {
            dst[i] = src[i];
            dst[i + 1] = src[i + 1];
            dst[i + 2] = src[i + 2];
            i += 2;
        } else if ((c & 0xF8) == 0xF0) {
            dst[i] = src[i];
            dst[i + 1] = src[i + 1];
            dst[i + 2] = src[i + 2];
            dst[i + 3] = src[i + 3];
            i += 3;
        } else {
            break; // invalid utf-8
        }
    }

    // fill rest with NUL bytes
    for (; i < max; i++) {
        dst[i] = '\0';
    }

    return dst;
}

inline char *utf8_strcat(char *dst, const char *src)
{
    while (*dst) {
        dst++;
    }
    while ((*dst++ = *src++));
    return --dst;
}

inline u32char char8to32(const char *str)
{
    const int num_bytes = std::strlen(str);

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

inline void char32to8(u32char src, char *dst)
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

inline char *get_bytes(u32char &ch) { return reinterpret_cast<char*>(&ch); }

inline void utf8to32(const char *src, u32char *dst, int size)
{
    const int max = size * (int)sizeof(u32char);

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

inline u32char utf8_charat(const char *str, int index)
{
    const int max = std::strlen(str);
    int i;
    int count;

    for (i = 0, count = 0; i < max; i++, count++) {
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

inline void utf8_charat(const char *str, char *dst, int index)
{
    char32to8(utf8_charat(str, index), dst);
}

class String {
public:
    String()
        : m_data(new char[1]),
          m_size(1),
          m_length(0)
    {
        m_data[0] = '\0';
    }

    explicit String(size_t size)
        : m_data(new char[size + 1]),
          m_size(size + 1),
          m_length(0)
    {
        std::memset(m_data, 0, m_size);
    }

    String(const char *str)
    {
        if (str == nullptr) {
            m_data = new char[1];
            m_data[0] = '\0';
            m_size = 1;
            m_length = 0;
        } else {
            // copy raw bytes
            m_size = std::strlen(str) + 1;
            m_data = new char[m_size];
            std::strcpy(m_data, str);
            // recalculate length
            m_length = utf8_strlen(m_data);
        }
    }

    String(const char *str, size_t size)
    {
        if (str == nullptr) {
            m_size = size;
            m_data = new char[m_size];
            m_data[0] = '\0';
            m_length = 0;
        } else {
            // copy raw bytes
            m_size = std::max(std::strlen(str), size) + 1;
            m_data = new char[m_size];
            std::strcpy(m_data, str);
            // recalculate length
            m_length = utf8_strlen(m_data);
        }
    }

    String(const String &other)
    {
        // copy raw bytes
        m_size = other.m_size;
        m_data = new char[m_size];
        std::strcpy(m_data, other.m_data);
        m_length = other.m_length;
    }

    ~String()
    {
        if (m_data != nullptr) {
            delete[] m_data;
        }
    }

    inline char *GetData() { return m_data; }
    inline char *GetData() const { return m_data; }
    inline size_t GetBufferSize() const { return m_size; }
    inline size_t GetLength() const { return m_length; }

    String &operator=(const char *str)
    {
        // check if there is enough space to not have to delete the data
        const int len = std::strlen(str) + 1;

        if (m_data != nullptr && m_size >= len) {
            std::strcpy(m_data, str);
            m_length = utf8_strlen(m_data);
        } else {
            // must delete the data if not null
            if (m_data) {
                delete[] m_data;
            }

            if (!str) {
                m_size = 1;
                m_data = new char[m_size];
                m_data[0] = '\0';
                m_length = 0;
            } else {
                // copy raw bytes
                m_size = len;
                m_data = new char[m_size];
                std::strcpy(m_data, str);
                // recalculate length
                m_length = utf8_strlen(m_data);
            }
        }

        return *this;
    }

    String &operator=(const String &other)
        { return operator=(other.m_data); }
    inline bool operator==(const char *str) const
        { return !(strcmp(m_data, str)); }
    inline bool operator==(const String &other) const
        { return !(strcmp(m_data, other.m_data)); }
    inline bool operator<(const char *str) const
        { return (utf8_strcmp(m_data, str) == -1); }
    inline bool operator<(const String &other) const
        { return (utf8_strcmp(m_data, other.m_data) == -1); }
    inline bool operator>(const char *str) const
        { return (utf8_strcmp(m_data, str) == 1); }
    inline bool operator>(const String &other) const
        { return (utf8_strcmp(m_data, other.m_data) == 1); }

    inline bool operator<=(const char *str) const
    {
        const int i = utf8_strcmp(m_data, str);
        return i == 0 || i == -1;
    }

    inline bool operator<=(const String &other) const
    {
        const int i = utf8_strcmp(m_data, other.m_data);
        return i == 0 || i == -1;
    }

    inline bool operator>=(const char *str) const
    {
        const int i = utf8_strcmp(m_data, str);
        return i == 0 || i == 1;
    }

    inline bool operator>=(const String &other) const
    {
        const int i = utf8_strcmp(m_data, other.m_data);
        return i == 0 || i == 1;
    }

    String operator+(const char *str) const
    {
        String result(m_length + strlen(str));

        utf8_strcpy(result.m_data, m_data);
        utf8_strcat(result.m_data, str);

        // calculate length
        result.m_length = utf8_strlen(result.m_data);

        return result;
    }

    String operator+(const String &other) const
    {
        String result(m_length + other.m_length);

        utf8_strcpy(result.m_data, m_data);
        utf8_strcat(result.m_data, other.m_data);

        // calculate length
        result.m_length = utf8_strlen(result.m_data);

        return result;
    }

    String &operator+=(const char *str)
    {
        const size_t this_len = std::strlen(m_data);
        const size_t other_len = std::strlen(str);
        const size_t new_size = this_len + other_len + 1;

        if (new_size <= m_size) {
            std::strcat(m_data, str);
        } else {
            // we must delete and recreate the array
            m_size = new_size;

            char *new_data = new char[m_size];
            std::strcpy(new_data, m_data);
            std::strcat(new_data, str);
            delete[] m_data;
            m_data = new_data;
        }
        
        // recalculate length
        m_length = utf8_strlen(m_data);

        return *this;
    }

    String &operator+=(const String &other)
    {
        return operator+=(other.m_data);
    }

    u32char operator[](size_t index) const
    {
        u32char result;
        if (m_data == nullptr || ((result = utf8_charat(m_data, index)) == -1)) {
            throw std::out_of_range("index out of range");
        }
        return result;
    }

    friend utf8_ostream &operator<<(utf8_ostream &os, const String &str)
    {
#ifdef _WIN32
        std::vector<wchar_t> buffer;
        buffer.resize(MultiByteToWideChar(CP_UTF8, 0, str.m_data, -1, 0, 0));
        MultiByteToWideChar(CP_UTF8, 0, str.m_data, -1, &buffer[0], buffer.size());
        os << buffer.data();
#else
        os << str.m_data;
#endif
        return os;
    }

private:
    char *m_data;
    size_t m_size; // buffer size (not length)
    size_t m_length;
};

} // namespace utf

#endif
