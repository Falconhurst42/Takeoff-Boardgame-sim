#ifndef PLAYER_H_
#define PLAYER_H_

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>

#include <set>
#include <utility>
#include <vector>
#include <string>

//#include "Game.h"

class Game;

#include "Color.h"
#include "Action.h"
#include "Airplane.h"


using std::vector, std::set, std::pair, std::string, std::cout;

class Player {
    private:
        Game* game;
        vector<Airplane> planes;
        Color plane_color;
        const float BUMP_VALUE = 20;
        bool finished;

        // default plane scoring, just goes by latitude of location
        float get_plane_score(Airplane a, bool main_board);

        // given a set of actions, returns best movement plan
        vector<pair<Airplane, Action>> decide_moves(vector<Action> actions, vector<Player> fellow_gamers);

        // referenced: "Print all possible strings of length k that can be formed from a set of n characters" from geeksforgeeks.com. https://www.geeksforgeeks.org/print-all-combinations-of-given-length/
        //   while developing this algorithm
        // and referenced zneak's answer from "Template container iterators" on StackOverflow https://stackoverflow.com/questions/30018517/template-container-iterators
        //   for how to template with my iterators
        template<typename It>
        void get_all_permutations(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0));
    
    public:        
        Player(Game* g, vector<Airplane> p, Color c);

        // takes moves, decides how to do them, and does them
        void take_turn(vector<Action> actions, vector<Player> fellow_gamers);

        // sync shadow location with real location for all planes
        void sync_planes();

        bool is_done() const;
};

#include "Player.cpp"

#endif