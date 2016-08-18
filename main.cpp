#include <iostream>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <Windows.h>
#include <stdio.h>
#include "conio2.h"
#include <tchar.h>
#include <ctime>
#define CLOSED ' '
#define DOOR '\xB1'
#define DOORHEIGHT 50
#define CROSS '\xC4'
#define BACKSPACE '\b'
#define STRAIGHT '\xB3'
#define UNDER '\x5F'
#define MAP_HEIGHT 20
#define PPYCENTER 80
#define MAP_WIDTH 20
#define PLANEWIDTH 300
#define PLANEHEIGHT 150
#define WALLHEIGHT 20
#define TILE_SIZE 64
#define PLAYERHEIGHT 16
#define PLAYERDISTANCEPP 277
#define ANGLE60 60 
#define ANGLE30 (ANGLE60/2)
#define ANGLE15 (ANGLE30/2)
#define ANGLE90 (ANGLE30*3)
#define ANGLE180 (ANGLE90*2)
#define ANGLE270 (ANGLE90*3)
#define ANGLE360 (ANGLE60*6)
#define ANGLE5 (ANGLE30/6)
#define ANGLE10 (ANGLE5*2)
#define ANGLE0 0
#define ROWSIZE 17
#define COLUMNSIZE 6
#define WALL 1
#define PLAYER 2
#define K_LEFT 75
#define K_RIGHT 77
#define K_UP 72
#define K_DOWN 80
#define PLAYERSPEED 5

using namespace std;
class Tables {
public:
	Tables() {
		int i = 0;
		float radian = 0;
		pi = 3.141592653589793;
	}

	float degreeconverter(float angle);
private:
	float pi;
};

float Tables::degreeconverter(float angle) {
	return((float)(angle*pi) / (float)ANGLE180);
}


class Player {
public:
	Player() {
		playerx = 0;
		playery = 0;
		playerarc = ANGLE270 - 1;
		moves = 0;
		playerspeed = PLAYERSPEED;
	}
	int playerx;
	int playery;
	int moves;
	float playerarc;
	float playeryarc;
	float playerxarc;
	int playerspeed;
	int playerbasex;
	int playerbasey;
};

class Buffer {
public:
	Buffer() {
		bufSize.X = PLANEWIDTH;
		bufSize.Y = PLANEHEIGHT;

		bufCoord.X = 0;
		bufCoord.Y = 0;

		buffer_source.Top = 0;
		buffer_source.Left = 0;
		buffer_source.Bottom = 159;
		buffer_source.Right = 319;

		mainScreen = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	void bufferDraw(CHAR_INFO chiBuffer[51200]);
private:
	COORD bufSize, bufCoord;
	SMALL_RECT buffer_source;
	HANDLE mainScreen, buffer;

};
void Buffer::bufferDraw(CHAR_INFO chiBuffer[51200]) {
	SetConsoleScreenBufferSize(mainScreen, bufSize);
	WriteConsoleOutputA(mainScreen, chiBuffer, bufSize, bufCoord, &buffer_source);
}

class Map {
public:
	Map(Player* playerclass) {
		height = 0;
		width = 0;
		player = playerclass;
		loadFile("test.txt");
		mapend[0] = 16;
		mapend[1] = 19;
		WallType = '#';
	}
	int map[20][20];
	int mapend[2];
	void loadFile(char filename[]);
	void loadMapFromFile(int width, int height);
	int height;
	int width;
	char WallType;
private:
	char ch[20][20];
	Player* player;
};

void Map::loadFile(char filename[]) {
	FILE *fp = NULL;
	fp = fopen(filename, "r");
	char temp = 1;
	int i = 0;
	int j = 0;
	width = 0;
	height = 0;
	while (temp!=EOF) {
		if ((temp = fgetc(fp)) == '\n') {
			i++;
			width = j;
			j = 0;
			ch[i][j] = fgetc(fp);
			j++;
			continue;
		}
		else if (temp == EOF) {
			height = i + 1;
			break;
		}
		else
			ch[i][j] = temp;
		j++;
	}
	loadMapFromFile(width, height);
}

void Map::loadMapFromFile(int width, int height)
{
	for (int i = 0; i < height + 1; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (ch[i][j] == '#')
				map[i][j] = 1;
			else if (ch[i][j] == '*')
				map[i][j] = 0;
			else if (ch[i][j] == '@')
				map[i][j] = 3;
			else if (ch[i][j] == '+')
				map[i][j] = 4;
			else if (ch[i][j] == '&')
				map[i][j] = 6;
			else if (ch[i][j] == '%')
				map[i][j] = 7;
			else if (ch[i][j] == '-') {
				map[i][j] = 0;
				mapend[0] = j;
				mapend[1] = i;
			}
			else if (ch[i][j] == 'd')
				map[i][j] = 5;
			else if (ch[i][j] == 'p') {
				map[i][j] = 2;
				player->playerx = (j+1)*TILE_SIZE - (TILE_SIZE / 2);
				player->playery = (i+1)*TILE_SIZE - (TILE_SIZE / 2);
			}
			else if (ch[i][j] == 's') {
				player->playerbasex = (j + 1)*TILE_SIZE - (TILE_SIZE / 2);
				player->playerbasey = (i + 1)*TILE_SIZE - (TILE_SIZE / 2);
				player->playerx = (j + 1)*TILE_SIZE - (TILE_SIZE / 2);
				player->playery = (i + 1)*TILE_SIZE - (TILE_SIZE / 2);
			}
		}
	}
}

