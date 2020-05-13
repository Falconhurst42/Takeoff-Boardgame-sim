#ifndef AIRPLANE_H_
#define AIRPLANE_H_

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

#include "Airport.h"
#include "Player.h"
#include "Action.h"
#include "Color.h"

using std::vector, std::pair, std::string, std::cout;

class Airplane {
    private:
        std::unordered_map<string, Airport> map;
        Airport* location;
        Airport* shadow_location;
        const Player* owner;

        // airpost occupancy accessor function which accounts for main-shadow distinction
        vector<Airplane*>& get_occupancy_special(Airport* loc, bool main_board) {
            if(main_board) {
                return loc->occupants;
            }
            else {
                return loc->shadow_occupants;
            }
        }

        // access location variable, which one depends on main-shadow flag
        Airport*& get_location_special(bool main_board) {
            if(main_board) {
                return location;
            }
            else {
                return shadow_location;
            }
        }

        // if plane gets bumped, do this
        bool bumped(bool main_board) {
            get_location_special(main_board) = &(map.at("START"));
            get_occupancy_special(&(map.at("START")), main_board).push_back(this);
        }

        // updates loc, returns nothing for now
        bool move(Airport* dest, bool main_board) {
            // remove this plane from the old location
            if(main_board && DEBUG) {
                cout << "Moved from " << location->name << " to " << dest->name << ". ";
            }
            std::remove(get_occupancy_special(get_location_special(main_board), main_board).begin(), get_occupancy_special(get_location_special(main_board), main_board).end(), this);
            // add it to the new one
            get_occupancy_special(dest, main_board).push_back(this);
            // update location
            get_location_special(main_board) = dest;
            return true;            
        }

        // jumps plane to given destination (key), returns success of jump (false if dest does not exist in map)
        bool jump(string dest, bool main_board) {
            try {
                // avoid stationary jumps (will throw error if dest is invalid)
                if(get_location_special(main_board) == &(map.at(dest))) {
                    // in stationary jump, nothing needs to be done
                    return true;
                }
                // else move
                move(&(map.at(dest)), main_board);
                return true;
            }
            // case that dest (or location) does not exist in map
            catch(const std::out_of_range& e) {
                return false;
            }
        }

        // moves plane along its child route with the given color
        // returns true if route existed and plane was moved
        bool move_route(Color c, bool main_board) {
            // find destination
            Airport* loc = get_location_special(main_board);
            for(int i = 0; i < loc->children.size(); i++) {
                // if the color of the rout matches
                if(loc->children[i].second == c) {
                    // move to the destination of the route
                    move(loc->children[i].first, main_board);
                    // return success
                    return true;
                }
            }
            // return failure if you make it here
            return false;
        }

        Airport* identify_dest(Color c, bool main_board) {
            Airport* loc = get_location_special(main_board);
            for (int i = 0; i < loc->children.size(); i++) {
                // if the color of the rout matches
                if(loc->children[i].second == c) {
                    // return dest
                    return loc->children[i].first;
                }
            }
            // return failure if you make it here
            return NULL;
        }

    public:
        // basic constructor, defaults position to start
        Airplane(const Player* own = NULL, const string& loc = "START") : 
            owner(own), 
            map((owner->get_game())->map),
            location( &(map.at(loc)) ) {}
        
        Airplane(const Airplane& other) :
            map(other.map),
            location(other.location),
            owner(other.owner) {}

        // does action (takeoff or color move)
        // returns success
        bool do_action(Action act, bool main_board) {
            // move takeoff
            if(act.type == 'T') {
                if(main_board && DEBUG) {
                    cout << "Take-off to " << act.info << "! ";
                }
                return jump(act.info, main_board);
            }
            else if(act.type == 'C') {
                if(main_board && DEBUG) {
                    cout << act.info << " from " << location->name << ". ";
                }
                return move_route(*(new Color(act.info)), main_board);
            }
            else {
                throw std::domain_error("invalid action type");
                return false;
            }
        }

        // checks whether plane should be bumping
        // returns whether bump occured
        bool check_bump(bool main_board) {
            bool did_the_thing = false;
            // if there are multiple planes here
            vector<Airplane*> occ = get_occupancy_special(get_location_special(main_board), main_board);
            if(occ.size() > 1) {
                // for each plane
                for(int i = 0; i < occ.size(); i++) {
                    // if it is a different plane and we aren't at the start or end
                    if( occ[i] != this && !(get_location_special(main_board)->name == "START" || get_location_special(main_board)->name == "END") ) {
                        // with a different owner
                        if(occ[i]->owner != owner) {
                            occ[i]->bumped(main_board);
                            did_the_thing = true;
                            if(main_board && DEBUG) {
                                cout << "Bumped on " << location->name << "! ";
                            }
                        }
                        // if same owner, problem!
                        else {
                            throw std::domain_error("trying to bump ally plane");
                        }
                    }
                }
            }
            return did_the_thing;
        }

        // sync shadow location with real location
        void sync() {
            shadow_location = location;
        }

        // Accessors
        string get_loc_name(bool main_board) {
            return get_location_special(main_board)->name;
        }

        Airport* get_loc(bool main_board) {
            return get_location_special(main_board);
        }

        const Player* get_owner_ptr() {
            return owner;
        }

        void set_owner_ptr(Player* own) {
            owner = own;
        }

        void change_map(std::unordered_map<string, Airport> new_map) {
            map = new_map;
        }

};

#endif