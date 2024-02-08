#include "Tile.h"


bool Tile::getWall(int dir) {
	if (dir == 0) {
		return upWall;
	}
	if (dir == 1) {
		return downWall;
	}
	if (dir == 2) {
		return leftWall;
	}
	if (dir == 3) {
		return rightWall;
	}
}
