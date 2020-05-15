#include "..\headers\Game.h"

struct Scenario {
    int players, planes;
    bool pm_takeoff_bump, wilds, takeoffs;

    Scenario(bool w = true, bool t = true, bool pm = false, int py = 4, int pn = 4) :
        wilds(w),
        takeoffs(t),
        pm_takeoff_bump(pm),
        players(py),
        planes(pn) {}
    
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

template <typename T>
string to_string(vector<T> source) {
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
    /*int players, planes;
    bool _pm_takeoff_bump(true), _wilds(true), _takeoffs(true);
    char input('n');
    cout << "How many players? ";
    std::cin >> players;
    cout << "How many planes per player? ";
    std::cin >> planes;
    cout << "pm_takeoff_bump? (n) ";
    std::cin >> input;
    if(input == 'n') {
        _pm_takeoff_bump = false;
    }
    cout << "wilds? (n) ";
    std::cin >> input;
    if(input == 'n') {
        _wilds = false;
    }
    cout << "takeoffs? (n) ";
    std::cin >> input;
    if(input == 'n') {
        _takeoffs = false;
    }*/

    int RUNS(1);
    cout << "How many runs? ";
    std::cin >> RUNS;

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
        Game test(scenarios[i].players, scenarios[i].planes, scenarios[i].pm_takeoff_bump, scenarios[i].wilds, scenarios[i].takeoffs);
        int runs(RUNS);
        while(runs) {
            test.run_game();
            runs--;
        }
        scenario_results.push_back(std::make_pair(scenarios[i], test.get_games_data()));
    }

    // for each scenario result
    for(int i = 0; i < scenario_results.size(); i++) {
        // output scenario
        cout << "Scenario #" << i+1 << ":\n  "
             << scenario_results[i].first.to_string() << "\n  ";

        // output results summary
        cout << to_string(scenario_results[i].second) << "\n\n";

        // output results proper
    }
}