class Renderer {
public:

	Renderer() {
		table = Tables();
		for (int i = 0; i < 51200; i++) {
			chiBuffer[i].Char.AsciiChar = ' ';
			chiBuffer[i].Attributes = 1;
			WallTypeX = 0;
			WallTypeY = 0;
			lastDoor = false;
			closedWall = false;
		}
		miniMapX = 250;
	}
	void rayCast(int PlayerY, int PlayerX, float PlayerArc, Map& map, bool opendoor);
	void drawLab(int x, int drawStart, int drawEnd, Map map);
	void clearBuffer();
	void drawMiniMap(Map map, Player player);
	float dist;
	float tempdist;
	CHAR_INFO chiBuffer[51200];

private:
	Tables table;
	int castColumn;
	int firstPY;
	int firstPX;
	int wallMapX;
	int wallMapY;
	int stepY;
	int stepX;
	int nextPY;
	int nextPX;
	int projectedWallHeight;
	int bottomOfWall;
	int topOfWall;
	int basePlayerArc;
	float finalHWall;
	float finalVWall;
	float distortion;
	bool isWall;
	int i;
	float negarc;
	int miniMapX;
	int WallTypeY;
	int WallTypeX;
	char checkOri(Player player);
	char wallType(int wall);
	void drawDoor(int x, int drawStart, int drawEnd, Map map);
	void closeDoor(int x, int drawStart, int drawEnd, Map map);
	Buffer buffer;
	bool lastDoor;
	bool closedWall;
};

