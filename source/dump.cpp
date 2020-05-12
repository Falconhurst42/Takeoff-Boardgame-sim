#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <time.h>
#include <random>
#include <cctype>

#include <unordered_map>
#include <queue>
#include <utility>
#include <vector>
#include <string>

using std::vector, std::queue, std::pair, std::string, std::cout;

std::unordered_map<string, Airport> map;

// valid colors: red, orange, yellow, green, blue, purple
struct Color {
    char c;
    const vector<string> DEFUALT_COLORS = {"red", "orange", "yellow", "green", "blue", "purple"};

    Color(string col = "white") : 
        c ( std::toupper( col[0] ) ) {}

    bool operator==(Color other) {
        return c = other.c;
    }
};

// valid actions: take-off or color-move
struct Action {
    char type;      // 'T' for take-off or 'C' for color-move
    string info;    // take-off destination key or color-move color

    Action(char t, string i) : 
        type(t), 
        info(i) {}
};

struct Airport {
    string name;
    float lon, lat;
    // routes stored as <Airport*, Color>
    vector<pair<Airport*, Color>> children, parents;
    vector<Airplane*> occupants;
};

struct Option {
    int score, actions_used;
};

// Airplane Class
class Airplane {
    private:
        Game* game;
        Airport* location;
        const Player* owner;

        bool bumped() {
            location = &(map.at("START"));
        }

        // updates loc, returns whether bumped
        bool move(Airport* old, Airport* loc) {

            /*bool bumped = false;
            // update location
            location = loc;
            // update old occuapancy
            old->occupant = NULL;
            // check for bump (avoid bumping oneself on stationary jump)
            if(location->occupant != NULL && location->occupant != this) {
                location->occupant->bumped();
                bumped = true;
            }
            // update new occupancy
            location->occupant = this;

            return bumped;*/
        }

        // jumps plane to given destination (key), returns success of jump (false if dest does not exist in map)
        bool jump(string dest) {
            /*try {
                // avoid stationary jumps (will throw error if dest is invalid)
                if(location == &(map.at(dest))) {
                    // in stationary jump, nothing needs to be done
                    return true;
                }
                // else move
                move(location, &(map.at(dest)));
                return true;
            }
            // case that dest (or location) does not exist in map
            catch(const std::out_of_range& e) {
                return false;
            }*/
        }

        // moves plane along its child route with the given color
        // returns true if route existed and plane was moved
        bool move_route(Color c) {
            /*// find destination
            for (int i = 0; i < location->children.size(); i++) {
                if(location->children[i].second == c) {
                    // move
                    move(location, location->children[i].first);
                    // return success
                    return true;
                }
            }
            // return failure
            return false;*/
        }

    public:
        // basic constructor, defaults position to start
        Airplane(const Player* own = NULL, const string& loc = "START") : 
            owner(own), 
                location( &( map.at(loc) ) ) {}

        // does action (takeoff or color move)
        // returns success
        bool do_action(Action act) {
            // move takeoff
            if(act.type == 'T') {
                return jump(act.info);
            }
            else if(act.type == 'C') {
                return move_route(*(new Color(act.info)));
            }
            else {
                throw std::domain_error("invalid action type");
                return false;
            }
        }

        bool check_bump() {
            // if there are multiple planes here
            if(location->occupants.size() > 1) {
                // for each plane
                for(int i = 0; i < location->occupants.size(); i++) {
                    // if it is a different plane
                    if(location->occupants[i] != this) {
                        // with a different owner
                        if(location->occupants[i]->owner != owner) {
                            location->occupants[i]->bumped();
                        }
                        // if same owner, problem!
                        else {
                            throw std::domain_error("trying to bump ally plane");
                        }
                    }
                }
            }
        }

        // Accessors
        string get_loc_name() const {
            return location->name;
        }

        Airport* get_loc() const {
            return location;
        }

        const Player* get_owner_ptr() const {
            return owner;
        }

        void set_owner_ptr(Player* own) {
            owner = own;
        }
};

class Player {
    private:
        Game* game;
        vector<Airplane> planes;
        Color plane_color;

        // returns vector with Actions to be taken this turn
        vector<Action> roll() {
            int NUM_SIDES = 8;
            string sides[NUM_SIDES] = {"red", "orange", "yellow", "green", "blue", "purple", "wild", "take-off"};
            vector<string> rolls = {sides[rand()%NUM_SIDES], sides[rand()%NUM_SIDES]};
            if(rolls[0] == rolls[1]) {
                rolls.push_back(rolls[0]);
                rolls.push_back(rolls[0]);
            }
            vector<Action> actions;
            for(int i = 0; i < rolls.size(); i++) {
                // generate takeoff action
                if(rolls[i] == "take-off") {
                    actions.emplace_back('T', game->draw_takeoff());
                }
                // generate color action
                else {
                    actions.emplace_back('C', rolls[i]);
                }
            }
            return actions;
        }
    public:
        // if action.info = "wild", must change color before sending to move command
        
        Player(Game* g, vector<Airplane> p, Color c)  :
            game(g), 
            planes(p), 
            plane_color(c) {
                for(int i = 0; i < planes.size(); i++) {
                    planes[i].set_owner_ptr(this);
                }
            }

        bool take_turn() {
            vector<Action> actions = roll();
            // decide what to do with rolls
            //  simulate permutations
            //   two different rolls
            if(actions.size() == 2) {
                // 1ab, 1ba, 2ab, 2ba, 1a2b, 2a1b
            }
            //   four of the same roll
            else if(actions.size() == 4) {

            }
            // do rolls
            // bump
        }

};

class Game {
    private:
        std::unordered_map<string, Airport> map;
        queue<string> takeoff_pile;
        vector<Player> players;
        // vector<Airplane> planes;
        const int NUM_OF_PLAYERS, PLANES_PER_PLAYER, MAX_TURNS;
        int turn_location, turn_num;
    public:
        Game(int player_count = 4, int planes_per_player = 4, int max_turns = -1) : 
            NUM_OF_PLAYERS(player_count), 
            PLANES_PER_PLAYER(planes_per_player), 
            MAX_TURNS(max_turns) {
            srand(time(0));
            // initialize players
            for(int i = 0; i < player_count; i++) {
                // make planes vector of right size
                vector<Airplane> temp_planes(planes_per_player);
                // initialize player and add to vector
                players.emplace_back(this, temp_planes, (new Color())->DEFUALT_COLORS[i]);  // can't really make a static const array, so I have to do this for now
                /*for(int i = 0; i < temp_planes.size(); i++) {
                    planes.push_back(temp_planes[i]);
                }*/
            }
            // generate Airports and connections
                // end goal: pull airport data from .txt file and generate connections automatically
                // intermediate: pull airport data from .txt file, data includes connections
                // basic: manually initialize airports and connections

        }

        string draw_takeoff() {
            // move the one we're drawing to the bottom
            takeoff_pile.push(takeoff_pile.front());
            takeoff_pile.pop();
            // return the one on the bottom
            return takeoff_pile.back();
        }

}

int main() {
    
}