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

#endif