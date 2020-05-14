#include "..\headers\Game.h"

int main() {
    Game test(4, 4);
    while(true) {
        test.run_game();
        cout << "\n\n\n\nPress any key for next simulation: ";
        char temp;
        std::cin >> temp;
        if(temp == 'n') {
            break;
        }
    }
}