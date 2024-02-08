
#include <iostream>
#include <utility>
#include <algorithm>
#include <vector>
#include <queue>
#include <random>
#include <list>
#include <string>

#include "Tile.h"
#include "Mouse.h"
#include "Cat.h"

#include "SFML/Graphics.hpp"

using namespace std;

//	Global Vars
const int cheeseAmountToGather = 2;//Amount of cheese the mouse has to gather to win
const int gridSize = 10;//size of maze
const int windowSize = 600;//size of window
const float cellSize = windowSize / gridSize;//Size of individual cells
vector<Tile> tileSet(14);//set of all tiles
vector<vector<int>> interactSet(8, vector<int>());//set of tile interactions

//	Functions
void setTiles(vector<Tile>& tiles);

void interactDefine(vector<vector<int>>& interactMap);

void initalizeCollapse(vector<vector<list<int>>>& stateMap);

void waveCollapse(vector<vector<list<int>>>& stateMap);

bool inBounds(int x, int y);

bool listContains(list<pair<int, int>> inList, pair<int, int> spot);

bool listContains(list<int> inList, int spot);

void windowRenderInitialization(sf::RenderTexture& windowTexture, vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet);

void windowTrailDraw(sf::RenderWindow& window, vector<vector<pair<int, int>>>& maze);

