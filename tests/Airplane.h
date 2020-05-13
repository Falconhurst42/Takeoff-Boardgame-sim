#ifndef AIRPLANE_H_
#define AIRPLANE_H_

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <utility>
#include <vector>
#include <string>

#include "Color.h"
#include "Action.h"
#include "Airport.h"



class Player;

using std::vector, std::pair, std::string, std::cout;

class Airplane {
    private:
        Airport* location;
        Airport* shadow_location;
        Airport* start;
        Airport* end;
        const Player* owner;

        // airpost occupancy accessor function which accounts for main-shadow distinction
        vector<Airplane*>& get_occupancy_special(Airport* loc, bool main_board);

        // access location variable, which one depends on main-shadow flag
        Airport*& get_location_special(bool main_board);

        // if plane gets bumped, do this
        bool bumped(bool main_board);

        // updates loc, returns nothing for now
        bool move(Airport* dest, bool main_board);

        // jumps plane to given destination (key), returns success of jump (false if dest does not exist in map)
        bool jump(Airport* dest, bool main_board);

        // moves plane along its child route with the given color
        // returns true if route existed and plane was moved
        bool move_route(Color c, bool main_board);

        Airport* identify_dest(Color c, bool main_board);

    public:
        // basic constructor, defaults position to start
        Airplane(const Player* own = NULL, Airport* loc = NULL, Airport* st = NULL, Airport* en = NULL);
        
        Airplane(const Airplane& other);

        // does action (takeoff or color move)
        // returns success
        bool do_action(Action act, bool main_board);

        // checks whether plane should be bumping
        // returns whether bump occured
        // throw std::domain_error if bumping own plane
        bool check_bump(bool main_board);

        // sync shadow location with real location
        void sync();

        // Accessors
        string get_loc_name(bool main_board);

        Airport* get_loc(bool main_board);

        const Player* get_owner_ptr();

        void set_owner_ptr(Player* own);
};

#endif