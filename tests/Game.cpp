#include "Game.h"

Game::Game(int player_count, int planes_per_player, int max_turns) : 
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
void Game::loop() {
    bool someone_left(true);
    // while ther are turn and players left
    for(turn_num = 0; turn_num != MAX_TURNS && someone_left; turn_num++) {
        someone_left = false;
        // have each player take their turn
        for(turn_location = 0; turn_location < players.size(); turn_location++) {
            if(!players[turn_location].is_done()) {
                players[turn_location].take_turn(roll(), players);
                someone_left = true;
            }
        }
    }
}

// simulate drawing a take-off card
Airport* Game::draw_takeoff() {
    // move the one we're drawing to the bottom
    takeoff_pile.push(takeoff_pile.front());
    takeoff_pile.pop();
    // return the one on the bottom
    return &(map.at(takeoff_pile.back()));
}

// sync all shadow occupancies with regular occupancies and all shadow locations with regular locations
void Game::sync_boards() {
    for(auto it = map.begin(); it != map.end(); it++) {
        (*it).second.shadow_occupants = (*it).second.occupants;
    }
    for(int i = 0; i < players.size(); i++) {
        players[i].sync_planes();
    }
}

// returns vector with Actions to be taken this turn
vector<Action> Game::roll(int dice) {
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
            actions.emplace_back(draw_takeoff());
        }
        // generate color action
        else {
            actions.emplace_back(Color(rolls[i]));
        }
    }

    // DEBUG
        cout << "[ "; 
        for(int i = 0; i < actions.size(); i++) {
            cout << "(" << actions[i].type << ", " << actions[i].info << "), ";
        }
        cout << " ]"; 

    // return vector of actions
    return actions;
}