void Renderer::clearBuffer() {
	for (int i = 0; i < 51200; i++) {
		chiBuffer[i].Char.AsciiChar = ' ';
		chiBuffer[i].Attributes = 7;
	}
}
void Renderer::rayCast(int PlayerY, int PlayerX, float PlayerArc, Map& map, bool opendoor) {

	dist = 0;
	distortion = ANGLE30;
	closedWall = false;
	PlayerArc = PlayerArc + ANGLE30;
	for (castColumn = 0; castColumn < PLANEWIDTH; castColumn += 1) {
		tempdist = dist;
		isWall = false;
		if (PlayerArc > 360)
			PlayerArc = PlayerArc - 360;
		if (PlayerArc < 0)
			PlayerArc = ANGLE360 + PlayerArc;
		//check vertical walls
		if (PlayerArc > 0 && PlayerArc < 180) {
			firstPY = floor(PlayerY / TILE_SIZE) * TILE_SIZE - 1;
			stepY = -TILE_SIZE;
			stepX = TILE_SIZE / tan(table.degreeconverter(PlayerArc + 0.00001f));
		}
		else {
			firstPY = floor(PlayerY / TILE_SIZE) * TILE_SIZE + TILE_SIZE;
			stepY = TILE_SIZE;
			stepX = -(TILE_SIZE / tan(table.degreeconverter(PlayerArc + 0.00001f)));
		}
		//if (PlayerArc == 0 || PlayerArc == 180)
		//	finalVWall = FLT_MAX;
		if(true) {
			firstPX = PlayerX + (int)((PlayerY - firstPY) / tan(table.degreeconverter(PlayerArc + 0.00001f)));

			wallMapX = (int)floor(firstPX / TILE_SIZE);
			wallMapY = (int)floor(firstPY / TILE_SIZE);

			i = 0;
			isWall = false;
			while (true) {
				if (wallMapX >= 0 && wallMapY >= 0 && wallMapX < MAP_WIDTH && wallMapY < MAP_HEIGHT && map.map[wallMapY][wallMapX] == 9 && opendoor == true) {
					map.WallType = CLOSED;
					WallTypeY = map.map[wallMapY][wallMapX];
					isWall = true;
					break;
				}
				if (wallMapX >= 0 && wallMapY >= 0 && wallMapX < MAP_WIDTH && wallMapY < MAP_HEIGHT && (map.map[wallMapY][wallMapX] == 1 || map.map[wallMapY][wallMapX] == 3 || map.map[wallMapY][wallMapX] == 4 || map.map[wallMapY][wallMapX] == 5 || map.map[wallMapY][wallMapX] == 6 || map.map[wallMapY][wallMapX] == 7)) {
					WallTypeY = map.map[wallMapY][wallMapX];
					isWall = true;
					break;
				}
				if (i == 0) {
					nextPY = firstPY + stepY;
					nextPX = firstPX + stepX;
				}
				else {
					nextPY += stepY;
					nextPX += stepX;
				}
				if (nextPX < 0 || nextPY < 0)
					break;

				wallMapX = (int)floor(nextPX / TILE_SIZE);
				wallMapY = (int)floor(nextPY / TILE_SIZE);

				if (wallMapX > MAP_WIDTH || wallMapX < 0 || wallMapY > MAP_HEIGHT || wallMapY < 0) {
					finalVWall = FLT_MAX;
					isWall = false;
					break;
				}
				i++;
			}
			if (isWall && i != 0)
				finalVWall = sqrt(pow((PlayerX - nextPX), 2) + pow((PlayerY - nextPY), 2));
			else if (isWall && i == 0)
				finalVWall = sqrt(pow((PlayerX - firstPX), 2) + pow((PlayerY - firstPY), 2));
			else
				finalVWall = FLT_MAX;
		}

		//check horizontal walls

		if (PlayerArc < ANGLE90 && PlayerArc > ANGLE0) {
			firstPX = floor(PlayerX / TILE_SIZE)*TILE_SIZE + TILE_SIZE;
			stepX = TILE_SIZE;
			stepY = -(TILE_SIZE*tan(table.degreeconverter(PlayerArc + 0.00001f)));

		}
		else if (PlayerArc > ANGLE270 && PlayerArc < ANGLE360) {
			firstPX = floor(PlayerX / TILE_SIZE)*TILE_SIZE + TILE_SIZE;
			stepX = TILE_SIZE;
			stepY = -(TILE_SIZE*tan(table.degreeconverter(PlayerArc + 0.00001f)));

		}
		else if (PlayerArc > ANGLE90 && PlayerArc < ANGLE180) {
			firstPX = floor(PlayerX / TILE_SIZE)*TILE_SIZE - 1;
			stepX = -TILE_SIZE;
			stepY = TILE_SIZE*tan(table.degreeconverter(PlayerArc + 0.00001f));

		}
		else {
			firstPX = floor(PlayerX / TILE_SIZE)*TILE_SIZE - 1;
			stepX = -TILE_SIZE;
			stepY = TILE_SIZE*tan(table.degreeconverter(PlayerArc + 0.00001f));

		}
		//if (PlayerArc == ANGLE90 || PlayerArc == ANGLE270)
		//	finalHWall = FLT_MAX;
		if(true) {

			firstPY = PlayerY + (PlayerX - firstPX)*tan(table.degreeconverter(PlayerArc+0.00001f));

			wallMapX = (int)floor(firstPX / TILE_SIZE);
			wallMapY = (int)floor(firstPY / TILE_SIZE);
			i = 0;
			isWall = false;
			while (true) {
				if (wallMapX >= 0 && wallMapY >= 0 && wallMapX < MAP_WIDTH && wallMapY < MAP_HEIGHT && map.map[wallMapY][wallMapX] == 9 && opendoor == true) {
					map.WallType = CLOSED;
					WallTypeX = map.map[wallMapY][wallMapX];
					isWall = true;
					break;
				}
				if (wallMapX >= 0 && wallMapY >= 0 && wallMapX < MAP_WIDTH && wallMapY < MAP_HEIGHT && (map.map[wallMapY][wallMapX] == 1 || map.map[wallMapY][wallMapX] == 3 || map.map[wallMapY][wallMapX] == 4 || map.map[wallMapY][wallMapX] == 5 || map.map[wallMapY][wallMapX] == 6 || map.map[wallMapY][wallMapX] == 7)) {
					WallTypeX = map.map[wallMapY][wallMapX];
					isWall = true;
					break;
				}
				if (i == 0) {
					nextPY = firstPY + stepY;
					nextPX = firstPX + stepX;
				}
				else {
					nextPY += stepY;
					nextPX += stepX;
				}
				if (nextPX < 0 || nextPY < 0)
					break;

				wallMapX = (int)floor(nextPX / TILE_SIZE);
				wallMapY = (int)floor(nextPY / TILE_SIZE);

				if (wallMapX > MAP_WIDTH || wallMapX < 0 || wallMapY > MAP_HEIGHT || wallMapY < 0) {
					finalHWall = FLT_MAX;
					isWall = false;
					break;
				}
				i++;
			}
			if (isWall && i != 0)
				finalHWall = sqrt(pow((PlayerX - nextPX), 2) + pow((PlayerY - nextPY), 2));
			else if (isWall && i == 0)
				finalHWall = sqrt(pow((PlayerX - firstPX), 2) + pow((PlayerY - firstPY), 2));
			else
				finalHWall = FLT_MAX;
		}


		if (finalHWall < finalVWall) {
			dist = finalHWall;
			map.WallType = wallType(WallTypeX);
		}
		else {
			dist = finalVWall;
			map.WallType = wallType(WallTypeY);
		}

		dist = dist*cos(table.degreeconverter(distortion));

		projectedWallHeight = (int)((WALLHEIGHT / dist)*(float)PLAYERDISTANCEPP);
		bottomOfWall = PPYCENTER + (int)(projectedWallHeight*0.5f);
		topOfWall = PPYCENTER - (int)(projectedWallHeight*0.5f);
		if (bottomOfWall >= PLANEHEIGHT)
			bottomOfWall = PLANEHEIGHT - 1;
		if (topOfWall < 0)
			topOfWall = 1;

		if (dist > 150) {
			textcolor(DARKGRAY);
		}
		else if (dist >50)
		{
			textcolor(WHITE);
		}
		else if (dist != tempdist)
		{
			textcolor(WHITE);
		}

		if (opendoor && map.WallType == DOOR && dist < 64) {
			drawDoor(castColumn, topOfWall, bottomOfWall, map);
			lastDoor = true;
		}
		else {
			drawLab(castColumn, topOfWall, bottomOfWall, map);
		}
		if (opendoor && map.WallType == CLOSED && dist < 64) {
			closeDoor(castColumn, topOfWall, bottomOfWall, map);
			lastDoor = true;
			closedWall = true;
		}
		if (castColumn == 319 && opendoor) {
			if (map.map[(int)floor(PlayerY / TILE_SIZE) + 1][(int)floor(PlayerX / TILE_SIZE)] == 5)
				map.map[(int)floor(PlayerY / TILE_SIZE) + 1][(int)floor(PlayerX / TILE_SIZE)] = 9;
			if (map.map[(int)floor(PlayerY / TILE_SIZE) -1][(int)floor(PlayerX / TILE_SIZE)] == 5)
				map.map[(int)floor(PlayerY / TILE_SIZE) -1][(int)floor(PlayerX / TILE_SIZE)] = 9;
			if (map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) + 1] == 5)
				map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) + 1] = 9;
			if (map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) - 1] == 5)
				map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) - 1] = 9;
		}
		if (castColumn == 319 && opendoor && closedWall) {
			if (map.map[(int)floor(PlayerY / TILE_SIZE) + 1][(int)floor(PlayerX / TILE_SIZE)] == 9)
				map.map[(int)floor(PlayerY / TILE_SIZE) + 1][(int)floor(PlayerX / TILE_SIZE)] = 5;
			if (map.map[(int)floor(PlayerY / TILE_SIZE) - 1][(int)floor(PlayerX / TILE_SIZE)] == 9)
				map.map[(int)floor(PlayerY / TILE_SIZE) - 1][(int)floor(PlayerX / TILE_SIZE)] = 5;
			if (map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) + 1] == 9)
				map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) + 1] = 5;
			if (map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) - 1] == 9)
				map.map[(int)floor(PlayerY / TILE_SIZE)][(int)floor(PlayerX / TILE_SIZE) - 1] = 5;
		}
		distortion -= 0.1875f;
		//negarc += (60/120);
		PlayerArc = PlayerArc - 0.1875f;
	}

}

