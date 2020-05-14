#include "..\headers\Game.h"

int main() {
    int players, planes;
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
    }


    Game test(players, planes, _pm_takeoff_bump, _wilds, _takeoffs);
    int runs(1);
    cout << "How many runs? ";
    std::cin >> runs;
    while(runs) {
        test.run_game();
        runs--;
    }
    test.end_game();
}