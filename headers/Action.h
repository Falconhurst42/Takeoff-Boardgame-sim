#ifndef ACTION_H_
#define ACTION_H_

#include <vector>
#include <string>

using std::vector, std::string;

// valid actions: take-off or color-move
struct Action {
    char type;      // 'T' for take-off or 'C' for color-move
    string info;    // take-off destination key or color-move color

    Action(char t, string i) : 
        type(t), 
        info(i) {}
    
    bool operator==(Action other) {
        return (type == other.type && info == other.info);
    }
};

#endif