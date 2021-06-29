towermac: main.o simulation.o bullet.o game_def.o 
	c++ -std=c++17 simulation.o bullet.o main.o game_def.o -o towermac -lSDL2 -lSDL2_image

main.o: main.cpp bullet.hpp  core.hpp  mob.hpp  path.hpp  simulation.hpp  sprite.hpp tower.hpp game_def.hpp
	c++ -std=c++17 -c main.cpp -o main.o

simulation.o: simulation.cpp bullet.hpp  core.hpp  mob.hpp  path.hpp  simulation.hpp sprite.hpp tower.hpp game_def.hpp
	c++ -std=c++17 -c simulation.cpp -o simulation.o

bullet.o: bullet.cpp bullet.hpp  core.hpp  mob.hpp  path.hpp  simulation.hpp  sprite.hpp tower.hpp game_def.hpp
	c++ -std=c++17 -c bullet.cpp -o bullet.o

game_def.o: game_def.cpp bullet.hpp  core.hpp  mob.hpp  path.hpp simulation.hpp sprite.hpp tower.hpp game_def.hpp
	c++ -std=c++17 -c game_def.cpp -o game_def.o 

debug:	main.cpp
	c++ -std=c++17 -g -c main.cpp -o main.o
	c++ -std=c++17 -g -c simulation.cpp -o simulation.o
	c++ -std=c++17 -g -c bullet.cpp -o bullet.o
	c++ -std=c++17 -g -c game_def.cpp -o game_def.o
	c++ -std=c++17 -g main.o simulation.o bullet.o game_def.o -o towermac -lSDL2 -lSDL2_image

clean:
	rm -f main.o simulation.o bullet.o game_def.o towermac
