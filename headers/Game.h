// Ethan Worth
// 5/14/2020
// Data Structures Final Project

// On file structures:
//   This header file contains all of the code for running a Take-off simulation game
//   I have tried splitting it into multiple files or even just into '.h' and 'cpp' but it will not compile
//   This is due to the overall interconnectedness of the program, it won't compile unless everything is in the same place
//   All of the code is contained within the Game class, even the other classes
//   This is philosophically due to the fact that every class is designed to be used only within a Game object
//   This is practially because it wouldn't compile otherwise
//
/* class Game {
    struct Color {}
    struct Action {}
    struct Airport {}
    class Airplane {}
    class Player {}
    // then the actual code for the game class
}*/

// This file contains all the code necessary to run a simulate game of Takeoff and gather the benchmarking data

#ifndef GAME_H_
#define GAME_H_

#include <iostream>
#include <fstream>
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

// mode variables, set through game initializer but stored globally for convenience
    // enables debug mode which has extra output trackng individual moves (significantly slows processing, don't use for large tests)
    bool DEBUG(true);

    // enables altered bumping. false by default 
    // If a player has all their planes over the prime meridian and one of them gets bumped, they will draw an eastern-hemisphere takeoff for that plane instead of sending it back to the start
    // has little effect on gameplay, no altered bumps have been recorded
    bool PM_TAKEOFF_BUMP(false);

    // enables wilds. true by default 
    // (disabling wilds significantly speeds up simulation by eliminating the lengthy double-wild simulations)
    // benchmarking: wilds noticably decrease game length and increase bumping
    bool WILDS(true);
    // enables takeoffs. true by default
    // (has little effect on simulation speed or benchmarks)
    bool TAKEOFFS(true);

    // FUTURE: make main vs. shadow board a global flag instead of function parameter

class Game {
    // stores the colors that are used by the game for players and connections
    const vector<string> DEFUALT_COLORS = {"red", "orange", "yellow", "green", "blue", "purple"};

    public:
        // I included these forward delcarations (not these exact ones, more like their ancestors) after 
        // referencing kilojoules' answer to " “X does not name a type” error in C++ " (https://stackoverflow.com/questions/2133250/x-does-not-name-a-type-error-in-c/2133260) 
        // when troubleshooting compile errors
        class Player;
        class Airplane;
        struct Airport;

    /*----------------------------------------------\\
    //                                              \\
    //                Color Struct                  \\
    //                                              \\
    //----------------------------------------------*/
    
    // Color struct, denotes a color (or a wild) as a char
    // Future: could hold rgb values of color as well
        struct Color {
            //char denoting the first letter of the color ('R' for red) or a '?' for a wild
            char c;

            // constructor
            // Takes: (string) color = "white"
            // c char is assigned to capitalized first char of given string
            Color(string col = "white") : 
            // referened http://www.cplusplus.com/reference/cctype/tolower/ (I know that's not quite the same function, but I switched it later)
                c ( std::toupper( col[0] ) ) {
                    if(col == "wild") {
                        c = '?';
                    }
                }

            // comparison operator, compares chars
            // Takes: (const Color&) other
            // Returns: (bool) c == other.c;
            bool operator==(const Color& other) const {
                return c == other.c;
            }

            // assignment operator (c = other.c)
            // Takes: (const Color&) other
            // Returns: (const Color&) *this
            const Color& operator=(const Color& other) {
                c = other.c;
                return *this;
            }
        };
    
    /*----------------------------------------------\\
    //                                              \\
    //                Action Struct                 \\
    //                                              \\
    //----------------------------------------------*/
        // Action structure, denotes an action an Airplane can take
        // Color-move Actions: (char) type == 'C', 
        //                     (Color) color == {color to be moved on, or wild}
        // Take-off Actions: (char) type == 'T', 
        //                   (Airport*) dest == {destination}
        // Future: Color-move and Take-off should be derived classes?
        //         Could an Action object have a function that takes an airplane and calls the proper movement function with the proper arguments? (would also need polymorphism?)
        struct Action {
            char type;      // 'C' for color move, 'T' for takeoff
            Airport* dest;  // destination of takeoff
            Color color;    // Color to be moved on for color move (can be wild, but the wild should be replaced before sending this to an Airplane)

            // blank constructor
            // type(' ')
            Action() :
                type(' ') {};

            // take-off constructor
            // Takes: Airport* d
            // type('T')
            // dest(d)
            Action(Airport* d) :
                type('T'),
                dest(d) {}

            // color-move constructor
            // Takes: Color c
            // type('C')
            // color(c)
            Action(Color c) :
                type('C'),
                color(c) {}
            
            // comparison operator
            // Takes  (const Action&) other
            // Returns: (bool) (type == other.type == 'T' && dest == other.dest) || (type == other.type == 'C' && color == other.color)
            //          (if types and respective data match)
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