void Renderer::closeDoor(int x, int drawStart, int drawEnd, Map map){
	for (int j = 0; j < 1; j++) {
		for (int i = drawStart; i < drawEnd; i++) {
			if (i == drawStart)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = CROSS;
			else if (i == drawEnd - 1)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = CROSS;
			else
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = DOOR;
		}
	}
	buffer.bufferDraw(chiBuffer);
}

char Renderer::wallType(int wall) {
	if (wall == 1)
		return '#';
	else if (wall == 3)
		return '@';
	else if (wall == 4)
		return '+';
	else if (wall == 5)
		return DOOR;
	else if (wall == 9)
		return CLOSED;
	else if (wall == 6)
		return '&';
	else if (wall == 7)
		return '%';
	else
		return '#';
}
void Renderer::drawLab(int x, int drawStart, int drawEnd, Map map) {
	gotoxy(x, drawStart);
	for (int j = 0; j < 1; j++) {
		for (int i = drawStart; i < drawEnd; i++) {
			if (i == drawStart)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = CROSS;
			else if (i == drawEnd - 1)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = CROSS;
			else
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = map.WallType;
		}
	}
}

void Renderer::drawDoor(int x, int drawStart, int drawEnd, Map map) {
	for (int j = 0; j < 1; j++) {
		for (int i = drawStart; i < drawEnd; i++) {
			if (i == drawStart)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = ' ';
			else if (i == drawEnd - 1)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = ' ';
			else if(i > drawEnd - DOORHEIGHT)
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = ' ';
			else
				chiBuffer[x + PLANEWIDTH * i].Char.AsciiChar = ' ';
		}
	}
	buffer.bufferDraw(chiBuffer);
}

