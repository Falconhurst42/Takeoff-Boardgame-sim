# Main Point of Sim:
Players with Airplanes using Actions to move them between Airports
(All this lives inside a Game object)
## Game rolls to get Actions
A player rolls two dice which could result in one of the defualt colors, a wild, or a "take-off" card that will jump a plane of that player's choice to a random location on the map
If doubles are rolled, a player can take that action 4 times instead of 2
Player decides what to do with those Actions
Players will use take-offs first if there are able to
Then the Player will simulate every possible turn and it's outcome. Simulations are acheived by moving planes on a shadow version of the regular board (not a copy, the board has a shadow side built in)
each outcome is scored on plane position and number of bumps on other Players
the Player selects the best course of action (which must use as many actions as possible) and sends the orders to the planes
Airplanes execute those actions, moving themselves on the Game's map. Airplanes receive actions and move the planes accordingly both in the main Game's map and their own location tracking variables
The Player's function resolves bumps and plane's which have reached the end
This turn loop continues until all Players have finished or a maximum turn number is reached

# Path forward
Make visual map (using equirectangular projection would be good)
Auto-map generation
Improve or vary Player Ai

# Major Problems:
## Compiling the darn thing
The classes have a lot of interdependency
Even when I reduced that, it took me a couple tries to arrange the files in an acceptable way
Had to put everything in the same Class in the same '.h' file
## Handling variables
While I selected the right data-types when initially building my system, I had to go back and switch a lot of the data types to pointers because my code was performing its operations on copies on the game object,s not the real deals
## Debugging
Just kept running simulations until it threw an error, then re-ran that seed and manually traced where things went wrong
Several bugs with individual players didn't manifest until I tried multiple players