            // assignment operator
            // Takes  (const Action&) other
            //  type = other.type
            //  {respective_data = other.respective_data}
            // Returns: (const Action&) this*
            const Action& operator=(const Action& other) {
                type = other.type;
                if(type == 'T') {
                    dest = other.dest;
                }
                else {
                    color = other.color;
                }
                return *this;
            }
        };

    /*----------------------------------------------\\
    //                                              \\
    //               Airport Struct                 \\
    //                                              \\
    //----------------------------------------------*/
        
    // Airport struct, represents a space on the board and stores its connections to other Airports/
    // Future: include "country capital" flag, takeoffs would only be generated for country capitals
        struct Airport {
            // key for map hash, name if displayed
            // the same as the IATA abbreviation for the airport
            // start always has name "START", end always has name "END" (although that does mean I can't use Vance Airforce Base elsewhere on the map...)
            string name;

            // longitude and latitude of airport in real life
            // the negative of longitude is used as a rudimentary measure of progress, with the START having very positive longitude and END having a very negative longitude
            float lat, lon;

            // vectors storing outgoing (child) and incoming (parent) routes
            // routes stored as pair<(Airport*) ptr to other end of route, (Color) color of route>
            vector<pair<Airport*, Color>> children, parents;

            // vectors storing the spaces current occupants on the main board
            // only START and END may have multiple occupants at the *end* of a turn
            vector<Airplane*> occupants;
            // vectors storing the spaces current occupants on the "shadow" simulation board
            // the shadow_occupants of every airport are synced to the real occupants at the start of every simulation
            vector<Airplane*> shadow_occupants;

            // all-purpose constructor
            // Takes: (string) n = ""
            //        (float) lati = 0
            //        (float) longi = 0
            // Does: name(n), lat(lati), lon(longi)
            //       occupants(0, NULL), shadow_occupants(0, NULL)
            Airport(string n = "", float lati = 0, float longi = 0) : 
                name(n),
                lat(lati),
                lon(longi),
                occupants(0, NULL),
                shadow_occupants(0, NULL) {}

            // creates connection of given color between given aiports, enforcing right to left motion
            // Takes: (Airport*) parent
            //        (Airport*) child
            //        (Color) c
            // Note: before the connection is made, the function makes sure that the parent pointer is east of (before) the child pointer
            //       if the pointers are in the wrong order, it swaps them before creating the connection
            static void connect(Airport* parent, Airport* child, Color c) {
                // enforce right to left motion
                if(child->lon > parent->lon) {
                    std::swap(parent, child);
                }

                // add connections ot the airport's vectors
                parent->children.push_back(std::make_pair(child, c));
                child->parents.push_back(std::make_pair(parent, c));
            }
        };

    /*----------------------------------------------\\
    //                                              \\
    //               Airplane Class                 \\
    //                                              \\
    //----------------------------------------------*/
    
    // Airplane class, represents a piece on the board and handles the movements and interactions of pieces
        class Airplane {
            private:
                // pointer to the Airplane's current location on the real board
                Airport* location;
                // pointer to the Airplane's current location on the "shadow" simulation board
                Airport* shadow_location;
                // pointer to the Airport at the start of the map
                // this means an Airplane can jump back to the start without having to access and search the map
                Airport* start;
                // pointer to the Airport at the end of the map
                // this means an Airplane can jump forward to the end without having to access and search the map (not acutally done in gameplay currently)
                Airport* end;
                // pointer to the player that owns this piece, important when evaluating whether you are bumping an enemy plane or intruding on an ally's space
                Player* owner;

                // Special accessors:
                // these access private functions take a "main_board" flag and return the variable for the main or shadow board depending on that flag
                // these functions allow us to use the same functions for simulating movements on the shadow board and actually perfomring them on the main board
                // FUTURE: these functions really should return pointers to the vectors instead of the vectors themselves
                    // get the occupancy of the given airpost, accounting for main-shadow distinction
                    // Takes: (Airport*) location to get the occupancy
                    //        (bool) whether to get the real occupancy or the shadow_occupancy
                    // Returns: (vector<Airplane*>&) the correct occupancy vector for that space
                    vector<Airplane*>& get_occupancy_special(Airport* loc, bool main_board)  {
                        if(main_board) {
                            return loc->occupants;
                        }
                        else {
                            return loc->shadow_occupants;
                        }
                    }

                    // get the location of this airplane, accounting for main-shadow distinction
                    // Takes: (bool) whether to get the real location or the shadow_location
                    // Returns: (Airport*&) the correct location for this plane
                    Airport*& get_location_special(bool main_board) {
                        if(main_board) {
                            return location;
                        }
                        else {
                            return shadow_location;
                        }
                    }

