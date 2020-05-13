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

// Airplane Class
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

class Player {
    private:
        Game* game;
        vector<Airplane> planes;
        Color plane_color;
        const float BUMP_VALUE = 20;
        bool finished;

        // returns vector with Actions to be taken this turn
        vector<Action> roll(int dice = 2) {
            vector<string> sides(plane_color.DEFUALT_COLORS); // = {"red", "orange", "yellow", "green", "blue", "purple", "wild", "take-off"};
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
                    actions.emplace_back('T', game->draw_takeoff());
                }
                // generate color action
                else {
                    actions.emplace_back('C', rolls[i]);
                }
            }

            if(DEBUG) {
                cout << "[ "; 
                for(int i = 0; i < actions.size(); i++) {
                    cout << "(" << actions[i].type << ", " << actions[i].info << "), ";
                }
                cout << " ]"; 
            }

            // return vector of actions
            return actions;
        }

        // default plane scoring, just goes by latitude of location
        float get_plane_score(Airplane a, bool main_board) {
            // really good if at end
            if(a.get_loc_name(main_board) == "END") {
                return 250;
            }
            return -(a.get_loc(main_board)->lon);
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

            // expand wilds
            // for each permutation
            for(int i = 0; i < action_permutations.size(); i++) {
                // for each action in the permutation
                for(int j = 0; j < action_permutations.size(); j++) {
                    // if the action is wild
                    if(action_permutations[i][j].type == 'C' && action_permutations[i][j].info == "wild") {
                        // create copies of the permutation replacing the wild with each of the default colors
                        for(int c = 0; c < plane_color.DEFUALT_COLORS.size(); c++) {
                            vector<Action> temp(action_permutations[i]);
                            temp[j].info = plane_color.DEFUALT_COLORS[c];
                            action_permutations.push_back(temp);
                        }
                    }
                }
            }

            // tracks best combination so far
            int best_actions_used = 0;  // highest priority
            float best_score = -10000;  // secondary priority
            pair<int, int> best_combo_indexes;

            // if there isn't a valid move at the maximum number of moves, we will decrease the number of moves until we find a valid move
            for(int action_count = actions.size(); action_count > 0; action_count--) {
                // for each permutation of planes
                for(int i = 0; i < plane_permutations.size(); i++ ) {
                    // for each permutation of actions applied to that order of planes
                    for(int j = 0; j < action_permutations.size; j++) {
                        // simulate that particular set of moves
                        // this will all done with "main_board" flagged as false
                        // which means we are moving planes in a shadow version of the board
                        // which will not effect the main board

                        // sync shadow board to main board
                        game->sync_boards();

                        int actions_used(0);

                        // do actions on imaginary planes
                        for(int t = 0; t < action_count; t++) {
                            // track successful moves
                            if(plane_permutations[i][t].do_action(action_permutations[j][t], false)) {
                                actions_used++;
                            }
                        }

                        int score = 0;

                        // check bumps and add points
                        for(int b = 0; b < plane_permutations[i].size(); b++) {
                            if(plane_permutations[i][b].check_bump(false)) {
                                score += BUMP_VALUE;
                            }
                        }

                        // add individual plane scores
                        for(int p = 0; p < plane_permutations[i].size(); p++) {
                            score += get_plane_score(plane_permutations[i][p], false);
                        }

                        // update best score
                        if(actions_used >= best_actions_used && score > best_score) {
                            best_score = score;
                            best_actions_used = actions_used;
                            best_combo_indexes = std::make_pair(i, j);
                        }
                    }
                }
            }

            // transform best moves into return structure
            vector<pair<Airplane, Action>> best_moves(actions.size());
            for(int i = 0; i < actions.size(); i++) {
                best_moves[i] = std::make_pair( plane_permutations[best_combo_indexes.first][i], 
                                                action_permutations[best_combo_indexes.second][i] );
            }

            return best_moves;
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
        Player(Game* g, vector<Airplane> p, Color c)  :
            game(g), 
            planes(p), 
            plane_color(c),
            finished(false) {
                for(int i = 0; i < planes.size(); i++) {
                    planes[i].set_owner_ptr(this);
                }
            }

        // rolls moves, decides how to do them, and does them
        void take_turn() {
            // roll to get actions
            vector<Action> actions = roll();

            // do take-offs on worst planes
            for(int i = 0; i < actions.size(); i++) {
                // if an action is a take-off
                if(actions[i].type == 'T') {
                    // find the worst plane
                    Airplane worst_plane = planes[0];
                    float worst_score = get_plane_score(worst_plane, true);
                    for(int j = 1; j < planes.size; j++) {
                        float score = get_plane_score(planes[j], true);
                        if(score < worst_score) {
                            worst_plane = planes[j];
                            worst_score = score;
                        }
                    }
                    // and use the take-off on it
                    worst_plane.do_action(actions[i], true);

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
                vector<pair<Airplane, Action>> move_plan = decide_moves(actions);

                // do moves
                for(int i = 0; i < move_plan.size(); i++) {
                    move_plan[i].first.do_action(move_plan[i].second, true);
                }
            }

            // check for bumps and planes at end
            for(int i = 0; i < planes.size(); i++) {
                // delete planes at end
                if(planes[i].get_loc_name(true) == "END") {
                    planes.erase(planes.begin()+i);
                    if(planes.size() == 0) {
                        finished == true;
                        if(DEBUG) {
                            cout << "A Player has finished!\n";
                        }
                    }
                }
                // check for bumps
                planes[i].check_bump(true);
            }

            if(DEBUG) {
                cout << "\n\n";
            }
        }

        // sync shadow location with real location for all planes
        void sync_planes() {
            for(int i = 0; i < planes.size(); i++) {
                planes[i].sync();
            }
        }

        Game* get_game() const {
            return game;
        }

        bool is_done() const {
            return finished;
        }
};

class Game {
    public:
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
            MAX_TURNS(max_turns),
            turn_location(0),
            turn_num(0) {
                srand(time(0));
                // initialize players
                for(int i = 0; i < player_count; i++) {
                    // make planes vector of right size
                    vector<Airplane> temp_planes(planes_per_player);
                    // initialize player and add to vector
                    players.emplace_back(this, temp_planes, (new Color())->DEFUALT_COLORS[i]);  // can't really make a static const array, so I have to do this to access default colors
                }
                // generate Airports and connections
                    // end goal: pull airport data from .txt file and generate connections automatically
                    // intermediate: pull airport data from .txt file, data includes connections
                    // basic: manually initialize airports and connections
                // make airports
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
                    Airport::connect(&(map.at("London")), &(map.at("Dubia")), Color("orange"));
                    Airport::connect(&(map.at("London")), &(map.at("Kemton Park")), Color("green"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("Kemton Park")), Color("purple"));
                    Airport::connect(&(map.at("London")), &(map.at("New York")), Color("blue"));
                    Airport::connect(&(map.at("London")), &(map.at("Altanta")), Color("green"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("Atlanta")), Color("red"));
                    Airport::connect(&(map.at("Dubai")), &(map.at("New York")), Color("yellow"));
                    Airport::connect(&(map.at("Kemton Park")), &(map.at("Atlanta")), Color("red"));
                    Airport::connect(&(map.at("Los Angeles")), &(map.at("Atlanta")), Color("orange"));
                    Airport::connect(&(map.at("New York")), &(map.at("Atlanta")), Color("blue"));
                    Airport::connect(&(map.at("END")), &(map.at("Atlanta")), Color("purple"));
                    Airport::connect(&(map.at("New York")), &(map.at("Los Angeles")), Color("yellow"));
                    Airport::connect(&(map.at("New York")), &(map.at("END")), Color("red"));
                    Airport::connect(&(map.at("END")), &(map.at("Los Angeles")), Color("green"));


                // initialize take-off pile
                for(auto it = map.begin(); it != map.end(); it++) {
                    takeoff_pile.push((*it).first);
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
                        players[turn_location].take_turn();
                        someone_left = true;
                    }
                }
            }
        }

        // simulate drawing a take-off card
        string draw_takeoff() {
            // move the one we're drawing to the bottom
            takeoff_pile.push(takeoff_pile.front());
            takeoff_pile.pop();
            // return the one on the bottom
            return takeoff_pile.back();
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

};

int main() {
    Game test(1, 1);
    test.loop();
}