//	Main
int main() {

	//create tile set
	setTiles(tileSet);

	//define interactions between tiles
	interactDefine(interactSet);

	//define grid to collapse
	vector<vector<list<int>>> waveGrid(gridSize, vector<list<int>>(gridSize));

	//initalize values of walls, corners, and normal tiles
	initalizeCollapse(waveGrid);

	//wave function collapse
	waveCollapse(waveGrid);

	//Create Maze from collapsed wave grid
	vector<vector<pair<int, int>>> maze(gridSize, vector<pair<int, int>>(gridSize));

	for (int x = 0; x < gridSize; x++) {
		for (int y = 0; y < gridSize; y++) {
			if (!waveGrid[x][y].empty()) {
				maze[x][y].first = waveGrid[x][y].front();
			}
			else {
				maze[x][y].first = -1;
			}
		}
	}

	//random number gen
	random_device rd;
	mt19937 randVal(rd());

	//Create Mouse actor with starting position
	Mouse mouseActor;
	mouseActor.newStart(make_pair(0, 0), maze, tileSet);
	maze[0][0].second = mouseActor.trailAmount;

	sf::CircleShape mouseCircle(cellSize / 4);
	mouseCircle.setFillColor(sf::Color::Green);
	mouseCircle.setPosition(cellSize/4,windowSize - (3*cellSize)/4);

	//Create Cat actor with starting position
	Cat catActor;
	catActor.newStart(make_pair(randVal() % gridSize, randVal() % gridSize), maze, tileSet);

	sf::CircleShape catCircle(cellSize / 4);
	catCircle.setFillColor(sf::Color::Magenta);
	catCircle.setPosition((cellSize / 4) + cellSize * catActor.moveStack.front().first.first, windowSize - ((3 * cellSize) / 4) - cellSize * catActor.moveStack.front().first.second);

	//Create cheese for mouse to find
	pair<int, int> cheesePosition = make_pair(randVal() % gridSize, randVal() % gridSize);
	sf::CircleShape cheeseCircle(cellSize / 4);
	cheeseCircle.setFillColor(sf::Color::Yellow);
	cheeseCircle.setPosition((cellSize / 4) + cellSize * cheesePosition.first, windowSize - ((3 * cellSize) / 4) - cellSize * cheesePosition.second);

	//Create render texture from maze so that we dont have to render every wall every loop
	sf::RenderTexture mazeRender;
	mazeRender.create(windowSize, windowSize);
	windowRenderInitialization(mazeRender,maze,tileSet);
	sf::RenderWindow window(sf::VideoMode(windowSize, windowSize), "Maze Visualization");
	sf::Sprite mazeImage;
	mazeImage.setTexture(mazeRender.getTexture());
	
	
	int cheeseGathered = 0;
	bool mouseOutOfMoves = false;
	bool mouseWins = false;
	bool catCaughtMouse = false;

	//window interactions
	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonReleased) {
				
				//Decrement all existing trails
				for (int x = 0; x < gridSize; x++) {
					for (int y = 0; y < gridSize; y++) {
						if (maze[x][y].second > 0) { maze[x][y].second--; }
					}
				}
				//record prev move
				pair<pair<int, int>, bool[4]> mousePrevMove = mouseActor.moveStack.front();
				pair<pair<int, int>, bool[4]> catPrevMove = catActor.moveStack.front();

				//Get new mouse move
				pair<int, int> mouseNewCoords;
				mouseNewCoords = mouseActor.makeMove(maze, tileSet);

				if (mouseNewCoords.first == -1 && mouseNewCoords.second == -1) {
					//mouse could not find cheese
					mouseOutOfMoves = true;
				}
				else {
					//update mouse position
					mouseCircle.setPosition((cellSize / 4) + cellSize * mouseNewCoords.first, windowSize - ((3 * cellSize) / 4) - cellSize * mouseNewCoords.second);

					//Check if cheese overlap
					if (mouseNewCoords.first == cheesePosition.first && mouseNewCoords.second == cheesePosition.second) {
						mouseActor.trailAmount = 0;
						cheesePosition = make_pair(randVal() % gridSize, randVal() % gridSize);
						cheeseCircle.setPosition((cellSize / 4) + cellSize * cheesePosition.first, windowSize - ((3 * cellSize) / 4) - cellSize * cheesePosition.second);
						mouseActor.moveStack.clear();
						mouseActor.newStart(make_pair(mouseNewCoords.first, mouseNewCoords.second), maze, tileSet);
						cheeseGathered++;
					}

					//Set trail if needed
					if (mouseActor.trailAmount == 10) {
						maze[mouseNewCoords.first][mouseNewCoords.second].second = mouseActor.trailAmount;
					}
					else { mouseActor.trailAmount++; }
				}
				//get new cat move
				pair<int, int> catNewCoords;
				catNewCoords = catActor.followTrail(maze, tileSet);

				//if theres no trail, make normal move
				if (catNewCoords.first == -1 && catNewCoords.second == -1) {
					catNewCoords = catActor.makeMove(maze, tileSet);
					if (catNewCoords.first == -1 && catNewCoords.second == -1) {//if there is no more moves, reset stack
						catActor.moveStack.clear();
						catActor.moveStack.push_back(catPrevMove);
						catNewCoords = catActor.makeMove(maze, tileSet);
					}
				}
				else {//follow trail
					catActor.moveStack.clear();
					pair<pair<int, int>, bool[4]> itemToPush;

					itemToPush.first = catNewCoords;
					for (int i = 0; i < 4; i++) {
						itemToPush.second[i] = tileSet[maze[catNewCoords.first][catNewCoords.second].first].getWall(i);
					}
					catActor.moveStack.push_back(itemToPush);
				}
				//update cat position
				catCircle.setPosition((cellSize / 4) + cellSize * catActor.moveStack.front().first.first, windowSize - ((3 * cellSize) / 4) - cellSize * catActor.moveStack.front().first.second);

				//check if mouse and cat are overlapped or have switched positions
				if (mouseActor.moveStack.front().first.first == catActor.moveStack.front().first.first && mouseActor.moveStack.front().first.second == catActor.moveStack.front().first.second) {
					catCaughtMouse = true;
				}
				else if (mouseActor.moveStack.front().first.first == catPrevMove.first.first && mouseActor.moveStack.front().first.second == catPrevMove.first.second) {
					catCaughtMouse = true;
				}

			}
		}
		
		window.clear();
		//Start loop drawing
		window.draw(mazeImage);

		windowTrailDraw(window, maze);

		window.draw(cheeseCircle);

		window.draw(mouseCircle);

		window.draw(catCircle);
		//End loop drawing
		window.display();
		
			
		//game over check
		if (cheeseGathered >= cheeseAmountToGather) {
			mouseWins = true;
			break;
		}
		else if (mouseOutOfMoves) {
			break;
		}
		else if (catCaughtMouse) {
			break;
		}
	}

	//Game endings
	if (mouseWins) {
		cout << "Mouse wins!" << endl;
	}

	if (catCaughtMouse) {
		cout << "Cat wins!" << endl;
	}

	if (mouseOutOfMoves) {
		cout << "Mouse ran out of moves" << endl;
	}

	return 0;
}