                // function run when a plane gets bumped, accounts for shadow-main board distinction
                // moves the plane to the start, handling all location and occupancy tracking
                // also handles the owner's bumped_count benchmark and over_pm flag
                // if altered bumping is enabled, handles that
                // Takes: (bool) main_board
                // Returns: (bool) {unused for now}
                bool bumped(bool main_board) {
                    // update ocupancy at location
                    for(int i = 0; i < get_occupancy_special(get_location_special(main_board), main_board).size(); i++) {
                        if(this == get_occupancy_special(get_location_special(main_board), main_board)[i]) {
                            get_occupancy_special(get_location_special(main_board), main_board).erase(get_occupancy_special(get_location_special(main_board), main_board).begin()+i);
                            break;
                        }
                    }

                    // make sure we want to do the regular jump
                    bool do_start_jump(true);
                    // update over_pm flag and bumped count
                    // handle altered bumps
                    if(main_board) {
                        owner->bumped();
                        // if PM_TAKE_OFF_BUMP, takeoff to the eastern hemisphere
                        if(PM_TAKEOFF_BUMP && owner->is_over()) {
                            Airport* dest = owner->get_game()->draw_takeoff();
                            // so long as we're allowed to go there
                            bool open(true);
                            // if any plane on the destination has the same owner us, don't go there
                            for(int i = 0; i < dest->occupants.size(); i++) {
                                if(dest->occupants[i]->get_owner_ptr() == owner) {
                                    open = false;
                                }
                            }
                            if(open) {
                                // takeoff and track
                                do_action(Action(dest), true);
                                owner->altered_bump();
                                do_start_jump = false;
                                this->check_bump(main_board);
                            }
                        }
                        owner->setback();
                    }

                    // only false if altered bump occured
                    if(do_start_jump) {
                        // set location to start
                        get_location_special(main_board) = start;
                        // update occupancy at start
                        get_occupancy_special(start, main_board).push_back(this);
                    }

                }

