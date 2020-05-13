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

#include "Game.h"
#include "Airplane.h"
#include "Color.h"
#include "Action.h"

using std::vector, std::set, std::pair, std::string, std::cout;

class Player {
    private:
        Game* game;
        vector<Airplane> planes;
        Color plane_color;
        const float BUMP_VALUE = 20;
        bool finished;

        // returns vector with Actions to be taken this turn
        vector<Action> roll(int dice = 2);

        // default plane scoring, just goes by latitude of location
        float get_plane_score(Airplane a, bool main_board);

        // given a set of actions, returns best movement plan
        vector<pair<Airplane, Action>> decide_moves(vector<Action> actions);

        // referenced: "Print all possible strings of length k that can be formed from a set of n characters" from geeksforgeeks.com. https://www.geeksforgeeks.org/print-all-combinations-of-given-length/
        //   while developing this algorithm
        // and referenced zneak's answer from "Template container iterators" on StackOverflow https://stackoverflow.com/questions/30018517/template-container-iterators
        //   for how to template with my iterators
        template<typename It>
        void get_all_permutations(vector<vector<typename It::value_type>>& output, It start, It end, int length, vector<typename It::value_type> prefix = vector<typename It::value_type>(0));
    
    public:        
        Player(Game* g, vector<Airplane> p, Color c);

        // rolls moves, decides how to do them, and does them
        void take_turn();

        // sync shadow location with real location for all planes
        void sync_planes();

        Game* get_game() const;

        bool is_done() const;
};

#endif