void Renderer::drawMiniMap(Map map, Player player) {
	for (int j = 0; j < map.height; j++) {
		for (int i = 0; i < map.width; i++) {
			if (map.map[i][j] == 1)
				chiBuffer[j + PLANEWIDTH * (i+1)].Char.AsciiChar = '#';
			else if (map.map[i][j] == 3)
				chiBuffer[j + PLANEWIDTH * (i + 1)].Char.AsciiChar = '@';
			else if (map.map[i][j] == 4)
				chiBuffer[j + PLANEWIDTH * (i + 1)].Char.AsciiChar = '+';
			else if (map.map[i][j] == 2)
				chiBuffer[j + PLANEWIDTH * (i+1)].Char.AsciiChar = checkOri(player);
			else if(map.map[i][j] == 0)
				chiBuffer[j + PLANEWIDTH * (i+1)].Char.AsciiChar = ' ';
			else if (map.map[i][j] == 5)
				chiBuffer[j + PLANEWIDTH * (i + 1)].Char.AsciiChar = 'd';
		}
	}
}

char Renderer::checkOri(Player player) {
	if (player.playerarc > ANGLE270 + ANGLE90 / 2 || player.playerarc < ANGLE90 / 2)
		return '>';
	else if (player.playerarc > ANGLE90 / 2 && player.playerarc < (ANGLE180 - (ANGLE90 / 2)))
		return '^';
	if (player.playerarc < ANGLE270 - (ANGLE90 / 2) && player.playerarc > (ANGLE180 - (ANGLE90 / 2)))
		return '<';
	else
		return 'v';
}

