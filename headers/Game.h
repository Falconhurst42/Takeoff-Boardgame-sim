#ifndef GAME_H_
#define GAME_H_

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

bool DEBUG(true);

class Game {
    const vector<string> DEFUALT_COLORS = {"red", "orange", "yellow", "green", "blue", "purple"};

    public:
        class Player;
        class Airplane;
        struct Airport;

    /*----------------------------------------------\\
    //                                              \\
    //                Color Struct                  \\
    //                                              \\
    //----------------------------------------------*/
        struct Color {
            char c;

            // c char is assigned to capitalized first char of given string
            Color(string col = "white") : 
                c ( std::toupper( col[0] ) ) {
                    if(col == "wild") {
                        c = '?';
                    }
                }

            // if chars match
            bool operator==(const Color& other) const {
                return c == other.c;
            }

            // assign chars to match
            void operator=(Color other) {
                c = other.c;
            }
        };
    
    /*----------------------------------------------\\
    //                                              \\
    //                Action Struct                 \\
    //                                              \\
    //----------------------------------------------*/
        // valid actions: take-off or color-move
        struct Action {
            char type;
            Airport* dest;
            Color color;

            // blank constructor
            Action() :
                type(' ') {};

            // take-off constructor
            Action(Airport* d) :
                type('T'),
                dest(d) {}

            // color-move constructor
            Action(Color c) :
                type('C'),
                color(c) {}
            
            // if type and info match
            bool operator==(const Action& other) const {
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

            // match type and info
            void operator=(const Action& other) {
                type = other.type;
                if(type == 'T') {
                    dest = other.dest;
                }
                else {
                    color = other.color;
                }
            }
        };

    /*----------------------------------------------\\
    //                                              \\
    //               Airport Struct                 \\
    //                                              \\
    //----------------------------------------------*/
        
        struct Airport {
            string name;        // used to check start and end, key and display name
            float lat, lon;
            // routes stored as <Airport*, Color>
            vector<pair<Airport*, Color>> children, parents;
            vector<Airplane*> occupants;
            vector<Airplane*> shadow_occupants;

            // all-purpose consturctor
            Airport(string n = "", float lati = 0, float longi = 0) : 
                name(n),
                lat(lati),
                lon(longi),
                occupants(0, NULL),
                shadow_occupants(0, NULL) {}

            // creates connection of given color between given aiports, enforcing right to left motion
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

    /*----------------------------------------------\\
    //                                              \\
    //               Airplane Class                 \\
    //                                              \\
    //----------------------------------------------*/
        
        class Airplane {
            private:
                Airport* location;
                Airport* shadow_location;
                Airport* start;
                Airport* end;
                const Player* owner;

                // airpost occupancy accessor function which accounts for main-shadow distinction
                vector<Airplane*>& get_occupancy_special(Airport* loc, bool main_board)  {
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
                    get_location_special(main_board) = start;
                    get_occupancy_special(start, main_board).push_back(this);
                }

                // updates loc, returns nothing for now
                bool move(Airport* dest, bool main_board) {
                    if(main_board && DEBUG && location != NULL) {
                        cout << "Moved from " << location->name << " to " << dest->name << ". ";
                    }
                    // remove this plane from the old location
                    if(get_location_special(main_board) != NULL) {
                        for(int i = 0; i < get_occupancy_special(get_location_special(main_board), main_board).size(); i++) {
                            if(get_occupancy_special(get_location_special(main_board), main_board)[i] == this) {
                                get_occupancy_special(get_location_special(main_board), main_board).erase(get_occupancy_special(get_location_special(main_board), main_board).begin()+i);
                            }
                        }
                    }
                    // add it to the new one
                    get_occupancy_special(dest, main_board).push_back(this);
                    // update location
                    get_location_special(main_board) = dest;
                    return true;            
                }

                // jumps plane to given destination (key), returns true
                bool jump(Airport* dest, bool main_board) {
                    // avoid stationary jumps
                    if(get_location_special(main_board) == dest) {
                        // in stationary jump, nothing needs to be done
                        return true;
                    }
                    // else move
                    move(dest, main_board);
                    return true;
                }

                // moves plane along its child route with the given color
                // returns true if route existed and plane was moved
                bool move_route(Color c, bool main_board) {
                    // find destination
                    Airport* dest = identify_dest(c, main_board);
                    if(dest == NULL) {
                        return false;
                    }
                    move(dest, main_board);
                    return true;
                }