void setTiles(vector<Tile>& tiles) {
	struct Tile t0 = { false,true,true,false,0,3,5,6,2 };
	tiles[0] = t0;
	struct Tile t1 = { false,true,false,true,0,3,4,7,2 };
	tiles[1] = t1;
	struct Tile t2 = { true,false,false,true,1,2,4,7,2 };
	tiles[2] = t2;
	struct Tile t3 = { true,false,true,false,1,2,5,6,2 };
	tiles[3] = t3;

	struct Tile t4 = { true,true,false,false,1,3,4,6,2 };
	tiles[4] = t4;
	struct Tile t5 = { false,false,true,true,0,2,5,7,2 };
	tiles[5] = t5;

	struct Tile t6 = { false,true,true,true,0,3,5,7,1 };
	tiles[6] = t6;
	struct Tile t7 = { true,true,false,true,1,3,4,7,1 };
	tiles[7] = t7;
	struct Tile t8 = { true,false,true,true,1,2,5,7,1 };
	tiles[8] = t8;
	struct Tile t9 = { true,true,true,false,1,3,5,6,1 };
	tiles[9] = t9;
	///*
	struct Tile t10 = { true,false,false,false,1,2,4,6,3 };
	tiles[10] = t10;
	struct Tile t11 = { false,true,false,false,0,3,4,6,3 };
	tiles[11] = t11;
	struct Tile t12 = { false,false,true,false,0,2,5,6,3 };
	tiles[12] = t12;
	struct Tile t13 = { false,false,false,true,0,2,4,7,3 };
	tiles[13] = t13;
	//*/
}

void interactDefine(vector<vector<int>>& interactMap) {
	//up open
	interactMap[0].assign({ 2,3,5,8,10,12,13});
	//up closed
	interactMap[1].assign({ 0,1,4,6,7,9,11});
	//down open
	interactMap[2].assign({ 0,1,5,6,11,12,13});
	//down closed
	interactMap[3].assign({ 2,3,4,7,8,9,10});
	//left open
	interactMap[4].assign({ 0,3,4,9,10,11,12});
	//left closed
	interactMap[5].assign({ 1,2,5,6,7,8,13});
	//right open
	interactMap[6].assign({ 1,2,4,7,10,11,13});
	//right closed
	interactMap[7].assign({ 0,3,5,6,8,9,12});
}

void initalizeCollapse(vector<vector<list<int>>>& stateMap) {
	for (int x = 0; x < gridSize; x++) {
		for (int y = 0; y < gridSize; y++) {
			if (x == 0) {//left
				if (y == 0) {//bottom left corner
					stateMap[x][y].assign({ 0,6,9 });
				}
				else if (y == gridSize - 1) {//top left corner
					stateMap[x][y].assign({ 3,8,9 });
				}
				else {//left wall
					stateMap[x][y].assign({ 0,3,5,6,8,9,12 });
				}
			}
			else if (y == 0) {//bottom
				if (x == gridSize - 1) {//bottom right corner
					stateMap[x][y].assign({ 1,6,7 });
				}
				else {//bottom wall
					stateMap[x][y].assign({ 0,1,4,6,7,9,11 });
				}
			}
			else if (y == gridSize - 1) {//top
				if (x == gridSize - 1) {//top right corner
					stateMap[x][y].assign({ 2,7,8 });
				}
				else {//top wall
					stateMap[x][y].assign({ 2,3,4,7,8,9,10 });
				}
			}
			else if (x == gridSize - 1) {//right wall
				stateMap[x][y].assign({ 1,2,5,6,7,8,13 });
			}
			else {
				stateMap[x][y].assign({ 0,1,2,3,4,5,6,7,8,9,10,11,12,13});
			}
		}
	}
}