void clear_screen() {
	DWORD n;
	DWORD size;
	COORD coord = { 0 };
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(h, &csbi);

	size = csbi.dwSize.X * csbi.dwSize.Y;

	FillConsoleOutputCharacter(h, TEXT(' '), size, coord, &n);
	GetConsoleScreenBufferInfo(h, &csbi);
	FillConsoleOutputAttribute(h, csbi.wAttributes, size, coord, &n);
	SetConsoleCursorPosition(h, coord);
}

bool checkCollision(int x, int y, Map map)
{
	x = (int)floor(x / TILE_SIZE);
	y = (int)floor(y / TILE_SIZE);
	if (map.map[y][x] == 1 || x > MAP_WIDTH || y > MAP_HEIGHT || map.map[y][x] == 3 || map.map[y][x] == 4 || map.map[y][x] == 5)
		return false;
	else
		return true;

}

void loadGame(Map& map) {
	clear_screen();
	FILE *fp;
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2);
	char file[20];
	cout << "Enter file to open: ";
	cin >> file;
	strcat(file, ".txt");
	map.loadFile(file);
}

void saveGame(Map& map) {
	clear_screen();
	FILE *fp;
	char file[20];
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2);
	cout << "Save file as: ";
	cin >> file;
	strcat(file, ".txt");
	fp = fopen(file, "w");
	for (int i = 0; i < map.height; i++) {
		for (int j = 0; j < map.width; j++) {
			if (map.map[i][j] == 1)
				fputc('#', fp);
			else if (map.map[i][j] == 0)
				fputc('*', fp);
			else if (map.map[i][j] == 2)
				fputc('p', fp);
			else if (map.map[i][j] == 3)
				fputc('@', fp);
			else if (map.map[i][j] == 4)
				fputc('+', fp);
			else if (map.map[i][j] == 5)
				fputc('d', fp);
			else if (map.map[i][j] == 6)
				fputc('&', fp);
			else if (map.map[i][j] == 7)
				fputc('%', fp);
		}
		fputc('\n', fp);
	}
	fclose(fp);
}

void changeWalls(Map& map) {
	clear_screen();
	char temp;
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2);
	cout << "Enter a new symbol: ";
	cin >> temp;
	map.WallType = temp;
}

char wallType(int wall) {
	if (wall == 1)
		return '#';
	else if (wall == 3)
		return '@';
	else if (wall == 4)
		return '+';
	else if (wall == 5)
		return 'd';
	else if (wall == 6)
		return '&';
	else if (wall == 7)
		return '%';
	else
		return '#';
}

bool illegalMap(int x, int y, Map map) {
	if (map.map[y][x - 1] == 0 && map.map[y - 1][x - 1] == 0)
		return false;
	else if (map.map[y][x + 1] == 0 && map.map[y + 1][x + 1] == 0)
		return false;
	else
		return true;

}

