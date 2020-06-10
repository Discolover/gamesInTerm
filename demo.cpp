//au BufWritePost demo.cpp !make Comp && make Link && ./demo-app
//Comp:
//	g++ -c demo.cpp
//Link:
//	g++ demo.o -o demo-app -lsfml-graphics -lsfml-window -lsfml-system
#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

using namespace sf;

void drawBoard(RenderWindow& win, Sprite& s);
void checkLine();

#define W 10
#define H 20

int figures[8][4] {
    {},
    {1, 3, 5, 7}, // I
    {2, 4, 5, 7}, // S
    {3, 5, 4, 6}, // Z
    {3, 5, 7, 6}, // J
    {2, 4, 6, 7}, // L
    {2, 3, 4, 5}, // O
    {2, 4, 5, 6}  // T
};

int board[H][W];

struct point {
    int x, y;
};

struct Tetrimino {
    point next[4];
    point prev[4];
    int fig;

    Tetrimino() {
	create();
    }

    void create() {
	fig = 1 + rand() % 7;
	for (int i = 0; i < 4; ++i) {
	    next[i].x = figures[fig][i] % 2;
	    next[i].y = figures[fig][i] / 2;

	    next[i].x += 3; //offset
	    next[i].y -= 4; //offset
	}
    }

    void draw(RenderWindow& win, Sprite& s) {
	static int cnt = 0;
	s.setTextureRect(IntRect(18 * fig, 0, 18, 18));
	for (int i = 0; i < 4; ++i) {
	    if (next[i].y < 0) continue;
	    s.setPosition(next[i].x * 18, next[i].y * 18);
	    win.draw(s);
	}
    }

    void move(int dx) {
	for (int i = 0; i < 4; ++i) {
	    next[i].x += dx;
	}
	handleCollision();
    }

    void rotate() {
	point pivot = next[1];
	for (int i = 0; i < 4; ++i) {
	    int x = next[i].y - pivot.y;
	    int y = next[i].x - pivot.x;
	    next[i].x = pivot.x - x;
	    next[i].y = pivot.y + y;
	}
	handleCollision();
    }

    void fall() {
	for (int i = 0; i < 4; ++i) {
	    next[i].y += 1;
	}
	if (isCollide()) {
	    for (int i = 0; i < 4; ++i) {
		if (next[i].y < 0) {
		    printf("Game over!!!\n");
		    exit(0);
		}
	    }
	    for (int i = 0; i < 4; ++i) {
		int x = prev[i].x, y = prev[i].y;
		board[y][x] = fig;
	    }
	    checkLine();
	    create();
	} else {
	    memcpy(prev, next, sizeof next);
	}
    }

    void handleCollision() {
	if (isCollide())
	    memcpy(next, prev, sizeof prev);
	else
	    memcpy(prev, next, sizeof next);
    }

    bool isCollide() {
	for (int i = 0; i < 4; ++i) {
	    int x = next[i].x;
	    int y = next[i].y;
	    if (y < 0) continue;
	    if (x < 0 || x >= W || y >= H || board[y][x])
		return true;
	}
	return false;
    }
};

int main() {
    RenderWindow win(VideoMode(W * 18, H * 18), "demo");
    Tetrimino piece; Texture t; Clock clock;
    Sprite s = Sprite(t);
    t.loadFromFile("tiles.png");
    float timer = 0, delay = 0.15;

    while (win.isOpen()) {
	Event event;
	int dx = 0;
	bool rotated = false;

	timer += clock.getElapsedTime().asSeconds();
	clock.restart();

	while (win.pollEvent(event))
	{
	    if (event.type == Event::MouseButtonPressed) {
		win.close();
	    } else if (event.type == Event::KeyPressed) {
		if (event.key.code == Keyboard::R)
		    rotated = true;
		if (event.key.code == Keyboard::A)
		    dx = -1;
		if (event.key.code == Keyboard::D)
		    dx = +1;
	    }
	}

	if (dx) piece.move(dx);
	if (rotated) piece.rotate();
	if (timer > delay) {
	    piece.fall();
	    timer = 0;
	}

	win.clear(Color::White);
	drawBoard(win, s);
	piece.draw(win, s);
	win.display();
    }
    return 0;
}

void drawBoard(RenderWindow& win, Sprite& s)
{
    for (int i = 0; i < H; ++i) {
	for (int j = 0; j < W; ++j) {
	    if (!board[i][j]) continue;
	    s.setTextureRect(IntRect(board[i][j] * 18, 0, 18, 18));
	    s.setPosition(18 * j, 18 * i);
	    win.draw(s);
	}
    }
}

//not very case-efficient, but simple
void checkLine() {
    for (int y = 0; y < H; ++y) {
	int x = 0;
	for (; x < W; ++x) {
	    if (board[y][x] == 0) break;
	}
	if (x != W) continue;
	for (int k = y; k > 0; --k) {
	    int sum = 0;
	    for (int l = 0; l < W; ++l) {
		sum += board[k-1][l];
		board[k][l] = board[k-1][l];
	    }
	    if (sum == 0) break;
	}
    }
}
