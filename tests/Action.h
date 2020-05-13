#ifndef ACTION_H_
#define ACTION_H_

// referenced https://stackoverflow.com/questions/11310898/how-do-i-get-the-type-of-a-variable

#include <vector>
#include <string>
#include <typeinfo>
#include "Color.h"
#include "Airport.h"

using std::vector, std::string;

// valid actions: take-off or color-move
struct Action {
    char type;
    Airport* dest;
    Color color;

    Action() :
        type(' ') {};

    Action(Airport* d) :
        type('T'),
        dest(d) {}

    Action(Color c) :
        type('C'),
        color(c) {}
    
    bool operator==(Action other) {
        // both takeoff type
        if(type == 'T' && other.type == 'T') {
            if(dest == other.dest) {
                return true;
            }
        }
        // both color-move type
        else if(type == 'C' && other.type == 'C') {
            if(color == other.color) {
                return true;
            }
        }
        // not equal
        return false;
    }

    void operator=(Action other) {
        type = other.type;
        if(type == 'T') {
            dest = other.dest;
        }
        else {
            color = other.color;
        }
    }
};



#endif