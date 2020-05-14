#include "..\headers\Game.h"

int main() {
    Game test(4, 4, true, true, false);
    int runs(1);
    cout << "How many runs? ";
    std::cin >> runs;
    while(runs) {
        test.run_game();
        runs--;
    }
    test.end_game();
}