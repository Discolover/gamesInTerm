// au BufWritePost demo.c !gcc -W -g -o demo demo.c -lncurses && ./demo
#include <stdio.h> 
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <time.h>

#define drwBlock(y, x, color) (mvwaddch(gameWin, y, x*2, ACS_BLOCK | (color)) \
		, waddch(gameWin, ACS_BLOCK | (color)))
#define clrBlock(y, x) (mvwaddch(gameWin, y, x*2, ' ') \
		, waddch(gameWin, ' '))
#define sec() (clock() / CLOCKS_PER_SEC)

#define width 10
#define heigth 20

char tetromino[8][16] = { 
	/* start indexing from 1 for better redabily */
	""
	,
	"..x."
	"..x."
	"..x."
	"..x."
	,
	"...."
	".xx."
	".xx."
	"...."
	,
	"...."
	"..x."
	".xxx"
	"...."
	,
	"...."
	".x.."
	".xxx"
	"...."
	,
	"...."
	"...x"
	".xxx"
	"...."
	,
	"...."
	"..xx"
	".xx."
	"...."
	,
	"...."
	".xx."
	"..xx"
	"...."
};

WINDOW *gameWin;
int map[heigth][width];
int gameOver = 0;
int Y, X, FIG, ROT;

void init()
{
	/* curses initialization */
	initscr();
	noecho();
	cbreak();

	/* colors */
	start_color();
	/* assume_default_colors(COLOR_BLACK, COLOR_WHITE); */
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
	gameWin = newwin(heigth, width*2, 0, 0);
	wtimeout(gameWin, 0);
	refresh();
	box(gameWin, 0, 0);
	/* game logic initialization */
	for (int i = 0; i < heigth; ++i) {
		for (int j = 0; j < width; ++j) {
			if (i != 0 && (j == 0 || i == (heigth-1) || j == (width-1))) {
				map[i][j] = 1;
			}
		}
	}
	wrefresh(gameWin);
}

void createFig()
{
	X = width/2, Y = 1;
	FIG = 3;//1 + rand() % 7;
	ROT = 0;
}

int figIndex(int y, int x, int r)
{
	switch(r) {
	case 0: return y * 4 + x;		/* 0 degrees */
	case 1: return 12 + y - x * 4;	/* 90 degrees */
	case 2:	return 15 - y * 4 - x;	/* 180 degrees */
	case 3: return 3 - y + 4 * x;	/* 270 degrees */
	}
}

void drwFig()
{
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (tetromino[FIG][figIndex(i, j, ROT)] == 'x') {
				drwBlock(Y + i, X + j, COLOR_PAIR(FIG));
			}
		}
	}
}

void undrwFig()
{
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (tetromino[FIG][figIndex(i, j, ROT)] == 'x') {
				clrBlock(Y + i, X + j);
			}
		}
	}
}

void mvFig(int dy, int dx)
{
	X += dx;	
	Y += dy;
}

int main()
{
	init();

	createFig();
	drwFig();
	wrefresh(gameWin);

	double st = sec();

	while (!gameOver) {
		switch (wgetch(gameWin)) {
		case 'r': 
			undrwFig();
			ROT = (ROT + 1) % 4;
			drwFig();
			wrefresh(gameWin);
			break;
		case 'a':
			undrwFig();
			mvFig(0, -1);
			drwFig();
			break;
		case 'd':
			undrwFig();
			mvFig(0, 1);
			drwFig();
			break;
		}
		if (sec() - st >= 0.5) {
			undrwFig();
			mvFig(1, 0);
			drwFig();
			st = sec();
		}
	}
	endwin();
	return 0;
}
