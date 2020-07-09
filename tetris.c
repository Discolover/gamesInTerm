#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#define W 10
#define H 20

#define sec() ((double)clock() / CLOCKS_PER_SEC)
#define draw_block(y, x, color)						\
do {									\
    mvwaddch(GAME.win, y, x * 2, ACS_BLOCK | COLOR_PAIR(color));        \
    mvwaddch(GAME.win, y, x * 2 + 1, ACS_BLOCK | COLOR_PAIR(color));    \
} while(0)

int const FIGURES[8][4] = {
    {},
    {1, 3, 5, 7}, // I
    {2, 4, 5, 7}, // S
    {3, 5, 4, 6}, // Z
    {3, 5, 7, 6}, // J
    {2, 4, 6, 7}, // L
    {2, 3, 4, 5}, // O
    {2, 4, 5, 6}  // T
};

struct Point {
    int x, y;
};

struct Figure {
   int type;
   struct Point next[4];
   struct Point prev[4];
};

void init_game();
void create_fig();
void process_events();
void update();
void move_fig();
void rotate_fig();
void tick();
int is_collide(struct Point a[]);
void check_remove_line();
void render();
void end_game();

struct GameState {
    WINDOW *win;
    int field[H][W];
    int gameOver;
    struct Figure curFig;
    int dx;
    int rotated;
    int tick;
    int figExists;
    int force;
} GAME;

int main()
{
    init_game();

    double delay = 0.3, t = sec();
    while (!GAME.gameOver) {
	if (sec() - t > delay) {
	    GAME.tick = 1;
	    t = sec();
	}
	process_events();
	update();
	render();
    }

    end_game();

    return 0;
}

void init_game()
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    //assume_default_colors(COLOR_BLACK, COLOR_WHITE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    GAME.win = newwin(H, W * 2, 0, 0);
    box(GAME.win, 0, 0);
    wtimeout(GAME.win, 0);
    srand(time(NULL));
}


void process_events()
{
    switch(wgetch(GAME.win)) {
    case 'a':
	GAME.dx = -1;
	break;
    case 'd':
	GAME.dx = +1;
	break;
    case 's':
	GAME.force = 1;
	break;
    case 'r':
	GAME.rotated = 1;
	break;
    case '\n':
	exit(0);
	break;
    }
}

void update()
{
    if (!GAME.figExists) {
	create_fig();
    }
    if (GAME.dx) {
	move_fig();
	GAME.dx = 0;
    }
    if (GAME.rotated) {
	rotate_fig();
	GAME.rotated = 0;
    }
    if (GAME.tick) {
	tick();
	GAME.force = 0;
	GAME.tick = 0;
    }
}

void create_fig()
{
    struct Figure *pCurFig = &GAME.curFig;
    pCurFig->type = 1 + rand() % 7;
    for (int i = 0; i < 4; ++i) {
	pCurFig->next[i].x = FIGURES[pCurFig->type][i] % 2;
	pCurFig->next[i].y = FIGURES[pCurFig->type][i] / 2;
	pCurFig->next[i].x += 4; //offset
	pCurFig->next[i].y -= 3; //offset
	pCurFig->prev[i] = pCurFig->next[i];
    }
    GAME.figExists = 1;
}

void move_fig()
{
    struct Figure *pCurFig = &GAME.curFig;
    for (struct Point *it = pCurFig->next; it < pCurFig->next + 4; ++it) {
	it->x += GAME.dx;
    }
    if (is_collide(pCurFig->next)) {
	memcpy(pCurFig->next, pCurFig->prev, sizeof pCurFig->prev);
    } else {
	memcpy(pCurFig->prev, pCurFig->next, sizeof pCurFig->next);
    }
}

void rotate_fig()
{
    struct Figure *pCurFig = &GAME.curFig;
    struct Point pivot = pCurFig->next[1];
    for (int i = 0; i < 4; ++i) {
	int x = pCurFig->next[i].y - pivot.y;
	int y = pCurFig->next[i].x - pivot.x;
	pCurFig->next[i].x = pivot.x - x;
	pCurFig->next[i].y = pivot.y + y;
    }
    if (is_collide(pCurFig->next)) {
	memcpy(pCurFig->next, pCurFig->prev, sizeof pCurFig->prev);
    } else {
	memcpy(pCurFig->prev, pCurFig->next, sizeof pCurFig->next);
    }
}

void tick()
{
    struct Figure *pCurFig = &GAME.curFig;
    int collision = 0, negative = 0;

    for (int i = 0; i < 4; ++i) {
	pCurFig->next[i].y += 1;
	if (pCurFig->next[i].y < 0)
	    negative = 1;
    }

    collision = is_collide(pCurFig->next);

    if (negative && collision) {
	GAME.gameOver = 1;
	return;
    }

    while (GAME.force && !collision) {
	memcpy(pCurFig->prev, pCurFig->next, sizeof pCurFig->next);
	for (int i = 0; i < 4; ++i) {
	    pCurFig->next[i].y += 1;
	}
	collision = is_collide(pCurFig->next);
    }

    if (collision) {
	for (int i = 0; i < 4; ++i) {
	    int y = pCurFig->prev[i].y, x = pCurFig->prev[i].x;
	    GAME.field[y][x] = pCurFig->type;
	}
	GAME.figExists = 0;
	check_remove_line();
    } else {
	memcpy(pCurFig->prev, pCurFig->next, sizeof pCurFig->next);
    }
}

int is_collide(struct Point a[])
{
    for (int i = 0; i < 4; ++i) {
	int y = a[i].y;
	int x = a[i].x;
	if (y < 0) continue;
	if (x < 0 || x >= W || y >= H || GAME.field[y][x])
	    return 1;
    }
    return 0;
}

void check_remove_line()
{
    for (int y = 0; y < H; ++y) {
	int x = 0;
	for (; x < W; ++x) {
	    if (GAME.field[y][x] == 0) break;
	}
	if (x != W) continue;
	for (int k = y; k > 0; --k) {
	    int sum = 0;
	    for (int l = 0; l < W; ++l) {
		sum += GAME.field[k-1][l];
		GAME.field[k][l] = GAME.field[k-1][l];
	    }
	    if (sum == 0) break;
	}
    }
}

void render()
{
    // draw field
    for (int y = 0; y < H; ++y) {
	for (int x = 0; x < W; ++x) {
	    draw_block(y, x, GAME.field[y][x]);
	}
    }
    // draw figure
    struct Figure *pCurFig = &GAME.curFig;
    for (struct Point *it = pCurFig->next; it < pCurFig->next + 4; ++it) {
	draw_block(it->y, it->x, pCurFig->type);
    }

    wrefresh(GAME.win);
}


void end_game()
{
    char *msg = "GAME OVER!!!";

    wattron(GAME.win, A_REVERSE | A_BOLD);
    mvwprintw(GAME.win, H/2, (W * 2 - strlen(msg)) / 2, msg);
    wattroff(GAME.win, A_REVERSE | A_BOLD);

    wrefresh(GAME.win);

    wtimeout(GAME.win, -1);
    wgetch(GAME.win);
    endwin();
}
