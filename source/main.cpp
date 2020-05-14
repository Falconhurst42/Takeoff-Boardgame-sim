#include "..\headers\Game.h"

int main() {
    Game test(1, 2, 100);
    for(int i = 0; i < 10; i++) {
        test.run_game();
        cout << "Press any key for next simulation: ";
        string temp;
        std::cin >> temp;
    }
}