void waveCollapse(vector<vector<list<int>>>& stateMap) {
	pair<int, int> nextPair;
	queue<pair<int, int>> updateQueue;
	
	list<pair<int, int>> tileList;
	tileList.push_back(make_pair(0, 0));

	//random generator
	random_device rd;
	mt19937 randVal(rd());
	

	while (!tileList.empty()) {

		nextPair = tileList.back();
		tileList.pop_back();

		//take point and choose tile
		//implement priority
		int numOpen = 0;
		int numUndecided = 0;
		int numOuterWall = 0;
		int chosenVal;
		int randIndex;
		vector<int> possibleValues;

		if (stateMap[nextPair.first][nextPair.second].size() == 0) {
			continue;
		}

		if (inBounds(nextPair.first, nextPair.second + 1)) {//up
			if (stateMap[nextPair.first][nextPair.second + 1].size() == 1) {
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].upWall) { numOpen++; }
			} else { numUndecided++; }
		} else { numOuterWall++; }
		if (inBounds(nextPair.first, nextPair.second - 1)) {//down
			if (stateMap[nextPair.first][nextPair.second - 1].size() == 1) {
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].downWall) { numOpen++; }
			} else { numUndecided++; }
		} else { numOuterWall++; }
		if (inBounds(nextPair.first-1, nextPair.second)) {//left
			if (stateMap[nextPair.first-1][nextPair.second].size() == 1) {
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].leftWall) { numOpen++; }
			} else { numUndecided++; }
		} else { numOuterWall++; }
		if (inBounds(nextPair.first+1, nextPair.second)) {//right
			if (stateMap[nextPair.first+1][nextPair.second].size() == 1) {
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].rightWall) { numOpen++; }
			} else { numUndecided++; }
		} else { numOuterWall++; }

		//determine possible tile placements
		for (list<int>::iterator iter = stateMap[nextPair.first][nextPair.second].begin(); iter != stateMap[nextPair.first][nextPair.second].end(); iter++) {
			int possibleValue = *iter;

			if (numUndecided == 0 && tileSet[possibleValue].numOpenEdges == numOpen) { possibleValues.push_back(possibleValue); }
			else if (numUndecided == 1) {
				if (numOpen > 0 && tileSet[possibleValue].numOpenEdges > numOpen) { possibleValues.push_back(possibleValue); }
				else if (numOpen > 0 && tileSet[possibleValue].numOpenEdges == 3) { possibleValues.push_back(possibleValue); }
				else if (numOpen == 0 && tileSet[possibleValue].numOpenEdges == 1) { possibleValues.push_back(possibleValue); }
			}
			else if (numUndecided == 2) {
				if (numOuterWall == 2 && tileSet[possibleValue].numOpenEdges > 0) { possibleValues.push_back(possibleValue); }
				else if (numOuterWall == 1) {
					if (!inBounds(nextPair.first, nextPair.second + 1) || !inBounds(nextPair.first, nextPair.second - 1)) {
						if (stateMap[nextPair.first-1][nextPair.second].size() != 1  && stateMap[nextPair.first+1][nextPair.second].size() != 1) {
							if (tileSet[possibleValue].numOpenEdges > numOpen + 1) { possibleValues.push_back(possibleValue); }
						}
						else if (tileSet[possibleValue].numOpenEdges > numOpen) { possibleValues.push_back(possibleValue); }
					}
					if (!inBounds(nextPair.first - 1, nextPair.second) || !inBounds(nextPair.first + 1, nextPair.second)) {
						if (stateMap[nextPair.first][nextPair.second+1].size() != 1 && stateMap[nextPair.first][nextPair.second-1].size() != 1) {
							if (tileSet[possibleValue].numOpenEdges > numOpen + 1) { possibleValues.push_back(possibleValue); }
						}
						else if (tileSet[possibleValue].numOpenEdges > numOpen) { possibleValues.push_back(possibleValue); }
					}
				}
				else if (numOuterWall == 0 && tileSet[possibleValue].numOpenEdges > numOpen) { possibleValues.push_back(possibleValue); }
			}
			else if (numUndecided == 3 && tileSet[possibleValue].numOpenEdges > numOpen) { possibleValues.push_back(possibleValue); }
		}


		//get random tile from all possible tiles
		if (possibleValues.size() > 0) {
			chosenVal = possibleValues[randVal() % possibleValues.size()];
		}
		else {
			cout << "no possible values" << endl;
			stateMap[nextPair.first][nextPair.second].assign({});
			continue;
		}
		possibleValues.clear();

		//choose tile
		stateMap[nextPair.first][nextPair.second].assign({ chosenVal });


		//add directions to queue
		pair<int, int> spot;

		if (inBounds(nextPair.first, nextPair.second + 1)) {//up
			if (stateMap[nextPair.first][nextPair.second + 1].size() > 1) {
				spot = make_pair(nextPair.first, nextPair.second + 1);
				updateQueue.push(spot);
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].upWall && !listContains(tileList, spot)) {
					tileList.push_back(spot);
				}
			}
		}
		if (inBounds(nextPair.first, nextPair.second - 1)) {//down
			if (stateMap[nextPair.first][nextPair.second - 1].size() > 1) {
				spot = make_pair(nextPair.first, nextPair.second - 1);
				updateQueue.push(spot);
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].downWall && !listContains(tileList, spot)) {
					tileList.push_back(spot);
				}
			}
		}
		if (inBounds(nextPair.first - 1, nextPair.second)) {//left
			if (stateMap[nextPair.first - 1][nextPair.second].size() > 1) {
				spot = make_pair(nextPair.first - 1, nextPair.second);
				updateQueue.push(spot);
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].leftWall && !listContains(tileList, spot)) {
					tileList.push_back(spot);
				}
			}
		}
		if (inBounds(nextPair.first + 1, nextPair.second)) {//right
			if (stateMap[nextPair.first + 1][nextPair.second].size() > 1) {
				spot = make_pair(nextPair.first + 1, nextPair.second);
				updateQueue.push(spot);
				if (!tileSet[stateMap[nextPair.first][nextPair.second].front()].rightWall && !listContains(tileList, spot)) {
					tileList.push_back(spot);
				}
			}
		}

		//while queue is not empty
		while (!updateQueue.empty()) {

			pair<int, int> popCoords = updateQueue.front();
			updateQueue.pop();

			list<int> upList;
			list<int> downList;
			list<int> leftList;
			list<int> rightList;
			list<int> updownList;
			list<int> leftrightList;
			list<int> values;
			//get lists of interactions from surrounding tiles
			if (inBounds(popCoords.first, popCoords.second + 1)) {//up
				for (int tileNum : stateMap[popCoords.first][popCoords.second + 1]) {
					for (int interaction : interactSet[tileSet[tileNum].downInteraction]) {
						if (!listContains(upList, interaction)) {
							upList.push_back(interaction);
						}
					}
				}
				upList.sort();
			}
			
			if (inBounds(popCoords.first, popCoords.second - 1)) {//down
				for (int tileNum : stateMap[popCoords.first][popCoords.second - 1]) {
					for (int interaction : interactSet[tileSet[tileNum].upInteraction]) {
						if (!listContains(downList, interaction)) {
							downList.push_back(interaction);
						}
					}
				}
				downList.sort();
			}
			
			if (inBounds(popCoords.first - 1, popCoords.second)) {//left
				for (int tileNum : stateMap[popCoords.first - 1][popCoords.second]) {
					for (int interaction : interactSet[tileSet[tileNum].rightInteraction]) {
						if (!listContains(leftList, interaction)) {
							leftList.push_back(interaction);
						}
					}
				}
				leftList.sort();
			}
			
			if (inBounds(popCoords.first + 1, popCoords.second)) {//right
				for (int tileNum : stateMap[popCoords.first + 1][popCoords.second]) {
					for (int interaction : interactSet[tileSet[tileNum].leftInteraction]) {
						if (!listContains(rightList, interaction)) {
							rightList.push_back(interaction);
						}
					}
				}
				rightList.sort();
			}

			//intersect up down
			if (upList.empty()) { updownList = downList; }
			else if (downList.empty()) { updownList = upList; }
			else { set_intersection(upList.begin(), upList.end(), downList.begin(), downList.end(),back_inserter(updownList)); }

			//intersect left right
			if (leftList.empty()) { leftrightList = rightList; }
			else if (rightList.empty()) { leftrightList = leftList; }
			else { set_intersection(leftList.begin(), leftList.end(), rightList.begin(), rightList.end(), back_inserter(leftrightList)); }

			//intersect updown leftright
			set_intersection(updownList.begin(), updownList.end(), leftrightList.begin(), leftrightList.end(), back_inserter(values));


			//determine if tile was updated
			bool didUpdate = false;
			for (list<int>::iterator iter = stateMap[popCoords.first][popCoords.second].begin(); iter != stateMap[popCoords.first][popCoords.second].end(); iter++) {
				bool valFound = false;
				for (int val : values) {
					if (*iter == val) {
						valFound = true;
					}
				}
				if (!valFound) {
					didUpdate = true;
				}
			}
			
			//	if this tile has been updated
			if (didUpdate) {
				//update list of possible values
				list<int> newValues;
				set_intersection(values.begin(), values.end(), stateMap[popCoords.first][popCoords.second].begin(), stateMap[popCoords.first][popCoords.second].end(), back_inserter(newValues));
				stateMap[popCoords.first][popCoords.second] = newValues;

				//if the update of this tile has determined it, add it to the top of the stack
				if (stateMap[popCoords.first][popCoords.second].size() == 1 && !listContains(tileList, make_pair(popCoords.first,popCoords.second))) {
					tileList.push_back(popCoords);
				}
				else if (stateMap[popCoords.first][popCoords.second].size() == 1 && listContains(tileList, make_pair(popCoords.first, popCoords.second))) {
					tileList.remove(make_pair(popCoords.first, popCoords.second));
					tileList.push_back(popCoords);
				}
				
				//update surroundings
				if (inBounds(popCoords.first, popCoords.second + 1)) {//up
					if (stateMap[popCoords.first][popCoords.second + 1].size() > 1) {
						spot = make_pair(popCoords.first, popCoords.second + 1);
						updateQueue.push(spot);
					}
				}
				if (inBounds(popCoords.first, popCoords.second - 1)) {//down
					if (stateMap[popCoords.first][popCoords.second - 1].size() > 1) {
						spot = make_pair(popCoords.first, popCoords.second - 1);
						updateQueue.push(spot);
					}
				}
				if (inBounds(popCoords.first - 1, popCoords.second)) {//left
					if (stateMap[popCoords.first - 1][popCoords.second].size() > 1) {
						spot = make_pair(popCoords.first - 1, popCoords.second);
						updateQueue.push(spot);
					}
				}
				if (inBounds(popCoords.first + 1, popCoords.second)) {//right
					if (stateMap[popCoords.first + 1][popCoords.second].size() > 1) {
						spot = make_pair(popCoords.first + 1, popCoords.second);
						updateQueue.push(spot);
					}
				}
			}
		}
	}
}