void editMap(Map& map) {
	clear_screen();
	char a = 'g';
	int x = PLANEWIDTH / 2;
	int y = PLANEHEIGHT / 2;
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2);
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (map.map[i][j] == 1 || map.map[i][j] == 3 || map.map[i][j] == 4 || map.map[i][j] == 5 || map.map[i][j] == 6 || map.map[i][j] == 7) {
				gotoxy(x + j, y + i);
				cout << wallType(map.map[i][j]);
			}
		}
	}

	x = PLANEWIDTH / 2 + 20;
	y = PLANEHEIGHT / 2 + 20;
	gotoxy(x, y);
	while (a  != 'q') {
		a = getch();	
		if (a == 0 || a == 224) {
			switch (getch()) {
			case K_DOWN:
				y++;
				gotoxy(x, y);
				break;
			case K_RIGHT:
				x++;
				gotoxy(x, y);
				break;
			case K_UP:
				y--;
				gotoxy(x, y);
				break;
			case K_LEFT:
				x--;
				gotoxy(x, y);
				break;
			}
		}
		else if (a == BACKSPACE) {
			if (x - PLANEWIDTH / 2 < MAP_WIDTH && y - PLANEHEIGHT / 2 < MAP_HEIGHT) {
				if (illegalMap(x - PLANEWIDTH / 2, y - PLANEHEIGHT / 2, map)) {
					map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 0;
					cout << ' ';
					gotoxy(x, y);
				}
			}
		}
		else {
			switch (a) {
			case '@':
				map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 3;
				cout << '@';
				gotoxy(x, y);
				break;
			case '+':
				map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 4;
				cout << '+';
				gotoxy(x, y);
				break;
			case '#':
				map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 1;
				cout << '#';
				gotoxy(x, y);
				break;
			case 'd':
				map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 5;
				cout << 'd';
				gotoxy(x, y);
				break;
			case '&':
				map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 6;
				cout << '&';
				gotoxy(x, y);
				break;
			case '%':
				map.map[y - PLANEHEIGHT / 2][x - PLANEWIDTH / 2] = 7;
				cout << '%';
				gotoxy(x, y);
				break;
			default:
				break;
			}
		}
	}
}

void welcomeScreen(Map& map, int& r, bool& p)
{
	clear_screen();
	int a = 0;
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2);
	cout << "Welcome Player! Please choose an option below:";
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2 + 2);
	cout << "1: New game";
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2 + 4);
	cout << "2: Load game";
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2 + 6);
	cout << "3: Save game";
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2 + 8);
	cout << "4: Edit map";
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2 + 10);
	cout << "5: Different wall style";
	gotoxy(PLANEWIDTH / 2, PLANEHEIGHT / 2 + 12);
	cout << "6: Quit";

	do {
		a = getch();
		switch (a) {
		case '1':
			r = 'r';
			p = false;
			break;
		case '2':
			loadGame(map);
			r = 'r';
			p = false;
			break;
		case '3':
			saveGame(map);
			break;
		case '4':
			editMap(map);
			break;
		case '5':
			changeWalls(map);
			break;
		case '6':
			r = 'q';
			break;
		}
	} while (a != '1' && a != '2' && a != '3' && a != '4' && a != '5' && a != '6');

}