                // moves this plane to the given Airport (on the main or shadow board depending on the flag)
                // Takes: (Airport*) destination
                //        (bool) main_board
                // Returns: (bool) {unused for now}
                bool move(Airport* dest, bool main_board) {
                    if(main_board && DEBUG && location != NULL) {
                        cout << "Moved from " << location->name << " to " << dest->name << ".\n  ";
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

                // jumps plane to given destination, returns true
                // FUTURE: redundant function since map rework, can be replaced with move in a scenarios
                // Takes: (Airport*) destination
                //        (bool) main_board
                // Returns: (bool) {unused for now}
                bool jump(Airport* dest, bool main_board) {
                    // avoid stationary jumps
                    if(dest == get_location_special(main_board)) {
                        // in stationary jump, nothing needs to be done
                        return true;
                    }
                    // else move
                    move(dest, main_board);
                    return true;
                }

                // moves plane along its child route with the given color
                // returns true if route existed and plane was moved, false otherwise
                // Takes: (Color) route color
                //        (bool) main_board
                // Returns: (bool) valid_route
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
                // Takes: (Color) route color
                //        (bool) main_board
                // Returns: (Airport*) destination
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
                // Takes: (Airport*) location
                //        (Airport*) start
                //        (Airport*) end
                //        (Player*) owner
                Airplane(Airport* loc = NULL, Airport* st = NULL, Airport* en = NULL, Player* own = NULL) : 
                    owner(own), 
                    location(loc),
                    start(st),
                    end(en) {
                        if(loc == NULL) {
                            loc = start;
                        }
                    }
                
                // copy constructor
                // Takes: (const Airplane&) other
                Airplane(const Airplane& other) :
                    location(other.location),
                    start(other.start),
                    end(other.end),
                    owner(other.owner) {}

                // assignment operator
                // Takes: (const Airplane&) other
                // Does: copies all pointers
                // Returns: (const Airplane&) *this
                const Airplane& operator=(const Airplane& other) {
                    location = other.location;
                    shadow_location = other.shadow_location;
                    start = other.start;
                    end = other.end;
                    owner = other.owner;

                    return *this;
                }

                // given an action, calls the proper function to perform that action
                // Takes: (Action) act
                //        (bool) main_board
                // Return: (bool) valid action
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
                // Takes: (bool) main_board
                // Returns: (bool) bumped
                // Throws: (std::domain_error) intruding on ally
                bool check_bump(bool main_board) {
                    bool did_the_thing = false;
                    // if there are multiple planes here
                    vector<Airplane*>* occ = &(get_occupancy_special(get_location_special(main_board), main_board));
                    if(occ->size() > 1) {
                        // for each plane
                        for(int i = 0; i < occ->size(); i++) {
                            // if we aren't at a the start or end or bumping ourself
                            if( !(occ->operator[](i) == this || get_location_special(main_board) == start || get_location_special(main_board) == end ) ) {
                                // if the plane has a different owner
                                if(occ->operator[](i)->owner != owner) {
                                    if(occ->operator[](i)->owner == NULL)
                                    // update bumper count
                                    if(main_board) {
                                        owner->bumper();
                                    }
                                    const Player* bumped_owner = occ->operator[](i)->owner;
                                    occ->operator[](i)->bumped(main_board);
                                    did_the_thing = true;
                                    if(main_board && DEBUG) {
                                        cout << owner->get_name() << " bumped " << bumped_owner->get_name() << " on " << location->name << "! ";
                                    }
                                    // don't repeat
                                    i--;
                                }
                                // if same owner, problem!
                                else {
                                    // this exists to be a debug mode breakpoint that only activates on a real move, not a simulation
                                    if(main_board) {
                                        int breaking = 1;
                                    }
                                    throw std::domain_error("trying to bump ally plane");
                                }
                            }
                        }
                    }
                    return did_the_thing;
                }

                // syncs shadow location with real location
                void sync() {
                    shadow_location = location;
                }

                // Board-sensitive accessors
                string get_loc_name(bool main_board) {
                    return get_location_special(main_board)->name;
                }

                Airport* get_loc(bool main_board) {
                    return get_location_special(main_board);
                }

                bool is_over(bool main_board) {
                    return (get_location_special(main_board)->lon <= 0);
                }

                // owner accessor and setter
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
    
    // Player class, handles decisionmaking and giving commands to planes, as well as tracking and compiling benchmarking data
        class Player {
            private:
                // pointer to the Game this player belongs to
                Game* game;
                // this player's color
                Color plane_color;
                // this player's planes
                vector<Airplane> planes;
                // how much this player values bumping other players
                // FUTURE: make this value a coefficient to be multiplied by adjusted enemy plane score (negative plus 180) (will prioritize planes further along)
                const float BUMP_VALUE = 20;
                // flags for a player who has all their finished or over the prime meridian
                bool finished, over_pm;

                // stores benchmarking data as pair<[total turns, bumpeD_count, bumpeR_count], [turns to finish first plane, turns to finish second plane, etc]
                pair<vector<int>, vector<int>> turn_data;
                // counters for how many times this player has bumped, been bumped, or been altered-bumped
                int bumper_count, bumped_count, altered_bump_count;

                // "defualt" colors list
                const vector<string> DEFUALT_COLORS = {"red", "orange", "yellow", "green", "blue", "purple"};

                // Takes: (vector<vector<Action*>>& a_p) a vector of permutations of action which is known to have wilds
                //        (int) the index of the permutation known to have a wild
                // Does: makes a copy of the permutation in question
                //       counts the wilds in the copy and replaces them with NULL pointers
                //       generates all possible permutations of color with a length equal to the number of wilds
                //       for each permutation of colors, makes a new permutation of actions based off the copy with blanks
                //          fills the blanks in the action permutation with color-move actions correspondign to the colors in the current color permutation
                //          pushes the completed action permutation onto the vector of permutations of actions
                // IMPROVE: The location of wild permutations and their deletion after expansion is handled in a loop in decide_moves when it really should be it's own function
                // Returns: it adds permutations to the vector
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

                // basic plane scoring, just goes by longitude of location
                // Takes: (Airplane*) given plane
                //        (bool) main_board
                // Returns: (float) score of given plane
                float get_plane_score(Airplane* a, bool main_board)  {
                    // really good if at end
                    if(a->get_loc_name(main_board) == "END") {
                        return 250;
                    }
                    return -(a->get_loc(main_board)->lon);
                }

                // given a set of actions, returns best movement plan
                // Takes: (vector<pair<Airplane*, Action*>>&) move_plan to be filled out
                //        (vector<Action>) actions to be taken
                // Does: Gets all possible orders that the given actions could be done and the order they can be applied to planes
                //       eliminates (adjacent) duplicate permutations and expands those with wilds
                //       Checks every permutation and selects the best one
                //          iterates through every combination of player permutation and action permutation
                //          simulates those movements on the shadow board
                //          scores the move plan based on the planes on the shadow board (extra points for bumping another player)
                //          tracks the move plan with the best score
                //       returns the best move_plan
                // IMPROVE: pretty much everything in here should be done in its own function
                void decide_moves(vector<pair<Airplane*, Action*>>& move_plan, vector<Action> actions)  {
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
                    // remove (adjacent) duplicate move permutations
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

                    // if wilds are enabled
                    if(WILDS) {
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
                                // referenced: "Goto stament in C/C++" from geeksforgeeks.com (https://www.geeksforgeeks.org/goto-statement-in-c-cpp/)
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

                    // I'm using a goto, I know it's evil but I gotta break out of the double for loop and this is really the most efficient way
                    // referenced: "Goto stament in C/C++" from geeksforgeeks.com (https://www.geeksforgeeks.org/goto-statement-in-c-cpp/)
                    all_done:
                        for(int i = 0; i < actions.size(); i++) {
                            move_plan[i] = std::make_pair( (plane_permutations[best_combo_indexes.first][i]), 
                                                            (new Action(action_permutations[best_combo_indexes.second][i]->color)) );   // memory allocation is weird
                        }
                        return;

                }
            
            public:        
                // default constructor
                // Takes: (Game*) g
                //        (vector<Airplane>) pl
                //        (Color) c 
                // Does: copies given values
                //       initilizes counters and flags
                //       sets the owner pointers of given planes
                Player(Game* g, vector<Airplane> p, Color c) :
                    game(g), 
                    planes(p), 
                    plane_color(c),
                    finished(false),
                    over_pm(false),
                    bumped_count(0),
                    bumper_count(0),
                    altered_bump_count(0) {
                        for(int i = 0; i < planes.size(); i++) {
                            planes[i].set_owner_ptr(this);
                        }
                    }

                // Permutation functions:
                // referenced: "Print all possible strings of length k that can be formed from a set of n characters" from geeksforgeeks.com. https://www.geeksforgeeks.org/print-all-combinations-of-given-length/
                //   while developing these algorithms
                // referenced zneak's answer from "Template container iterators" on StackOverflow https://stackoverflow.com/questions/30018517/template-container-iterators
                //   for how to template with my iterators
                    // get all permutations of a container of a given length and add them to the given storage container, allowing items to be repeated in a permutation
                    // Takes: (vector<vector<typename It::value_type>>&) output storage
                    //        (It) starting iterator
                    //        (It) ending iterator
                    //        (int) length
                    //        (vector<typename It::value_type>) prefix {nothing by default, used for recursion}
                    template<typename It>
                    static void get_all_permutations(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0))  {
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

                    // get all permutations of a container of a given length and add them to the given storage container, NOT allowing items to be repeated in a permutation
                    // Takes: (vector<vector<typename It::value_type>>&) output storage
                    //        (It) starting iterator
                    //        (It) ending iterator
                    //        (int) length
                    //        (vector<typename It::value_type>) prefix {nothing by default, used for recursion}
                    template<typename It>
                    static void get_all_permutations_no_repeats(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0))  {
                        
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


                // takes moves, decides how to do them, does them, tracks bumps, track planes at end, saves benchmarks
                // Takes: (vector<Action>) actions for this turn
                //        (vector<Player>&) player list
                // Does: First, the function uses any take-off actions
                //          it checks to makes sure they don't already have a plane on the destination (if it does, it just wastes the take-off)
                //          it finds their plane with the worst score and uses the take-ff on it
                //       If there are still actions left, call another function to decide how to use the remaining move, then execute them
                //       check each plane to see if it is at the end space, if so delete it from the list
                //          if that was the last plane, mark this player as finished
                //       check each plane to see if its bumping another plane
                //       check if every plane is over the prime meridian, if so update the flag for that
                // IMPROVE: the expenditure of take-offs could be its own function
                void take_turn(vector<Action> actions, vector<Player>& fellow_gamers) {
                    // if takeoffs are enabled
                    if(TAKEOFFS) {
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
                                i--;
                            }
                        }
                    }
                    // if there are still moves left
                    if(actions.size() > 0) {
                        // decide how to move given those actions
                        vector<pair<Airplane*, Action*>> move_plan(actions.size());
                        decide_moves(move_plan, actions);

                        // do moves
                        for(int i = 0; i < move_plan.size(); i++) {
                            move_plan[i].first->do_action(*(move_plan[i].second), true);
                            // clean up dynamic declarations
                            delete move_plan[i].second;
                        }
                    }

                    // check for planes/players at end
                    for(int i = 0; i < planes.size(); i++) {
                        // delete planes at end
                        if(planes[i].get_loc_name(true) == "END") {
                            //planes.erase(planes.begin()+i);
                            for(int e = i+1; e < planes.size(); e++) {
                                // find [e]'s loc's occ pointer to [e]
                                Airplane** occ_ptr;
                                for(int p = 0; p < planes[e].get_loc(true)->occupants.size(); p++) {
                                    if(&planes[e] == planes[e].get_loc(true)->occupants[p]) {
                                        occ_ptr = &(planes[e].get_loc(true)->occupants[p]);
                                    }
                                }
                                //DEBUG = false;
                                (planes[e-1]) = (planes[e]);
                                //DEBUG = true;
                                *occ_ptr = &(planes[e-1]);
                            }
                            planes.pop_back();

                            // BENCHMARKING: update turn data
                            turn_data.second.push_back(game->turn_num);
                            if(DEBUG) {
                                cout << "Plane Finished! ";
                            }

                            // if all planes finished, make note of it
                            if(planes.size() == 0) {
                                finished = true;
                                turn_data.first.push_back(game->turn_num+1);
                                turn_data.first.push_back(bumped_count);
                                turn_data.first.push_back(bumper_count);
                                turn_data.first.push_back(altered_bump_count);
                                if(DEBUG)
                                    cout << get_name() << " has finished!\n";
                            }
                            i--;
                        }
                    }
                    // check for bumps
                    for(int i = 0; i < planes.size(); i++) {
                        planes[i].check_bump(true);
                    }
                    // check for over_pm
                    if(!over_pm) {
                        bool over(true);
                        for(int i = 0; i < planes.size(); i++) {
                            if(!planes[i].is_over(true)) {
                                over = false;
                            }
                        }
                        over_pm = over;
                        if(over_pm && over) {
                            int breaking = 0;
                        }
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

                // gather all planes at the given airport
                void gather_planes(Airport* start) {
                    for(int i = 0; i < planes.size(); i++) {
                        DEBUG = false;
                        planes[i].do_action(Action(start), true);
                        DEBUG = true;
                    }
                }

                // Returns: (bool) finished
                bool is_done() const {
                    return finished;
                }

                // Returns: (bool) over_pm
                bool is_over() const {
                    return over_pm;
                }

                // Returns: (string) "Player " + color.c
                string get_name() const {
                    string out("Player ");
                    out += plane_color.c;
                    return out;
                }

                // Returns: game
                Game* get_game() {
                    return game;
                }

                // Returns: (vector<Airplane>&) planes
                vector<Airplane>& get_planes() {
                    return planes;
                }
        
                // Returns: (pair<vector<int>, vector<int>>) turn_data
                pair<vector<int>, vector<int>> get_turn_data() const {
                    return turn_data;
                }
        
                // over_pm = false;
                void setback() {
                    over_pm = false;
                }
        
                // bumper_count++;
                void bumper() {
                    bumper_count++;
                }

                // bumped_count++;
                void bumped() {
                    bumped_count++;
                }

                // altered_bump_count++;
                void altered_bump() {
                    altered_bump_count++;
                }
        };


    /*----------------------------------------------\\
    //                                              \\
    //                 Game Class                   \\
    //                                              \\
    //----------------------------------------------*/

    // everything for game objects
    // Games hold the game map, take-off decks, and players
    // Games handle the initialization of a game, the turn loop, and the collection of benchmark data

        // hash_table containing all the airports, keyed by airport name
        std::unordered_map<string, Airport> map;
        // two take-off decks, one for each hemisphere
        queue<string> western_hemi, eastern_hemi;
        // the players
        vector<Player> players;
        // the sides of the dice
        vector<string> sides; // = {"red", "orange", "yellow", "green", "blue", "purple", "wild", "take-off"};
        // settings
        const int NUM_OF_PLAYERS, PLANES_PER_PLAYER, MAX_TURNS;
        // turn tracking
        int turn_location, turn_num;
        // benchmarking data storage
        vector<pair<string, pair<vector<int>, vector<int>>>> player_turn_datas;
        vector<vector<pair<string, pair<vector<int>, vector<int>>>>> game_datas;
        // this Game's seed (for the map initilization)
        const int MAP_SEED;
    
        // Takes: (int) players = 4
        //        (int) planes = 4
        //        (bool) pm_takeoff_bump = false
        //        (bool) wilds = true
        //        (bool) takeoffs = true
        //        (bool) debug = false
        //        (int) max_turns) = 300
        // Does: copies given values, initializes dice sides, initializes map
        //       (the map and dice sides are initilized here because they will stay the same over multiple game-runs on the same game object)
        //       IMPROVE: (Well, sides actually can change but I'm not going to bother right now)
        Game(int player_count = 4, int planes_per_player = 4, bool pm_takeoff_bump = false, bool wilds = true, bool takeoffs = true, bool debug = false, int max_turns = 1000) : 
            NUM_OF_PLAYERS(player_count), 
            PLANES_PER_PLAYER(planes_per_player), 
            MAX_TURNS(max_turns),
            sides(DEFUALT_COLORS),
            MAP_SEED(time(0)) {
            // seed randomness
                srand(MAP_SEED);
            // update mode variables
                DEBUG = debug;
                PM_TAKEOFF_BUMP = pm_takeoff_bump;
                WILDS = wilds;
                TAKEOFFS = takeoffs;
            // init turn counters
                turn_location = turn_num = 0;
            // init sides
                if(WILDS) {
                    sides.push_back("wild");
                }
                if(TAKEOFFS) {
                    sides.push_back("take-off");
                }
            // initialize map
                initialize_map();
            }

        // function to run a single game
        void run_game() {
            int seed = new_game();
            loop();
            cout << ".";
            if(DEBUG) {
                cout << "\n\n";
            }
            // delay to ensure unique seeds for turn orders
            while(time(0) == seed) {
                ;
            }
        }

        // function to set up a game, seed randomness and creates players
        int new_game() {
            // seed randomness
                int seed = time(0);
                //int seed = 1589430794;
                if(DEBUG)
                    cout << seed << '\n';
                srand(seed);
            
            // update/reset game data
                player_turn_datas.clear();

            // initialize players
                players.clear();
                for(int i = 0; i < NUM_OF_PLAYERS; i++) {
                    // make planes vector of right size
                    vector<Airplane> temp_planes(PLANES_PER_PLAYER, Airplane(&(map.at("START")), &(map.at("START")), &(map.at("END"))));
                    // initialize player and add to vector
                    players.emplace_back(this, temp_planes, DEFUALT_COLORS[i]);
                }

            // move all planes to start and fix their owner pointers
                for(int y = 0; y < NUM_OF_PLAYERS; y++) {
                    for(int p = 0; p < PLANES_PER_PLAYER; p++) {
                        (map.at("START")).occupants.push_back(&(players[y].get_planes()[p]));
                        players[y].get_planes()[p].set_owner_ptr(&(players[y]));
                    }
                }

            // initialize take-off piles
                while(!western_hemi.empty()) {
                    western_hemi.pop();
                }
                while(!eastern_hemi.empty()) {
                    eastern_hemi.pop();
                }
                for(auto it = map.begin(); it != map.end(); it++) {
                    // don't push start or end
                    if(!((*it).first == "START" || (*it).first == "END")) {
                        if((*it).second.lon > 0) {
                            eastern_hemi.push((*it).first);
                        }
                        else {
                            western_hemi.push((*it).first);
                        } 
                    }
                }
            return seed;
        }

        // function to collect data from all games played with this object so far
        // Returns: (vector<int>) games_data [avg_win_turn, avg_lose_turn, avg_total_turn, avg_bumps(, avg_altered_bumps)]
        vector<float> get_games_data() {
            int total_winner_turn(0);
            int total_loser_turn(0);
            int total_player_turn(0);
            int total_bumps(0);
            int total_altered_bumps(0);
            for(int g = 0; g < game_datas.size(); g++) {
                for(int i = 0; i < game_datas[g].size(); i++) {
                    // track winner turn
                    if(i == 0) {
                        total_winner_turn += game_datas[g][i].second.first[0];
                    }
                    // track loser turn
                    else if(i == game_datas[g].size()-1) {
                        total_loser_turn += game_datas[g][i].second.first[0];
                    }
                    total_player_turn += game_datas[g][i].second.first[0];
                    total_bumps += game_datas[g][i].second.first[1];
                    if(PM_TAKEOFF_BUMP) {
                        total_altered_bumps += game_datas[g][i].second.first[3];
                    }
                }
            }

            vector<float> output;
            output.push_back((float)total_winner_turn/(float)game_datas.size());
            output.push_back((float)total_loser_turn/(float)game_datas.size());
            output.push_back((float)total_player_turn/(float)game_datas.size());
            output.push_back((float)total_bumps/(float)game_datas.size());
            if(PM_TAKEOFF_BUMP) {
                output.push_back((float)total_altered_bumps/(float)game_datas.size());
            }

            if(DEBUG) {
                cout << "\nWinners took an average of " << output[0] << " turns to finish\n"
                    << "Losers took an average of " << output[1] << " turns to finish\n"
                    << "There were, on average, " << output[2] << " total turns per game\n"
                    << "There were, on average, " << output[3] << " total bumps per game\n";
                if(PM_TAKEOFF_BUMP) {
                    cout << "There were an average of " << output[4] << " altered bumps\n";
                }
            }

            // clear game results, player, and take-off data
            game_datas.clear();
            players.clear();

            // I got this method of erasing queues from David Rodriguez's answer to "How do I clear the std::queue efficiently?" on StackOverflow (https://stackoverflow.com/questions/709146/how-do-i-clear-the-stdqueue-efficiently)
            queue<string> empty1;
            queue<string> empty2;
            std::swap(western_hemi, empty1);
            std::swap(eastern_hemi, empty2);

            return output;
        }

        // game loop, runs turn loop, including giving players their rolls, collects benchmarking data
        void loop() {
            bool someone_left(true);
            // while ther are turn and players left
            for(turn_num = 0; turn_num != MAX_TURNS && someone_left; turn_num++) {
                //if(DEBUG) {
                    cout << "Turn #" << turn_num+1 << ":\n";
                //}
                someone_left = false;
                // have each player take their turn
                for(turn_location = 0; turn_location < players.size(); turn_location++) {
                    // if the player is still in the game
                    if(!players[turn_location].is_done()) {
                        if(DEBUG) {
                            cout << players[turn_location].get_name() << "'s turn:\n  ";
                        }
                        // take their turn
                        players[turn_location].take_turn(roll(&(players[turn_location])), players);

                        // check for end, update flags and data accordingly
                        if(players[turn_location].is_done())
                            player_turn_datas.push_back(std::make_pair(players[turn_location].get_name(), players[turn_location].get_turn_data()));
                        else 
                            someone_left = true;
                    }
                }
            }
            if(DEBUG) {
                cout << "All players have finished!\nFinishing data:\n";
                // output player_turn_data
                for(int i = 0; i < player_turn_datas.size(); i++) {
                    cout << "   " << player_turn_datas[i].first << " finshed in " << player_turn_datas[i].second.first[0] << " turns.\n";
                    cout << "They got bumped " << player_turn_datas[i].second.first[1] << " times and bumped other players " << player_turn_datas[i].second.first[2] << " times.\n";
                    for(int j = 0; j < player_turn_datas[i].second.second.size(); j++) {
                        cout << "      Plane #" << j+1 << " finished in " << player_turn_datas[i].second.second[j]+1 << " turns.\n";
                    }
                }
            }
            game_datas.push_back(player_turn_datas);
        }

        // simulate drawing a take-off card, draws from hemisphere denoted by bool
        // Takes: (bool) west = false
        // Returns: (Airport*) dest
        Airport* draw_takeoff(bool west = false) {
            queue<string>* correct_pile(&eastern_hemi);
            if(west) {
                correct_pile = &western_hemi;
            }
            // move the one we're drawing to the bottom
            correct_pile->push(correct_pile->front());
            correct_pile->pop();
            // return the one on the bottom
            return &(map.at(correct_pile->back()));
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

        // roll actions for player
        // Takes: (const Player*) roller
        //        (int) dice
        // Returns: (vector<Action>) rolls
        vector<Action> roll(const Player* roller, int dice = 2) {
            // get string versions
            vector<string> rolls(dice);
            for(int i = 0; i < dice; i++) {
                rolls[i] = sides[rand()%sides.size()];
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
                    actions.emplace_back(draw_takeoff(roller->is_over()));
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
                cout << " ]\n  "; 
            }

            // return vector of actions
            return actions;
        }

        // initializes map by reading airport data from a file and automaticall creating connections to "close" airports
        // FUTURE: each game object has a map_file string
        //         get close airports by longitude AND latitude instead of jump longitude (avoid super vertical routes?)
        //         randomize which close airports to connect to, don't just choose the closest ones
        //         create single-color tranversals before filling connections
        void initialize_map() {
            // generate Airports and connections
            map.clear();

            // initialize file
            std::ifstream map_source("map.txt");
            if(!map_source.is_open()) {
                throw std::runtime_error("can't find map.txt");
            }
            // read airports from file
            string name;
            float lati, longi;
            // referenced Kerrek SB's answer to "Read file line by line using ifstream in C++" on Stack Overflow (https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c)
            // for this method of looping a file
            while(map_source >> name >> lati >> longi) {
                map.insert(std::make_pair(name, Airport(name, lati, longi)));
            }
        
            vector<Airport*> connectees;
            for(auto it = map.begin(); it != map.end(); it++) {
                connectees.push_back(&(*it).second);
            }
            // sort by latittude
            std::sort(connectees.begin(), connectees.end(), [](Airport* a, Airport* b) { return (a->lon > b->lon);});

            // for each plane, starting in the east (don't bother with the last one)
            for(int i = 0; i < connectees.size()-1; i++) {
                // number of connections (1 to all colors)
                int connections = 1+rand()%DEFUALT_COLORS.size();
                // always make max connections for start
                if(connectees[i]->name == "START") {
                    connections = DEFUALT_COLORS.size();
                }

                vector<string> unused_colors(DEFUALT_COLORS);

                // for the next (connections) airports in the vector
                for(int j = i+1; j < connectees.size() && j-i <= connections; j++) {
                    // select which color we are using
                    int color_index = rand()%unused_colors.size();

                    // make connection
                    Airport::connect(connectees[i], connectees[j], Color(unused_colors[color_index]));

                    //cout << "Connected " << connectees[i]->name << " & " << connectees[j]->name << " with " << unused_colors[color_index] << '\n';

                    // erase used color
                    unused_colors.erase(unused_colors.begin()+color_index);
                }
            }
        }

        // updates settings to given settings
        void update_settings(bool pm_takeoff_bump, bool wilds, bool takeoffs) {
            // update the easy one
            PM_TAKEOFF_BUMP = pm_takeoff_bump;
            // update the wilds
            // remove it or add it to the dice as needed
            if(WILDS && !wilds) {
                for(auto it = sides.begin(); it != sides.end(); it++) {
                    if(*it == "wild") {
                        sides.erase(it);
                        break;
                    }
                }
            }
            else if(!WILDS && wilds) {
                sides.push_back("wild");
            }
            WILDS = wilds;
            if(TAKEOFFS && !takeoffs) {
                for(auto it = sides.begin(); it != sides.end(); it++) {
                    if(*it == "take-off") {
                        sides.erase(it);
                        break;
                    }
                }
            }
            else if(!TAKEOFFS && takeoffs) {
                sides.push_back("take-off");
            }
            TAKEOFFS = takeoffs;
        }
};

#endif