bool inBounds(int x, int y) {
	if (x >= 0 && x < gridSize) {
		if (y >= 0 && y < gridSize) {
			return true;
		}
	}
	return false;
}

bool listContains(list<pair<int,int>> inList,pair<int,int> spot) {
	for (list<pair<int, int>>::iterator iter = inList.begin(); iter != inList.end(); iter++) {
		if (iter->first == spot.first && iter->second == spot.second) {
			return true;
		}
	}
	return false;
}

bool listContains(list<int> inList, int spot) {
	for (list<int>::iterator iter = inList.begin(); iter != inList.end(); iter++) {
		if (*iter == spot) {
			return true;
		}
	}
	return false;
}

void windowRenderInitialization(sf::RenderTexture& windowTexture, vector<vector<pair<int, int>>>& maze, vector<Tile> tileSet) {
	const sf::Color wallColor = sf::Color::Blue;
	float wallSize = cellSize * 0.05;
	for (int y = 0; y < gridSize; y++) {
		for (int x = 0; x < gridSize; x++) {
			if (maze[x][y].first >= 0) {
				sf::RectangleShape wallShapes[4];

				wallShapes[0].setSize(sf::Vector2f(cellSize, wallSize)); // Up
				wallShapes[1].setSize(sf::Vector2f(cellSize, wallSize)); // Down
				wallShapes[2].setSize(sf::Vector2f(wallSize, cellSize)); // Left
				wallShapes[3].setSize(sf::Vector2f(wallSize, cellSize)); // Right

				wallShapes[0].setPosition(cellSize * x, (cellSize * (y+1))-wallSize);
				wallShapes[1].setPosition(cellSize * x, (cellSize * (y)));
				wallShapes[2].setPosition(cellSize * x, (cellSize * (y)));
				wallShapes[3].setPosition(cellSize * (x + 1) - wallSize, (cellSize * (y)));

				for (int i = 0; i < 4; ++i) {
					if (tileSet[maze[x][y].first].getWall(i)) {
						wallShapes[i].setFillColor(wallColor);
						windowTexture.draw(wallShapes[i]);
					}
				}
			}
			else {
				sf::RectangleShape undecidedTile(sf::Vector2f(cellSize,cellSize));
				undecidedTile.setPosition(cellSize * x, (cellSize * (y + 1)) - wallSize);
				undecidedTile.setFillColor(sf::Color::Red);
				windowTexture.draw(undecidedTile);
			}
		}
	}
}

void windowTrailDraw(sf::RenderWindow& window, vector<vector<pair<int, int>>>& maze) {
	sf::Color trailColor;
	trailColor.r = 139;
	trailColor.g = 69;
	trailColor.b = 19;
	for (int x = 0; x < gridSize; x++) {
		for (int y = 0; y < gridSize; y++) {
			if (maze[x][y].second > 0) {
				sf::CircleShape trailShape(cellSize / 2);
				trailColor.a = maze[x][y].second * 25;
				trailShape.setFillColor(trailColor);
				trailShape.setPosition(cellSize * x, windowSize-(cellSize* (y+1)));
				window.draw(trailShape);
			}
		}
	}
}