int main()
{
	CONSOLE_FONT_INFOEX text;
	clock_t begin = clock();
	clock_t end;
	double elapsed_seconds;

	HANDLE mainScreen;
	mainScreen = GetStdHandle(STD_OUTPUT_HANDLE);
	GetCurrentConsoleFontEx(mainScreen, false, &text);
	text.dwFontSize.X = 4;
	text.dwFontSize.Y = 6;
	SetCurrentConsoleFontEx(mainScreen, false, &text);

	//initialize
	bool r = false;
	int a = 0;
	bool opendoor = false;
	settitle("LABIRYNT");
	system("mode 320, 160");
	Buffer buffer = Buffer();
	Tables tables = Tables();
	Player player = Player();
	Map map = Map(&player);
	Map mini = Map(&player);
	Renderer renderer = Renderer();

	welcomeScreen(map, a, r);
	r = true;
	do {
		//output
		renderer.rayCast(player.playery, player.playerx, player.playerarc, map, opendoor);
		if (opendoor) {
			opendoor = false;
			renderer.rayCast(player.playery, player.playerx, player.playerarc, map, opendoor);
		}
		renderer.drawMiniMap(mini, player);
		buffer.bufferDraw(renderer.chiBuffer);
		//input
		
		gotoxy(1, 1);
		end = clock();
		elapsed_seconds = double(end - begin) / CLOCKS_PER_SEC;
		cout << "Time: " << elapsed_seconds << " Moves: " << player.moves;
		a = getch();
		if (a == (int)'m')
			welcomeScreen(map, a, r);
		if (a == (int)'a')
			player.playerspeed = 64;
		if (a == 'd') {
			opendoor = true;
			continue;
		}
		if (a == 'r') {
			if (r) {
				player.playerx = player.playerbasex;
				player.playery = player.playerbasey;
			}
			player.playerarc = ANGLE270;
			player.moves = 0;
			elapsed_seconds = 0;
			begin = clock();
			r = true;
			renderer.clearBuffer();
			continue;
		}
		//if (a == (int)'h')
			//dosomething
		if (a == 0 || a == 224) {

			player.playerxarc = cos(tables.degreeconverter(player.playerarc));
			player.playeryarc = sin(tables.degreeconverter(player.playerarc));
			//set player direction
			switch (getch()) {

			case K_DOWN:
				if (player.playery + (int)(player.playeryarc*player.playerspeed) > 0 && player.playery + (int)(player.playeryarc*PLAYERSPEED) < MAP_HEIGHT*TILE_SIZE && player.playerx - (int)(player.playerxarc*player.playerspeed) > 0 && player.playerx - (int)(player.playerxarc*player.playerspeed) < TILE_SIZE*MAP_WIDTH && (player.playerx / TILE_SIZE) && checkCollision(player.playerx - (int)(player.playerxarc*player.playerspeed), player.playery + (int)(player.playeryarc*player.playerspeed), map))
				{
					mini.map[(int)floor(player.playery / TILE_SIZE)][(int)floor(player.playerx / TILE_SIZE)] = 0;
					player.playery += (int)(player.playeryarc*player.playerspeed);
					player.playerx -= (int)(player.playerxarc*player.playerspeed);
					mini.map[(int)floor(player.playery / TILE_SIZE)][(int)floor(player.playerx / TILE_SIZE)] = 2;
					player.moves += 1;
				}
				break;

			case K_RIGHT:
				if ((player.playerarc -= ANGLE5) < ANGLE0)
					player.playerarc += ANGLE360;
				break;

			case K_UP: if (player.playery - (int)(player.playeryarc*player.playerspeed) > 0 && player.playery - (int)(player.playeryarc*player.playerspeed) < MAP_HEIGHT*TILE_SIZE && player.playerx + (int)(player.playerxarc*player.playerspeed) > 0 && player.playerx + (int)(player.playerxarc*player.playerspeed) < TILE_SIZE * MAP_WIDTH && checkCollision(player.playerx + (int)(player.playerxarc*player.playerspeed), player.playery - (int)(player.playeryarc*player.playerspeed), map))
			{
				mini.map[(int)floor(player.playery / TILE_SIZE)][(int)floor(player.playerx / TILE_SIZE)] = 0;
				player.playery -= (int)(player.playeryarc*player.playerspeed);
				player.playerx += (int)(player.playerxarc*player.playerspeed);
				mini.map[(int)floor(player.playery / TILE_SIZE)][(int)floor(player.playerx / TILE_SIZE)] = 2;
				player.moves += 1;
			}
					   break;

			case K_LEFT:
				if ((player.playerarc += ANGLE5) > ANGLE360)
					player.playerarc -= ANGLE360;
				break;
			}
		}

		//processing
		renderer.clearBuffer();
	} while (a != 'q');

	return 0;
}




