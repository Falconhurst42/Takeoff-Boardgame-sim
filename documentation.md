## What the program is meant to do:
> My family owns and enjoys a board game called "Take-off" in which players move a fleet of planes across a map of the world. However, whenever we play we get into an argument about how to play. We each want to impliment slightly more or less punishing rules to make the game a more managable length. My project attempts to put those arguments to rest by simulating many Take-off games (which are themselves a simulation of real planes) under different rule-sets and comparing the results

## What the program will need to do:
> To simulate a game the program will need
> - a `Game` object to store and handle the map (hash table), deck (queue), dice (function), and players (objects) as well and compiling the benchmarking data
> - `Airport` structures to populate the map as spaces, track the connections between spaces, and track the location of planes
> - `Player` objects to store and handle `Airplane` objects as gameplay pieces. On their turn, the `Game` will tell a `Player` which actions they are allowed to take and a player must consider all the possibilites, choose the best one, and send those directions to the planes
> - `Airplane` objects to serve as piece which should handle their movements. `Players` will send Airplanes `Actions` which the Airplane must parse and execute: locating their destination and updating the location-tracking of their current (soon to be former) `Airport`, their destination `Airport`, and their own internal locaion tracking
> - Additional utlity structures such as `Color` and `Action` to hold common groupings of data and/or data designated for a particular role
> 
> A Game will require:
> - Game setup
> - A main gameplay loop
> - Game ending
> 
> Game setup will require:
> - generating all `Players`, `Airplanes`, `Airports`, and connections between `Airports`, as well as initializing the map and deck(s)
> 
> Game ending will involve:
> - Variable cleanup(if necessary)
> - Outputting benchmark data
> 
> The main gameplay loop will involve:
> - Iterating through rounds
> - Iterating through `player's` turns
> - Rolling for `players` and sending them actions to use that turn
> - Keeping track of when `players` or the `game` are finished
> - Collecting benchmark data
> 
> Player's turns will involve
> - Simulating all possible movements with given `actions` and `airplanes`
> - Evaluating which set of movements is the best
> - Executing those movements by sending `actions` to the `airplanes`
> - Handling bumping other `player's` `airplanes`
> - Detecting when one or all of their `airplanes` have finished
> - Collecting benchmark data

Clear overview of the simulation and how all of the different elements work.

A detailed report and analysis of your findings.

Detailed use case and manual for how to run your project.

Works Cited. This includes any third party libraries used, any online sources and any code utilized that was not specifically written for this project. These citations should be mirrored in your code.

## Referenced:
> "Print all possible strings of length k that can be formed from a set of n characters" from geeksforgeeks.com (https://www.geeksforgeeks.org/print-all-combinations-of-given-length/) when developing permutations algorithms
> 
> zneak's answer from "Template container iterators" on StackOverflow (https://stackoverflow.com/questions/30018517/template-container-iterators) when developing permutations algorithms
> 
> "Goto stament in C/C++" from geeksforgeeks.com (https://www.geeksforgeeks.org/goto-statement-in-c-cpp/) when developing my decision-making algorithm
> 
> John Dibling's answer to "C++ new operator scope" on StackOverflow (https://stackoverflow.com/questions/23508156/c-new-operator-scope) when troubleshooting memory scoping
> 
> kilojoules' answer to " “X does not name a type” error in C++ " (https://stackoverflow.com/questions/2133250/x-does-not-name-a-type-error-in-c/2133260) when troubleshooting compile errors
> 
> referenced Kerrek SB's answer to "Read file line by line using ifstream in C++" on Stack Overflow (https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c) for his method of reading through a text file
> 
> General reference: http://www.cplusplus.com/reference/algorithm/, http://www.cplusplus.com/reference/vector/vector/, http://www.cplusplus.com/reference/map/map/, http://www.cplusplus.com/reference/cctype/tolower/, http://www.cplusplus.com/reference/queue/queue/, http://www.cplusplus.com/reference/set/set/set/, http://www.cplusplus.com/doc/tutorial/exceptions/