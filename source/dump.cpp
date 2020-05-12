#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <time.h>
#include <random>
#include <cctype>

#include <unordered_map>
#include <queue>
#include <set>
#include <utility>
#include <vector>
#include <string>

using std::vector, std::queue, std::set, std::pair, std::string, std::cout;

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

struct Airport {
    string name;
    float lat, lon;
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

        // if plane gets bumped, do this
        bool bumped() {
            location = &(game->map.at("START"));
            (game->map.at("START")).occupants.push_back(this);
        }

        // updates loc, returns nothing for now
        bool move(Airport* dest) {
            // remove this plane from the old location
            std::remove(location->occupants.begin(), location->occupants.end(), this);
            // add it to the new one
            dest->occupants.push_back(this);
            return true;            
        }

        // jumps plane to given destination (key), returns success of jump (false if dest does not exist in map)
        bool jump(string dest) {
            try {
                // avoid stationary jumps (will throw error if dest is invalid)
                if(location == &(game->map.at(dest))) {
                    // in stationary jump, nothing needs to be done
                    return true;
                }
                // else move
                move(&(game->map.at(dest)));
                return true;
            }
            // case that dest (or location) does not exist in map
            catch(const std::out_of_range& e) {
                return false;
            }
        }

        // moves plane along its child route with the given color
        // returns true if route existed and plane was moved
        bool move_route(Color c) {
            // find destination
            for(int i = 0; i < location->children.size(); i++) {
                // if the color of the rout matches
                if(location->children[i].second == c) {
                    // move to the destination of the route
                    move(location->children[i].first);
                    // return success
                    return true;
                }
            }
            // return failure if you make it here
            return false;
        }

        Airport* identify_dest(Color c) {
            for (int i = 0; i < location->children.size(); i++) {
                // if the color of the rout matches
                if(location->children[i].second == c) {
                    // return dest
                    return location->children[i].first;
                }
            }
            // return failure if you make it here
            return NULL;
        }

    public:
        // basic constructor, defaults position to start
        Airplane(const Player* own = NULL, const string& loc = "START") : 
            owner(own), 
                location( &(game->map.at(loc) ) ) {}

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

        // checks whether plane should be bumping
        // returns whether bump occured
        bool check_bump() {
            bool did_the_thing = false;
            // if there are multiple planes here
            if(location->occupants.size() > 1) {
                // for each plane
                for(int i = 0; i < location->occupants.size(); i++) {
                    // if it is a different plane
                    if(location->occupants[i] != this) {
                        // with a different owner
                        if(location->occupants[i]->owner != owner) {
                            location->occupants[i]->bumped();
                            did_the_thing = true;
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
        vector<Action> roll(int dice = 2) {
            int NUM_SIDES = 8;
            string sides[NUM_SIDES] = {"red", "orange", "yellow", "green", "blue", "purple", "wild", "take-off"};

            // get string versions
            vector<string> rolls(dice);
            for(int i = 0; i < dice; i++) {
                rolls[i] = sides[rand()%NUM_SIDES];
            }

            // handle doubles (only works for dice = 2)
            if(rolls[0] == rolls[1]) {
                rolls.push_back(rolls[0]);
                rolls.push_back(rolls[0]);
            }

            // convert from string to action
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

            // return vector of actions
            return actions;
        }

        // default plane scoring, just goes by latitude of location
        float get_plane_score(Airplane a) {
            return a.get_loc()->lat;
        }

        // given a set of actions, returns best movement plan
        vector<pair<Airplane, Action>> decide_moves(vector<Action> actions) {
            // decide what to do with rolls
            // find every permutation of planes with length equal to number of actions (including repeated planes)
            vector<vector<Airplane>> plane_permutations;
            get_all_permutations(plane_permutations, planes.begin(), planes.end(), actions.size());

            // find every permutation of actions with length equal to number of actions (eliminate repeats by permutating on unique set)
            set<Action> unique_actions;
            for(int i = 0; i < actions.size(); i++) {
                unique_actions.insert(actions[i]);
            }
            vector<vector<Action>> action_permutations;
            get_all_permutations(action_permutations, unique_actions.begin(), unique_actions.end(), actions.size());

            // tracks best combination so far
            int best_actions_used = 0;  // highest priority
            float best_score = 0;       // secondary priority
            pair<int, int> best_combo_indexes;

            // for each permutation of planes
            for(int i = 0; i < plane_permutations.size(); i++ ) {
                // for each permutation of actions applied to that order of planes
                for(int j = 0; j < action_permutations.size; j++) {
                    // simulate that particular order of moves
                    // record score
                    // update best if necessary
                }
            }
        }

        // referenced: "Print all possible strings of length k that can be formed from a set of n characters" from geeksforgeeks.com. https://www.geeksforgeeks.org/print-all-combinations-of-given-length/
        //   while developing this algorithm
        // and referenced zneak's answer from "Template container iterators" on StackOverflow https://stackoverflow.com/questions/30018517/template-container-iterators
        //   for how to template with my iterators
        template<typename It>
        void get_all_permutations(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0)) {
            // base case: length == 0
            // push prefix into output
            if(length == 0) {
                output.push_back(prefix);
                return;
            }

            // for every item in source
            It temp = start;
            while(temp != end) {
                // make a new prefix which is the old prefix plus the new item
                vector<typename It::value_type> new_prefix(prefix);
                new_prefix.push_back(*temp);
                // get all permutations with that prefix one size smaller
                get_all_permutations(output, start, end, length-1, new_prefix);
                temp++;
            }
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
            // roll to get actions
            vector<Action> actions = roll();

            // do take-offs on worst planes

            // decide how to move given those actions
            vector<pair<Airplane, Action>> move_plan = decide_moves(actions);

            // do moves
            for(int i = 0; i < move_plan.size(); i++) {
                move_plan[i].first.do_action(move_plan[i].second);
            }

            // check for bumps
            for(int i = 0; i < planes.size(); i++) {
                planes[i].check_bump();
            }
        }

};

class Game {
    public:
    //private:
        std::unordered_map<string, Airport> map;
        queue<string> takeoff_pile;
        vector<Player> players;
        // vector<Airplane> planes;
        const int NUM_OF_PLAYERS, PLANES_PER_PLAYER, MAX_TURNS;
        int turn_location, turn_num;
    //public:
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