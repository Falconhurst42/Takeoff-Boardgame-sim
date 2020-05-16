// Ethan Worth
// 5/14/2020
// Data Structures Final Project

// This file contains the code to run many takeoff games under different conditions and collect the results
// The results are output to "results.md" and cout
// On my machine, it takes 1:38 to run it with 1 game per scenario
//                         13:19 for 10 runs
// So I'm 

#include "..\headers\Game.h"

// a structure for storing the various settings for a run of Takeoff
struct Scenario {
    // settings
    int players, planes;
    bool pm_takeoff_bump, wilds, takeoffs;

    // basic compiler with default values
    Scenario(bool w = true, bool t = true, bool pm = false, int py = 4, int pn = 4) :
        wilds(w),
        takeoffs(t),
        pm_takeoff_bump(pm),
        players(py),
        planes(pn) {}
    
    // returns the settings of the scenario as
    // "(players x planes, " followed by whatever settings are on
    // for example: "(4x4, Altered Bumps, No Wilds, No Takeoffs, )"
    string to_string() {
        string output("(");
        output += std::to_string(players);
        output += "x";
        output += std::to_string(planes);
        output += ", ";
        if(pm_takeoff_bump)
            output += "Altered Bumps, ";
        if(!wilds)
            output += "No Wilds, ";
        if(!takeoffs)
            output += "No Takeoffs, ";
        output += ")";
        return output;
    }
};

// a function for converting a vector to a readable string
// "[0, 1, ..., size-1]"
template <typename T>
string vector_to_string(vector<T> source) {
    string out("[");
    for(int i = 0; i < source.size()-1; i++) {
        out += std::to_string(source[i]);
        out += ", ";
    }
    out += std::to_string(source[source.size()-1]);
    out += "]";
    return out;
}

// runs the given scenario in the given game for the given number of runs and adds the data to the correct places in the results_container
void run_game_and_scenario(Game* game, vector<pair<Scenario, vector<float>>>& results_container, vector<Scenario>& scenarios, int scen_ind = 0, unsigned int runs = 1) {
    // update settings
    game->update_settings(scenarios[scen_ind].pm_takeoff_bump, scenarios[scen_ind].wilds, scenarios[scen_ind].takeoffs);
    // each run
    for(int run = 0; run < runs; run++) {
        // run game
        game->run_game();
    }
    // get results
    vector<float> results = game->get_games_data();
    // add results to container
    for(int i = 0; i < results.size(); i++) {
        results_container[scen_ind].second[i] += results[i];
    }
}

int main() {
    // total game runs for each scenario
    unsigned int RUNS(1);
    unsigned int MAPS(1);
    cout << "How many maps? ";
    std::cin >> MAPS;
    cout << "How many runs per scenario per map? ";
    std::cin >> RUNS;

    // generate scenarios
        // get all permutations of 3 bools
        vector<bool> options = {true, false};
        vector<vector<bool>> switches;
        Game::Player::get_all_permutations(switches, options.begin(), options.end(), 3);

        // turn those into scenarios
        vector<Scenario> scenarios;
        for(int i = 0; i < switches.size(); i++) {
            scenarios.push_back(Scenario(switches[i][0], switches[i][1], switches[i][2]));
        }

    vector<pair<Scenario, vector<float>>> scenario_results;
    for(int i = 0; i < scenarios.size(); i++) {
        if(scenarios[i].pm_takeoff_bump) {
            scenario_results.push_back(std::make_pair(scenarios[i], vector<float>(5, 0)));
        }
        else {
            scenario_results.push_back(std::make_pair(scenarios[i], vector<float>(4, 0)));
        }
    }

    /*// for each scenario
    for(int i = 0; i < scenarios.size(); i++) {
        cout << "\nScenario #" << i+1;
        // set up the game object
        Game test(scenarios[i].players, scenarios[i].planes, scenarios[i].pm_takeoff_bump, scenarios[i].wilds, scenarios[i].takeoffs);
        int runs(RUNS);
        // run the game over and over again
        while(runs) {
            test.run_game();
            runs--;
        }
        // collect the benchmarking data
        scenario_results.push_back(std::make_pair(scenarios[i], test.get_games_data()));
    }*/

    for(int map = 0; map < MAPS; map++) {
        cout << "\nMap #" << map+1 << ":";
        // create game (and map)
        Game test(scenarios[0].players, scenarios[0].planes);
        // run each scenario on the map and add the data
        for(int scen_ind = 0; scen_ind < scenarios.size(); scen_ind++) {
            cout << "\n   Scenario #" << scen_ind+1 << ": ";
            run_game_and_scenario(&test, scenario_results, scenarios, scen_ind, RUNS);
        }
    }
    // adjust averages of benchmark data according to number of maps
    for(int scen_ind = 0; scen_ind < scenarios.size(); scen_ind++) {
        for(int result_ind = 0; result_ind < scenario_results[scen_ind].second.size(); result_ind++) {
            scenario_results[scen_ind].second[result_ind] /= MAPS;
        }
    }

    // cout results
    cout << "\n";
    for(int i = 0; i < scenario_results.size(); i++) {
        // output scenario
        cout << "Scenario #" << i+1 << ":\n  "
             << scenario_results[i].first.to_string() << "\n  ";

        // output results summary
        cout << vector_to_string(scenario_results[i].second) << "\n  ";

        // output results proper
        cout << "Winners took an average of " << scenario_results[i].second[0] << " turns to finish\n  "
             << "Losers took an average of " << scenario_results[i].second[1] << " turns to finish\n  "
             << "There were, on average, " << scenario_results[i].second[2] << " total turns per game\n  "
             << "There were, on average, " << scenario_results[i].second[3] << " total bumps per game\n  ";
        if(scenario_results[i].first.pm_takeoff_bump) {
            cout << "There were, on average, " << scenario_results[i].second[4] << " altered bumps\n";
        }
        cout << "\n";
    }

    // .md results
    std::ofstream results_file("results.md", std::fstream::trunc);
    if(!results_file.is_open()) {
        throw std::runtime_error("results file didn't open");
    }
    // header
    results_file << "| " << MAPS << "x" << RUNS << " | Avg Winner Turns | Avg Loser Turns | Avg Total Turns | Avg Total Bumps | Avg Total Altered Bumps |\n|-:|:-:|:-:|:-:|:-:|:-:|\n";
    // data
    for(int i = 0; i < scenario_results.size(); i += 2) {
        // first row header
        results_file << "| __";
        if(!scenario_results[i].first.wilds)
            results_file << "No ";
        results_file << "Wilds, ";
        if(!scenario_results[i].first.takeoffs)
            results_file << "No ";
        results_file << "Takeoffs__ | ";

        // data without altered bumps
        for(int j = 0; j < 4; j++) {
            results_file << scenario_results[i+1].second[j] << " | ";
        }
        results_file << " |\n";

        // second row header
        results_file << "| _(with altered bumps):_ | ";
        for(int j = 0; j < 5; j++) {
            results_file << scenario_results[i].second[j] << " | ";
        }
        results_file << "\n|\n";
    }
}