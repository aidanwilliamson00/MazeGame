#pragma once

#include <vector>

#include "Tile.h"
#include "MazeTraverser.h"

class Cat :public MazeTraverser {
public:
	
	pair<int, int> followTrail(vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet);

};