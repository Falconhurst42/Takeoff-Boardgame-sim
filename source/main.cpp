#include "..\headers\Game.h"

int main() {
    Game test(1, 2, 100);
    while(true) {
        test.run_game();
        cout << "Press any key for next simulation: ";
        string temp;
        std::cin >> temp;
    }
}