#ifndef AIRPORT_H_
#define AIRPORT_H_

#include <utility>
#include <vector>
#include <string>

#include "Airplane.h"
#include "Color.h"

using std::vector, std::pair, std::string;

struct Airport {
    string name;        // used to check start and end, key and display name
    float lat, lon;
    // routes stored as <Airport*, Color>
    vector<pair<Airport*, Color>> children, parents;
    vector<Airplane*> occupants;
    vector<Airplane*> shadow_occupants;

    Airport(string n, float lati = 0, float longi = 0) : 
        name(n),
        lat(lati),
        lon(longi),
        occupants(0, NULL),
        shadow_occupants(0, NULL) {}

    static void connect(Airport* parent, Airport* child, Color c) {
        // enforce right to left motion
        if(child->lon > parent->lon) {
            std::swap(parent, child);
        }

        // add connections
        parent->children.push_back(std::make_pair(child, c));
        child->parents.push_back(std::make_pair(parent, c));
    }
};

#endif