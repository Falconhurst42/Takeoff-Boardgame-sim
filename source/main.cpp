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

int main() {
    // total game runs for each scenario
    int RUNS(1);
    cout << "How many runs per scenario? ";
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

    vector<pair<Scenario, vector<int>>> scenario_results;

    // for each scenario
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
    }

    // output the benchmarking data
    // for each scenario result

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
            cout << "There were a total of " << scenario_results[i].second[4] << " altered bumps\n";
        }
        cout << "\n";
    }

    // .md results
    std::ofstream results_file("results.md", std::fstream::trunc);
    if(!results_file.is_open()) {
        throw std::runtime_error("results file didn't open");
    }
    // header
    results_file << "|| Avg Winner Turns | Avg Loser Turns | Avg Total Turns | Avg Total Bumps | Total Altered Bumps |\n|-:|:-:|:-:|:-:|:-:|:-:|\n";
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