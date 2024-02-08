#pragma once

struct Tile {
	bool upWall, downWall, leftWall, rightWall;
	int  upInteraction, downInteraction, leftInteraction, rightInteraction;
	int numOpenEdges;
	bool getWall(int dir);
};