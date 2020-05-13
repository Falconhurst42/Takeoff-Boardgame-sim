#ifndef COLOR_H_
#define COLOR_H_

#include <cctype>
#include <vector>
#include <string>

using std::vector, std::string;

// valid colors: red, orange, yellow, green, blue, purple
struct Color {
    char c;
    const vector<string> DEFUALT_COLORS = {"red", "orange", "yellow", "green", "blue", "purple"};

    Color(string col = "white") : 
        c ( std::toupper( col[0] ) ) {}

    bool operator==(Color other) {
        return c == other.c;
    }
};

#endif