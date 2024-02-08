
#include <list>
#include <vector>
#include <iostream>

#include "Tile.h"
#include "MazeTraverser.h"

using namespace std;


pair<int, int> MazeTraverser::makeMove(vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet) {
	if (moveStack.size() > 0) {

		//remove from back if no paths
		bool removing = true;
		while (removing) {
			bool shouldRemove = true;
			for (bool wall : moveStack.back().second) {
				if (!wall) { shouldRemove = false; }
			}
			if (shouldRemove) {
				moveStack.pop_back();
				if (moveStack.size() == 0) {
					return make_pair(-1, -1);
				}
			}
			else {
				removing = false;
				break;
			}
		}

		//determine if a move can be made
		bool needNewMove = false;
		pair<int, int> returnPair;

		for (bool possibleDir : moveStack.front().second) {
			if (!possibleDir) { needNewMove = true; }
		}

		if (!needNewMove) {//no possible moves so go back
			moveStack.pop_front();
			return  moveStack.front().first;
		}
		else {//move is possible
			bool isMoveChosen = false;
			list<pair<pair<int, int>,int>> moveList;
			while (!isMoveChosen) {
				pair<int, int> newMove = make_pair(-1, -1);

				if (relativeDir == 0) {//facing up
					if (!moveStack.front().second[2]) {
						addMoveToList(2,moveList);
					}
					if (!moveStack.front().second[0]) {
						addMoveToList(0, moveList);
					}
					if (!moveStack.front().second[3]) {
						addMoveToList(3, moveList);
					}
					if (!moveStack.front().second[1]) {
						addMoveToList(1, moveList);
					}

				}
				else if (relativeDir == 1) {//facing down
					if (!moveStack.front().second[3]) {
						addMoveToList(3, moveList);
					}
					if (!moveStack.front().second[1]) {
						addMoveToList(1, moveList);
					}
					if (!moveStack.front().second[2]) {
						addMoveToList(2, moveList);
					}
					if (!moveStack.front().second[0]) {
						addMoveToList(0, moveList);
					}

				}
				else if (relativeDir == 2) {//facing left
					if (!moveStack.front().second[1]) {
						addMoveToList(1, moveList);
					}
					if (!moveStack.front().second[2]) {
						addMoveToList(2, moveList);
					}
					if (!moveStack.front().second[0]) {
						addMoveToList(0, moveList);
					}
					if (!moveStack.front().second[3]) {
						addMoveToList(3, moveList);
					}

				}
				else if (relativeDir == 3) {//facing right
					if (!moveStack.front().second[0]) {
						addMoveToList(0, moveList);
					}
					if (!moveStack.front().second[3]) {
						addMoveToList(3, moveList);
					}
					if (!moveStack.front().second[1]) {
						addMoveToList(1, moveList);
					}
					if (!moveStack.front().second[2]) {
						addMoveToList(2, moveList);
					}

				}


				//check if surrounding tiles are in the stack already
				list<pair<pair<int, int>, int>> newMoveList;
				for (list<pair<pair<int, int>, int>>::iterator moveIter = moveList.begin(); moveIter != moveList.end(); moveIter++) {

					if (stackHasTile(moveIter->first)) {

						list<pair<pair<int, int>, bool[4]>>::iterator iter = stackIterForTile(moveIter->first);
						
						//close path
						if (moveIter->second == 0) { iter->second[1] = true; moveStack.front().second[0] = true; }
						else if (moveIter->second == 1) { iter->second[0] = true; moveStack.front().second[1] = true; }
						else if (moveIter->second == 2) { iter->second[3] = true; moveStack.front().second[2] = true; }
						else if (moveIter->second == 3) { iter->second[2] = true; moveStack.front().second[3] = true; }

						//check if the tiles between this one and the found adjacent tile are all out of paths
						bool noMorePossibleBetween = true;

						for (list<pair<pair<int, int>, bool[4]>>::iterator remIter = moveStack.begin(); remIter != iter; remIter++) {
							for (bool wall : remIter->second) {
								if (!wall) { noMorePossibleBetween = false; }
							}
						}

						// remove the path
						if (noMorePossibleBetween) {
							pair<pair<int, int>, bool[4]> frontCopy = moveStack.front();
							while (moveStack.begin() != iter) {
								moveStack.pop_front();
							}
							moveStack.push_front(frontCopy);
						}
					}
					else {
						newMoveList.push_back(*moveIter);
					}
				}
				
				
				if (newMoveList.size()>0) {
					pair<int, int> tileToAdd = newMoveList.front().first;
					relativeDir = newMoveList.front().second;
					pair<pair<int, int>, bool[4]> itemToPush;

					itemToPush.first = tileToAdd;
					for (int i = 0; i < 4; i++) {
						itemToPush.second[i] = tileSet[maze[tileToAdd.first][tileToAdd.second].first].getWall(i);
					}

					if (relativeDir == 0) { moveStack.front().second[0] = true; }
					else if (relativeDir == 1) { moveStack.front().second[1] = true; }
					else if (relativeDir == 2) { moveStack.front().second[2] = true; }
					else if (relativeDir == 3) { moveStack.front().second[3] = true; }

					moveStack.push_front(itemToPush);

					if (relativeDir == 0) { moveStack.front().second[1] = true; }
					else if (relativeDir == 1) { moveStack.front().second[0] = true; }
					else if (relativeDir == 2) { moveStack.front().second[3] = true; }
					else if (relativeDir == 3) { moveStack.front().second[2] = true; }

					return tileToAdd;

				}
				else { //no possible move so go back
					moveStack.pop_front();
					return  moveStack.front().first;
				}
			}
		}
	}
	else {
		//stack empty, maze has been traversed
		return make_pair(-1, -1);
	}
}

