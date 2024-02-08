
#include <vector>

#include "Tile.h"
#include "MazeTraverser.h"
#include "Cat.h"

pair<int, int> Cat::followTrail(vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet) {

	pair<int, int> returnValue = make_pair(-1, -1);
	int trailValue = 0;

	if (!tileSet[maze[moveStack.front().first.first][moveStack.front().first.second].first].upWall) {
		if (maze[moveStack.front().first.first][moveStack.front().first.second+1].second > trailValue) {
			returnValue.first = moveStack.front().first.first;
			returnValue.second = moveStack.front().first.second + 1;
			trailValue = maze[moveStack.front().first.first][moveStack.front().first.second + 1].second;
		}
	}
	if (!tileSet[maze[moveStack.front().first.first][moveStack.front().first.second].first].downWall) {
		if (maze[moveStack.front().first.first][moveStack.front().first.second - 1].second > trailValue) {
			returnValue.first = moveStack.front().first.first;
			returnValue.second = moveStack.front().first.second - 1;
			trailValue = maze[moveStack.front().first.first][moveStack.front().first.second - 1].second;
		}
	}
	if (!tileSet[maze[moveStack.front().first.first][moveStack.front().first.second].first].leftWall) {
		if (maze[moveStack.front().first.first - 1][moveStack.front().first.second].second > trailValue) {
			returnValue.first = moveStack.front().first.first - 1;
			returnValue.second = moveStack.front().first.second;
			trailValue = maze[moveStack.front().first.first - 1][moveStack.front().first.second].second;
		}
	}
	if (!tileSet[maze[moveStack.front().first.first][moveStack.front().first.second].first].rightWall) {
		if (maze[moveStack.front().first.first + 1][moveStack.front().first.second].second > trailValue) {
			returnValue.first = moveStack.front().first.first + 1;
			returnValue.second = moveStack.front().first.second;
			trailValue = maze[moveStack.front().first.first + 1][moveStack.front().first.second].second;
		}
	}

	return returnValue;
	
}

