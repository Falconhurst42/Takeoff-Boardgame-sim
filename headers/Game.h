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

bool DEBUG(true);


// forward declarations
class Game;
class Player;
class Airplane;
class Airport;
struct Color;
struct Action;

#include "Color.h"
#include "Action.h"
#include "Airport.h"
#include "Airplane.h"
#include "Player.h"

using std::vector, std::queue, std::set, std::pair, std::string, std::cout;

class Game {
    public:
    //private:
        std::unordered_map<string, Airport> map;
        queue<string> takeoff_pile;
        vector<Player> players;
        const int NUM_OF_PLAYERS, PLANES_PER_PLAYER, MAX_TURNS;
        int turn_location, turn_num;
    //public:
        Game(int player_count = 4, int planes_per_player = 4, int max_turns = 100);

        // game loop
        void loop();

        // simulate drawing a take-off card
        string draw_takeoff();

        // sync all shadow occupancies with regular occupancies and all shadow locations with regular locations
        void sync_boards();

};


#endif
