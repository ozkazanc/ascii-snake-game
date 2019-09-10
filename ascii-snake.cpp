// ascii-snake.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <chrono>
#include <Windows.h>

int nScreenWidth = 120;			// Console Screen Size X (columns)
int nScreenHeight = 30;			// Console Screen Size Y (rows)
std::list<int> snake;
const int field_width = 120;
const int field_height = 24;
wchar_t field[field_height*field_width];
int nCurrentX = 0;
int nCurrentY = 0;
enum Direction { NORTH, WEST, SOUTH, EAST };

void InitField();
void InitSnake();
void MoveSnake(Direction dir);
void GrowSnake(Direction dir);
bool InBounds(int curX, int curY);
int SpawnFood();

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	swprintf_s(&screen[2 * nScreenWidth + nScreenWidth - 28], 28, L"--A game by: A. D. Ozkazanc");

	swprintf_s(&screen[(nScreenHeight - 2) * nScreenWidth + nScreenWidth - 19], 19, L"Press 'q' to quit.");

	InitField();
	
	InitSnake();

	//Variables
	int bKey[5];
	int score = 0;
	bool bGameOver = false;

	Direction dir = WEST;

	bool bNewFood = true;
	int nFoodIndex = 0;
	int nFoodCount = 0;
	const int nFoodValue = 10;

	bool bTimedFoodActive = false;
	int nTimedFoodIndex = 0;
	const int nTimedFoodValue = 100;
	const int nActiveTime = 10;
	const int nFoodInterval = 5;
	double nCountdown = nActiveTime;
	
	while (!bGameOver) {
		//Timing
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small Step = 1 Game Tick
		
		//Input
		for (int k = 0; k < 5; k++)								 // R   L   D   U Q
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26Q"[k]))) != 0;

		//Game Logic
		//Press Q to quit
		if (bKey[4]) bGameOver = true;

		// Handle player movement
		// Check arrow key presses and make sure that the snake cannot turn into itself
		// only register 1 key press at a time
		if		(bKey[0] && dir != WEST)	dir = EAST;
		else if (bKey[1] && dir != EAST)	dir = WEST;
		else if (bKey[2] && dir != NORTH)	dir = SOUTH;
		else if (bKey[3] && dir != SOUTH)	dir = NORTH;

		//Decrement the timed food countdown if there is a timed food
		if (bTimedFoodActive) {
			nCountdown -= 0.1;
			if (nCountdown <= 0) {
				bTimedFoodActive = false;
				swprintf_s(&screen[nTimedFoodIndex + 4 * nScreenWidth], 2, L" ");
			}
		}
	
		//Spawn food
		if (bNewFood) {
			nFoodIndex = SpawnFood();
			nFoodCount++;
			bNewFood = false;
			
			//Spawn a higher valued timed food  in every 5 foods
			if (nFoodCount % nFoodInterval == 0) {
				do {
					nTimedFoodIndex = SpawnFood();
				} while (nTimedFoodIndex == nFoodIndex);
				//do not spawn a timed food on top of an existing food

				bTimedFoodActive = true;
				nCountdown = nActiveTime;
			}
		}

		MoveSnake(dir);
			
		if (!InBounds(nCurrentX,nCurrentY)) bGameOver = true;
		
		//Snake eats the food
		if (snake.front() == nFoodIndex) {
			score += nFoodValue;
			GrowSnake(dir);

			bNewFood = true;
			swprintf_s(&screen[nFoodIndex + 4 * nScreenWidth], 2, L" ");
		}
		//Snake eats the timed food
		if (bTimedFoodActive && snake.front() == nTimedFoodIndex) {
			score += nTimedFoodValue;
			GrowSnake(dir);

			bTimedFoodActive = false;
			swprintf_s(&screen[nTimedFoodIndex + 4 * nScreenWidth], 2, L" ");
		}
		//Does the snake interrsect with itself?
		for (auto it = snake.begin(); it != snake.end(); it++)
			if (it != snake.begin() && *it == snake.front())
				bGameOver = true;
		

		//Display
		//Draw the field
		for (int i = 0; i < field_height; i++) {
			for (int j = 0; j < field_width; j++)
				screen[(4 + i) * nScreenWidth + j] = field[i * field_width + j];
		}
	
		//Draw the food
		swprintf_s(&screen[nFoodIndex + 4 * nScreenWidth], 2, L"$");

		//Draw the snake
		for (auto it = snake.begin(); it != snake.end(); it++) {
			if(it == snake.begin()) screen[*it + 4 * nScreenWidth] = L'@';
			else screen[*it + 4 * nScreenWidth] = L'*';
		}
		//Draw the score
		swprintf_s(&screen[2 * nScreenWidth], 16, L"SCORE: %8d", score);
		
		//Drwa the snake length
		swprintf_s(&screen[2 * nScreenWidth + 20], 23, L"Snake Length: %8d", snake.size());

		//Draw the timer for the timed food
		if (bTimedFoodActive) {
			
			wsprintf(&screen[2 * nScreenWidth + 45], L"Timer: %2d.%d",(int)nCountdown,(unsigned int)(fmod(nCountdown,1)*10));

			screen[nTimedFoodIndex + 4 * nScreenWidth] = L'%';

		}
		else {
			wsprintf(&screen[2 * nScreenWidth + 45], L"               ");
		}

		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);	
	}
	CloseHandle(hConsole);
	std::cout << "Game Over, Score:" << score << std::endl;
	std::cout << "Thanks for playing!" << std::endl;
	std::cin.get();
	return 0;
}
void InitField() {
	for (int i = 0; i < field_height; i++) {
		for (int j = 0; j < field_width; j++)
			field[i * field_width + j] = (j == 0 || j == field_width - 1 || i == 0 || i == field_height - 1) ? L'#' : L' ';
	}
}
void InitSnake() {
	//Initialize snake at the middle of the board
	nCurrentX = field_width / 2;
	nCurrentY = field_height / 2;
	int nStartIndex = nCurrentY * field_width + nCurrentX;
	snake.push_back(nStartIndex);
	snake.push_back(nStartIndex + 1);
	snake.push_back(nStartIndex + 2);
	snake.push_back(nStartIndex + 3);
}
int SpawnFood() {
	int index = 0;
	do {
		//do not want to spawn food on the walls
		int nFoodX = (rand() % (field_width - 2)) + 1;
		int nFoodY = (rand() % (field_height - 2)) + 1;
		index = nFoodY * field_width + nFoodX;

	} while (std::find(snake.begin(), snake.end(), index) != snake.end()); //do not spawn the food on top of snake's body

	return index;
}
void MoveSnake(Direction dir) {
	int nNextIndex = snake.front();
	
	switch (dir) {
	case NORTH:
		nNextIndex -= field_width;
		nCurrentY--;
		break;
	case WEST:
		nNextIndex -= 1;
		nCurrentX--;
		break;
	case SOUTH:
		nNextIndex += field_width;
		nCurrentY++;
		break;
	case EAST:
		nNextIndex += 1;
		nCurrentX++;
	}
	snake.push_front(nNextIndex);
	snake.pop_back();
}
bool InBounds(int curX, int curY){
	return curX > 0 && curY > 0 && curX < field_width - 1 && curY < field_height - 1;
}
void GrowSnake(Direction dir) {
	int nNewTail = snake.back();
	switch (dir) {
	case NORTH:
		nNewTail += field_width;
		break;
	case WEST:
		nNewTail += 1;		
		break;
	case SOUTH:
		nNewTail -= field_width;		
		break;
	case EAST:
		nNewTail -= 1;
	}
	snake.push_back(nNewTail);
}