                // trace color route to destination (if it exists)
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
                Airplane(Airport* loc = NULL, Airport* st = NULL, Airport* en = NULL, const Player* own = NULL) : 
                    owner(own), 
                    location(loc),
                    start(st),
                    end(en) {}
                
                // copy constructor
                Airplane(const Airplane& other) :
                    location(other.location),
                    start(other.start),
                    end(other.end),
                    owner(other.owner) {}

                // does action (takeoff or color move)
                // returns success
                bool do_action(Action act, bool main_board) {
                    // move takeoff
                    if(act.type == 'T') {
                        if(main_board && DEBUG) {
                            cout << "Take-off to " << act.dest->name << "! ";
                        }
                        return jump(act.dest, main_board);
                    }
                    else if(act.type == 'C') {
                        if(main_board && DEBUG) {
                            cout << act.color.c << " from " << location->name << ". ";
                        }
                        return move_route(act.color, main_board);
                    }
                    else {
                        throw std::domain_error("invalid action type");
                        return false;
                    }
                }

                // checks whether plane should be bumping
                // returns whether bump occured
                // throws std::domain_error if bumping own plane
                bool check_bump(bool main_board) {
                    bool did_the_thing = false;
                    // if there are multiple planes here
                    vector<Airplane*> occ = get_occupancy_special(get_location_special(main_board), main_board);
                    if(occ.size() > 1) {
                        // for each plane
                        for(int i = 0; i < occ.size(); i++) {
                            // if we aren't at a the start or end or bumping ourself
                            if( !(occ[i] == this || get_location_special(main_board) == start || get_location_special(main_board) == end ) ) {
                                // if the plane has a different owner
                                if(occ[i]->owner != owner) {
                                    occ[i]->bumped(main_board);
                                    did_the_thing = true;
                                    if(main_board) {
                                        cout << "Bumped on " << location->name << "! ";
                                    }
                                }
                                // if same owner, problem!
                                else {
                                    if(main_board) {
                                        int breaking = 0;
                                    }
                                    if(occ[i] == this) {
                                        cout << "I AM A DUMMY";
                                    }
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
        };
    
    /*----------------------------------------------\\
    //                                              \\
    //                Player Class                  \\
    //                                              \\
    //----------------------------------------------*/
        
        class Player {
            private:
                Game* game;
                vector<Airplane> planes;
                Color plane_color;
                const float BUMP_VALUE = 20;
                bool finished;

                const vector<string> DEFUALT_COLORS = {"red", "orange", "yellow", "green", "blue", "purple"};

                // referenced: "Print all possible strings of length k that can be formed from a set of n characters" from geeksforgeeks.com. https://www.geeksforgeeks.org/print-all-combinations-of-given-length/
                //   while developing this algorithm
                // and referenced zneak's answer from "Template container iterators" on StackOverflow https://stackoverflow.com/questions/30018517/template-container-iterators
                //   for how to template with my iterators
                template<typename It>
                void get_all_permutations(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0))  {
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

                template<typename It>
                void get_all_permutations_no_repeats(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0))  {
                    
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
                        // move used one to start
                        std::swap(*temp, *start);
                        // get all permutations with that prefix one size smaller
                        get_all_permutations_no_repeats(output, start+1, end, length-1, new_prefix);
                        temp++;
                    }
                }

                void expand_wild_permutation(vector<vector<Action*>>& a_p, int ind) {
                    // copy
                    vector<Action*> expandee = a_p[ind];

                    // count wilds and leave markers
                    int wild_count(0);
                    for(int i = 0; i < expandee.size(); i++) {
                        if(expandee[i]->color.c == '?') {
                            wild_count++;
                            expandee[i] = NULL;
                        }
                    }

                    // generate options for a wild
                    vector<Action *> wild_options;
                    for(int i = 0; i < DEFUALT_COLORS.size(); i++) {
                        wild_options.push_back(new Action(Color(DEFUALT_COLORS[i])));
                    }
                    // get wild permutations
                    vector<vector<Action*>> wild_perms;
                    get_all_permutations(wild_perms, wild_options.begin(), wild_options.end(), wild_count);

                    // append wild expansions
                    // for wild_perm
                    for(int i = 0; i < wild_perms.size(); i++) {
                        // create copy of expandee
                        vector<Action*> expandee_copy(expandee);
                        int w_p_index = 0;
                        // for pointers in copy of expandee
                        for(int j = 0; j < expandee_copy.size(); j++) {
                            if(expandee_copy[j] == NULL) {
                                expandee_copy[j] = wild_perms[i][w_p_index];
                                w_p_index++;
                            }
                        }
                        // push back filled expansion
                        a_p.push_back(expandee_copy);
                    }      
                }

                // default plane scoring, just goes by longitude of location
                float get_plane_score(Airplane* a, bool main_board)  {
                    // really good if at end
                    if(a->get_loc_name(main_board) == "END") {
                        return 250;
                    }
                    return -(a->get_loc(main_board)->lon);
                }

                // given a set of actions, returns best movement plan
                void decide_moves(vector<pair<Airplane*, Action*>>& move_plan, vector<Action> actions, vector<Player>& fellow_gamers)  {
                    // decide what to do with rolls
                    // find every permutation of planes with length equal to number of actions (including repeated planes)
                    // permutations are tracked with pointers to ensure that operations are performed on base planes, not copies

                    vector<vector<Airplane*>> plane_permutations;
                    vector<Airplane*> plane_pointers(planes.size());
                    for(int i = 0; i < planes.size(); i++) {
                        plane_pointers[i] = &(planes[i]);
                    }
                    get_all_permutations(plane_permutations, plane_pointers.begin(), plane_pointers.end(), actions.size());

                    // find every permutation of actions with length equal to number of actions (eliminate repeats by permutating on unique set)
                    vector<vector<Action*>> action_permutations;
                    vector<Action*> action_pointers(actions.size());
                    for(int i = 0; i < actions.size(); i++) {
                        action_pointers[i] = &(actions[i]);
                    }
                    get_all_permutations_no_repeats(action_permutations, action_pointers.begin(), action_pointers.end(), actions.size());
                    // remove duplicate move permutations
                    // for each permutation
                    for(int i = 1; i < action_permutations.size(); i++) {
                        // compare to previous permutation
                        // for each action in the permutations
                        bool sameish(true);
                        for(int j = 0; sameish && j < action_permutations[i].size(); j++) {
                            // if they don't match, note it
                            if( !( *(action_permutations[i][j]) == *(action_permutations[i-1][j]) ) ) {
                                sameish = false;
                            }
                        }
                        // if they matched, erase the previous one and don't advance
                        if(sameish) {
                            action_permutations.erase(action_permutations.begin()+i-1);
                            i--;
                        }
                    }

                    // expand wilds
                    // for each permutation
                    for(int i = 0; i < action_permutations.size(); i++) {
                        // for each action in the permutation
                        for(int j = 0; j < action_permutations[i].size(); j++) {
                            // if the action is wild
                            if(action_permutations[i][j]->type == 'C' && action_permutations[i][j]->color.c == '?') {
                                expand_wild_permutation(action_permutations, i);
                                action_permutations.erase(action_permutations.begin()+i);
                                i--;
                                break;
                            }
                        }
                    }

                    // tracks best combination so far
                    // from the combinations that use the maximum number of actions, we will take the one with the best score
                    int best_actions_used = -1;  // highest priority
                    float best_score = -10000;  // secondary priority
                    pair<int, int> best_combo_indexes;

                    // for each permutation of planes
                    for(int i = 0; i < plane_permutations.size(); i++ ) {
                        // for each permutation of actions applied to that order of planes
                        for(int j = 0; j < action_permutations.size(); j++) {
                            // simulate that particular set of moves
                            // this will all done with "main_board" flagged as false
                            // which means we are moving planes in a shadow version of the board
                            // which will not effect the main board

                            // sync shadow board to main board
                            game->sync_boards();

                            int actions_used(0);

                            // do actions on imaginary planes
                            for(int t = 0; t < actions.size(); t++) {
                                // track successful moves
                                if(plane_permutations[i][t]->do_action(*(action_permutations[j][t]), false)) {
                                    actions_used++;
                                }
                            }

                            int score = 0;

                            // check for ending 
                            bool all_done(true);
                            for(int p = 0; p < planes.size(); p++) {
                                if(planes[p].get_loc_name(false) != "END") {
                                    all_done = false;
                                    break;
                                }
                            }
                            // if this move would reach the end
                            if(all_done) {
                                best_score = score;
                                best_actions_used = actions_used;
                                best_combo_indexes = std::make_pair(i, j);
                                goto all_done;
                            } 

                            // check bumps and add points
                            for(int b = 0; b < planes.size(); b++) {
                                // watch out for self-bumps, thrown as error
                                try {
                                    if(planes[b].check_bump(false)) {
                                        score += BUMP_VALUE;
                                    }
                                }
                                catch (const std::domain_error& self_bump) {
                                    // in case of self-bump, don't even consider this option
                                    actions_used = -2;
                                }
                            }

                            // add individual plane scores
                            for(int p = 0; p < planes.size(); p++) {
                                score += get_plane_score(&(planes[p]), false);
                            }

                            // update best score
                            if(actions_used > best_actions_used) {
                                best_score = score;
                                best_actions_used = actions_used;
                                best_combo_indexes = std::make_pair(i, j);
                            } 
                            else if(actions_used == best_actions_used && score > best_score) {
                                best_score = score;
                                best_actions_used = actions_used;
                                best_combo_indexes = std::make_pair(i, j);
                            }
                        }
                    }

                    // dereference best moves into move_plan
                    for(int i = 0; i < actions.size(); i++) {
                        move_plan[i] = std::make_pair( (plane_permutations[best_combo_indexes.first][i]), 
                                                        (new Action(action_permutations[best_combo_indexes.second][i]->color)) );   // memory allocation is weird
                    }
                    return;

                    // I'm using a goto, I know it's evil but I gotta break out of the doulbe for loop and this is really the most efficient way
                    all_done:
                        for(int i = 0; i < actions.size(); i++) {
                            move_plan[i] = std::make_pair( (plane_permutations[best_combo_indexes.first][i]), 
                                                            (new Action(action_permutations[best_combo_indexes.second][i]->color)) );   // memory allocation is weird
                        }
                        return;

                }
            
            public:        
                // default constructor
                Player(Game* g, vector<Airplane> p, Color c) :
                    game(g), 
                    planes(p), 
                    plane_color(c),
                    finished(false) {
                        for(int i = 0; i < planes.size(); i++) {
                            planes[i].set_owner_ptr(this);
                        }
                    }

                // takes moves, decides how to do them, and does them
                void take_turn(vector<Action> actions, vector<Player>& fellow_gamers) {
                    // do take-offs on worst planes
                    for(int i = 0; i < actions.size(); i++) {
                        // if an action is a take-off
                        if(actions[i].type == 'T') {
                            // if the location of the take-off matches another ally plane, just delete it
                            bool use_it(true);
                            for(int p = 0; p < planes.size(); p++) {
                                if(actions[i].dest == planes[p].get_loc(true)) {
                                    use_it = false;
                                    break;
                                }
                            }
                            if(use_it) {
                                // find the worst plane
                                Airplane* worst_plane = &(planes[0]);
                                float worst_score = get_plane_score(worst_plane, true);
                                for(int j = 1; j < planes.size(); j++) {
                                    float score = get_plane_score(&(planes[j]), true);
                                    if(score < worst_score) {
                                        worst_plane = &(planes[j]);
                                        worst_score = score;
                                    }
                                }
                                // and use the take-off on it
                                worst_plane->do_action(actions[i], true);
                            }

                            // delete used take-off
                            for(int j = i+1; j < actions.size(); j++) {
                                actions[j-1] = actions[j];
                            }
                            actions.pop_back();
                        }
                    }

                    // if there are still moves left
                    if(actions.size() > 0) {
                        // decide how to move given those actions
                        vector<pair<Airplane*, Action*>> move_plan(actions.size());
                        decide_moves(move_plan, actions, fellow_gamers);

                        // do moves
                        for(int i = 0; i < move_plan.size(); i++) {
                            move_plan[i].first->do_action(*(move_plan[i].second), true);
                            // clean up dynamic declarations
                            delete move_plan[i].second;
                        }
                    }

                    // check for planes at end
                    for(int i = 0; i < planes.size(); i++) {
                        // delete planes at end
                        if(planes[i].get_loc_name(true) == "END") {
                            planes.erase(planes.begin()+i);
                            if(planes.size() == 0) {
                                finished = true;
                                if(DEBUG)
                                    cout << "A Player has finished!\n";
                            }
                            i--;
                        }
                    }
                    // check for bumps
                    for(int i = 0; i < planes.size(); i++) {
                        planes[i].check_bump(true);
                    }

                    if(DEBUG)
                        cout << "\n\n";
                }

                // sync shadow location with real location for all planes
                void sync_planes() {
                    for(int i = 0; i < planes.size(); i++) {
                        planes[i].sync();
                    }
                }

                void gather_planes(Airport* start) {
                    for(int i = 0; i < planes.size(); i++) {
                        DEBUG = false;
                        planes[i].do_action(Action(start), true);
                        DEBUG = true;
                    }
                }

                bool is_done() const {
                    return finished;
                }
        };


    /*----------------------------------------------\\
    //                                              \\
    //                 Game Class                   \\
    //                                              \\
    //----------------------------------------------*/

    //private:
        std::unordered_map<string, Airport> map;
        queue<string> takeoff_pile;
        vector<Player> players;
        const int NUM_OF_PLAYERS, PLANES_PER_PLAYER, MAX_TURNS;
        int turn_location, turn_num;
    //public:
        Game(int player_count = 4, int planes_per_player = 4, int max_turns = 100) : 
            NUM_OF_PLAYERS(player_count), 
            PLANES_PER_PLAYER(planes_per_player), 
            MAX_TURNS(max_turns) {
                turn_location = turn_num = 0;
            }

        void run_game() {
            new_game();
            loop();
            cout << "\n\n";
        }

        void new_game() {
            // seed randomness
                int seed = time(0);
                //int seed = 1589428500;
                if(DEBUG)
                    cout << seed << '\n';
                srand(seed);

                // generate Airports and connections
                    // end goal: pull airport data from .txt file and generate connections automatically
                    // intermediate: pull airport data from .txt file, data includes connections
                    // basic: manually initialize airports and connections
                // make airports
                map.clear();
                    map.insert(std::make_pair("START", Airport("START", 21.318611, 202.0775)));
                    map.insert(std::make_pair("Atlanta", Airport("Atlanta", 33.636667, -84.428056)));
                    map.insert(std::make_pair("Los Angeles", Airport("Los Angeles", 33.9425, -118.408056)));
                    map.insert(std::make_pair("New York", Airport("New York", 40.639722, -73.778889)));
                    map.insert(std::make_pair("Beijing", Airport("Beijing", 40.0725, 116.5975)));
                    map.insert(std::make_pair("Dubai", Airport("Dubai", 25.252778, 55.364444)));
                    map.insert(std::make_pair("London", Airport("London", 51.4775, -0.461389)));
                    map.insert(std::make_pair("Sydney", Airport("Sydney", -33.946111, 151.177222)));
                    map.insert(std::make_pair("Kemton Park", Airport("Kemton Park", -26.133333, 28.25)));
                    map.insert(std::make_pair("END", Airport("END", 21.318611, -157.9225)));

                // make connections
                    Airport::connect(&(map.at("START")), &(map.at("Beijing")), Color("red"));
                    Airport::connect(&(map.at("START")), &(map.at("Sydney")), Color("green"));
                    Airport::connect(&(map.at("Beijing")), &(map.at("Sydney")), Color("orange"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("Sydney")), Color("yellow"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("Beijing")), Color("yellow"));
                    Airport::connect(&(map.at("Kemton Park")), &(map.at("Beijing")), Color("purple"));
                    Airport::connect(&(map.at("Kemton Park")), &(map.at("Sydney")), Color("yellow"));
                    Airport::connect(&(map.at("London")), &(map.at("Beijing")), Color("red"));
                    Airport::connect(&(map.at("London")), &(map.at("Dubai")), Color("orange"));
                    Airport::connect(&(map.at("London")), &(map.at("Kemton Park")), Color("green"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("Kemton Park")), Color("purple"));
                    Airport::connect(&(map.at("London")), &(map.at("New York")), Color("red"));
                    Airport::connect(&(map.at("London")), &(map.at("Atlanta")), Color("green"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("Atlanta")), Color("red"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("New York")), Color("yellow"));
                    Airport::connect(&(map.at("Kemton Park")), &(map.at("Atlanta")), Color("red"));
                    Airport::connect(&(map.at("Los Angeles")), &(map.at("Atlanta")), Color("orange"));
                    Airport::connect(&(map.at("New York")), &(map.at("Atlanta")), Color("blue"));
                    Airport::connect(&(map.at("END")), &(map.at("Atlanta")), Color("purple"));
                    Airport::connect(&(map.at("New York")), &(map.at("Los Angeles")), Color("yellow"));
                    Airport::connect(&(map.at("New York")), &(map.at("END")), Color("red"));
                    Airport::connect(&(map.at("END")), &(map.at("Los Angeles")), Color("green"));

                // initialize players
                players.clear();
                for(int i = 0; i < NUM_OF_PLAYERS; i++) {
                    // make planes vector of right size
                    vector<Airplane> temp_planes(PLANES_PER_PLAYER, Airplane(&(map.at("START")), &(map.at("START")), &(map.at("END"))));
                    // initialize player and add to vector
                    players.emplace_back(this, temp_planes, DEFUALT_COLORS[i]);
                }

                // move all planes to start
                for(int y = 0; y < NUM_OF_PLAYERS; y++) {
                    players[y].gather_planes(&(map.at("START")));
                }

                // initialize take-off pile
                while(!takeoff_pile.empty()) {
                    takeoff_pile.pop();
                }
                for(auto it = map.begin(); it != map.end(); it++) {
                    // don't push start or end
                    if(!((*it).first == "START" || (*it).first == "END")) {
                        takeoff_pile.push((*it).first);
                    }
                }
        }

        // game loop
        void loop() {
            bool someone_left(true);
            // while ther are turn and players left
            for(turn_num = 0; turn_num != MAX_TURNS && someone_left; turn_num++) {
                someone_left = false;
                // have each player take their turn
                for(turn_location = 0; turn_location < players.size(); turn_location++) {
                    if(!players[turn_location].is_done()) {
                        players[turn_location].take_turn(roll(), players);
                        if(!players[turn_location].is_done())
                            someone_left = true;
                    }
                }
            }
            cout << "All players have finished!";
        }

        // simulate drawing a take-off card
        Airport* draw_takeoff() {
            // move the one we're drawing to the bottom
            takeoff_pile.push(takeoff_pile.front());
            takeoff_pile.pop();
            // return the one on the bottom
            return &(map.at(takeoff_pile.back()));
        }

        // sync all shadow occupancies with regular occupancies and all shadow locations with regular locations
        void sync_boards() {
            for(auto it = map.begin(); it != map.end(); it++) {
                (*it).second.shadow_occupants = (*it).second.occupants;
            }
            for(int i = 0; i < players.size(); i++) {
                players[i].sync_planes();
            }
        }

        // returns vector with Actions to be taken this turn
        vector<Action> roll(int dice = 2) {
            vector<string> sides(DEFUALT_COLORS); // = {"red", "orange", "yellow", "green", "blue", "purple", "wild", "take-off"};
            sides.push_back("wild");
            sides.push_back("take-off");
            const int NUM_SIDES = sides.size();

            // get string versions
            vector<string> rolls(dice);
            for(int i = 0; i < dice; i++) {
                rolls[i] = sides[rand()%NUM_SIDES];
            }

            // handle doubles                                               FIX: (ONLY WORKS FOR dice = 2)
            if(rolls[0] == rolls[1]) {
                rolls.push_back(rolls[0]);
                rolls.push_back(rolls[0]);
            }

            // convert from string to action
            vector<Action> actions;
            for(int i = 0; i < rolls.size(); i++) {
                // generate takeoff action
                if(rolls[i] == "take-off") {
                    actions.emplace_back(draw_takeoff());
                }
                // generate color action
                else {
                    actions.emplace_back(Color(rolls[i]));
                }
            }

            if(DEBUG) {
                cout << "[ "; 
                for(int i = 0; i < actions.size(); i++) {
                    string out;
                    if(actions[i].type == 'T')
                        out = actions[i].dest->name;
                    else if(actions[i].type == 'C')
                        out = actions[i].color.c;
                    cout << "(" << actions[i].type << ", " << out << "), ";
                }
                cout << " ]  "; 
            }

            // return vector of actions
            return actions;
        }
};

#endif