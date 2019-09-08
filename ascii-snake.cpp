// ascii-snake.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <list>

#include <thread>
#include <chrono>
#include <Windows.h>

int nScreenWidth = 120;			// Console Screen Size X (columns)
int nScreenHeight = 30;			// Console Screen Size Y (rows)
std::list<int> snake;
const int field_width = 120;
const int field_height = 24;
wchar_t field[field_height*field_width];
enum Direction { NORTH, WEST, SOUTH, EAST };

void InitField();
void MoveSnake(Direction dir);

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	InitField();
	
	
	//Init snake at the middle of the board
	int nCurrentX = field_width / 2;
	int nCurrentY = field_height / 2;
	int nStartIndex = nCurrentY * field_width + nCurrentX;
	snake.push_back(nStartIndex);
	snake.push_back(nStartIndex + 1);
	snake.push_back(nStartIndex + 2);
	snake.push_back(nStartIndex + 3);

	int bKey[5];
	bool bGameOver = false;
	bool bForceMove;
	int nPlaySpeed = 2;
	int nTickCount = 0;
	Direction dir = WEST;
	
	while (!bGameOver) {
		//Timing
		std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small Step = 1 Game Tick
		nTickCount++;
		bForceMove = nTickCount == nPlaySpeed;
		
		//Input
		for (int k = 0; k < 5; k++)								 // R   L   D   U Q
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26Q"[k]))) != 0;

		//Game Logic
		//Press Q to quit
		if (bKey[4]) break;

		// Handle player movement
		dir = bKey[0] ? EAST : dir;
		dir = bKey[1] ? WEST : dir;
		dir = bKey[2] ? SOUTH : dir;
		dir = bKey[3] ? NORTH : dir;

		//TODO: Make sure that the snake cannot turn into itself
		
		if (bForceMove) {
			nTickCount = 0;
			MoveSnake(dir);
		}

		//Display
		//Draw the field
	
		for (int i = 0; i < field_height; i++) {
			for (int j = 0; j < field_width; j++)
				screen[(4 + i) * nScreenWidth + j] = field[i * field_width + j];
		}
	
		//Draw the snake

		for (auto it = snake.begin(); it != snake.end(); it++) {
			if(it == snake.begin()) screen[*it + 4 * nScreenWidth] = L'@';
			else screen[*it + 4 * nScreenWidth] = L'*';
		}
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	
	}
	CloseHandle(hConsole);
	//std::cout << "Game Over!! Score:" << score << std::endl;
	std::cin.get();
	return 0;
}
void InitField() {
	for (int i = 0; i < field_height; i++) {
		for (int j = 0; j < field_width; j++)
			field[i * field_width + j] = (j == 0 || j == field_width - 1 || i == 0 || i == field_height - 1) ? L'#' : L' ';
	}
}
void MoveSnake(Direction dir) {
	int nNextIndex = snake.front();
	
	switch (dir) {
	case NORTH:
		nNextIndex -= field_width;
		break;
	case WEST:
		nNextIndex -= 1;
		break;
	case SOUTH:
		nNextIndex += field_width;
		break;
	case EAST:
		nNextIndex += 1;
	}
	snake.push_front(nNextIndex);
	snake.pop_back();
}

