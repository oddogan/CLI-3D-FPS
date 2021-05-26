# CLI-3D-FPS
Render a room and walk in it in 3D in Linux Terminal. Remember Wolfenstein!

Before starting playing, you need to install ncurses library:
> sudo apt install libncurses5-dev<br>
> sudo apt install libncursesw5-dev

Remember to include them while compiling:
> g++ -Wall -o Game Game.cpp -lncursesw

Lastly, set the terminal window to the game's determined size in source code.
Default: 120px x 80px