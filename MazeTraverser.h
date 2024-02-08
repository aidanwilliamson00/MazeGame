#pragma once

#include <list>
#include <vector>

#include "Tile.h"

using namespace std;

class MazeTraverser {
private:
	int relativeDir = 0;//0 up, 1 down, 2 left, 3 right

	void addMoveToList(int dir, list<pair<pair<int, int>, int>>& moveList);
public:
	list<pair<pair<int, int>, bool[4]>> moveStack;

	pair<int, int> makeMove(vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet);
	bool stackHasTile(pair<int, int> tile);
	list<pair<pair<int, int>, bool[4]>>::iterator stackIterForTile(pair<int, int> tile);
	void newStart(pair<int, int> startingSpot, vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet);
};