bool MazeTraverser::stackHasTile(pair<int, int> tile) {
	for (list<pair<pair<int, int>, bool[4]>>::iterator iter = moveStack.begin(); iter != moveStack.end(); iter++) {
		if (iter->first.first == tile.first && iter->first.second == tile.second) {
			return true;
		}
	}
	return false;
}

list<pair<pair<int, int>, bool[4]>>::iterator MazeTraverser::stackIterForTile(pair<int, int> tile) {
	for (list<pair<pair<int, int>, bool[4]>>::iterator iter = moveStack.begin(); iter != moveStack.end(); iter++) {
		if (iter->first.first == tile.first && iter->first.second == tile.second) {
			return iter;
		}
	}
}

void MazeTraverser::newStart(pair<int, int> startingSpot, vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet) {
	relativeDir = 0;
	pair<pair<int, int>, bool[4]> itemToPush;

	itemToPush.first = startingSpot;
	for (int i = 0; i < 4; i++) {
		itemToPush.second[i] = tileSet[maze[startingSpot.first][startingSpot.second].first].getWall(i);
	}

	moveStack.push_front(itemToPush);
}

void MazeTraverser::addMoveToList(int dir, list<pair<pair<int, int>, int>>& moveList) {
	pair<int, int> newMove;
	if (dir == 0) {
		newMove.first = moveStack.front().first.first;
		newMove.second = moveStack.front().first.second + 1;
		moveList.push_back(make_pair(newMove, dir));
	}
	else if (dir == 1) {
		newMove.first = moveStack.front().first.first;
		newMove.second = moveStack.front().first.second - 1;
		moveList.push_back(make_pair(newMove, dir));
	}
	else if (dir == 2) {
		newMove.first = moveStack.front().first.first - 1;
		newMove.second = moveStack.front().first.second;
		moveList.push_back(make_pair(newMove, dir));
	}
	else if (dir == 3) {
		newMove.first = moveStack.front().first.first + 1;
		newMove.second = moveStack.front().first.second;
		moveList.push_back(make_pair(newMove, dir));
	}
}