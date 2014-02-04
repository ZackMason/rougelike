#include <ncurses.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#define MAP_WALL	'#'
#define MAP_DOOR	'+'
#define MAP_FLOOR	'.'
#define MAP_STAIR_UP	'>'
#define MAP_STAIR_DOWN	'<'
#define MAIN_PLAYER	'@'

int levelReached = 1;

typedef struct map_s {
    int		width, height;
    char	*data;
} map_t;

typedef struct player_s {
    char	ch;
    int		y;
    int		x;
} player_t;

typedef struct game_s {
    int		running;
    player_t	*player;
    map_t	*map;
    map_t	*oldMap;
} game_t;

void setMapTile(map_t *map, int y, int x, char ch)
{
    assert((x >= 0) && (x < map->width));
    map->data[y * map->width + x] = ch;
}

char getMapTile(map_t *map, int y, int x)
{
    return map->data[y * map->width + x];
}
/*
int getMapDistance(map_t *map, int y1, int x1, int y2, int x2) {
    int Y = y2 - y1;
    int X = x2 - x1;
    int D = powf(Y, 2) + powf(X, 2);
    return sqrt(D);
}
*/
int isWall(map_t *map, int y, int x)
{
    return getMapTile(map, y, x) == MAP_WALL;
}

int isDoor(map_t *map, int y, int x)
{
    return getMapTile(map, y, x) == MAP_DOOR;
}

void caveMap(game_t *game) {
    int y, x, n, m;
    int count;
    int i = 100;


    for (y = 0; y < game->map->height; ++y) {
	for (x = 0; x < game->map->width; ++x) {
	    char ch = MAP_WALL;
	    int r = rand() % 2;
	    if (r == 0) {
		setMapTile(game->oldMap, y, x, ch);
	    } else {
		setMapTile(game->oldMap, y, x, MAP_FLOOR);
	    }
	}
    }
    while(i > 0) {
	for (y = 0; y < game->oldMap->height; ++y) {
	    for (x = 0; x < game->oldMap->width; ++x) {
		count = 0;
		for (n = y - 1; n != y + 2; ++n) {
		    for (m = x - 1; m != x + 2; ++m) {
			if (getMapTile(game->oldMap, n, m) == MAP_WALL) {
			    ++count;
			}
		    }
		}
		if (count > 4) {
		    setMapTile(game->map, y, x, MAP_WALL);
		} else {
		    setMapTile(game->map, y, x, MAP_FLOOR);
		}
	    }
	}
	memcpy(game->oldMap->data, game->map->data,
	       game->map->width * game->map->height);
	--i;
    }
}


void fillMap(map_t *map)
{
    int x, y;

    for (y = 0; y < map->height; ++y) {
	for (x = 0; x < map->width; ++x) {
	    char ch = MAP_FLOOR;
	    if ((x == 0) || (x == (map->width - 1))
		|| (y == 0) || (y == (map->height - 1))) {
		ch = MAP_WALL;
	    }
	    setMapTile(map, y, x, ch);
	}
    }
}

void splitMap(map_t *map, int iterations)
{
    int x, y, splitX, splitY, lastY, lastX, i;
    char ch = MAP_WALL;
    i = iterations;
    splitX = map->width;
    splitY = map->height;

    while(i > 0) {

	if (i % 2 == 1) {
	    for (y = 0; y < splitY; ++y) {
		for (x = 0; x < splitX; ++x) {
		    if(x == splitX/2) {
			setMapTile(map, y, x, ch);
		    }
		    if (y == splitY - 1) {
			lastX = x;
		    }
		}
	    }
	    splitX = lastX;
	} else {
	    for (y = 0; y < splitY; ++y) {
		for (x = 0; x < splitX; ++x) {
		    if(y == splitY/2) {
			setMapTile(map, y, x, ch);
		    }
		}
		lastY = y;
	    }
	    splitY = lastY;
	}
	--i;
    }
 }



void drawMap(game_t *game)
{
    fillMap(game->map);
    caveMap(game);
    int placed = 0;
    game->player->ch = MAIN_PLAYER;
    while (placed == 0) {
	int RX = rand() % game->map->width;
	int RY = rand() % game->map->height;
	if (getMapTile(game->map, RY, RX) == MAP_FLOOR) {
	    setMapTile(game->map, RY, RX, MAP_STAIR_DOWN);
	    game->player->x = RX;
	    game->player->y = RY;
	    placed = 1;
	}
    }


    placed = 0;
    while (placed == 0) {
	int RX = rand() % game->map->width;
	int RY = rand() % game->map->height;
	if (getMapTile(game->map, RY, RX) == MAP_FLOOR) {
	    setMapTile(game->map, RY, RX, MAP_STAIR_UP);
	    placed = 1;
	}
    }
}


void updateScreen(game_t *game)
{
    int x, y;
    map_t *map = game->map;
    player_t *player = game->player;

    clear();
    /* Draw the map */
    for (y = 0; y < map->height; ++y) {
	for (x = 0; x < map->width; ++x) {
	    mvprintw(y, x, "%c", getMapTile(map, y, x));
	}
    }

    /* Draw the player */
    if (player) {
	mvprintw(player->y, player->x, "%c", player->ch);
    }

    if (getMapTile(game->map, game->player->y,
		   game->player->x) == MAP_STAIR_UP) {
	    drawMap(game);
	    ++levelReached;
    }
    refresh();
}



void processInput(game_t *game)
{
    int c = getch();

    switch (c) {
    case KEY_UP:
	if (getMapTile(game->map, game->player->y - 1,
		       game->player->x) != MAP_WALL) {
	    game->player->y -= 1;
	}
	break;

    case KEY_DOWN:
	if (getMapTile(game->map, game->player->y + 1,
		       game->player->x) != MAP_WALL) {
	    game->player->y += 1;
	}
	break;

    case KEY_LEFT:
	if (getMapTile(game->map, game->player->y,
		       game->player->x - 1) != MAP_WALL) {
	    game->player->x -= 1;
	}
	break;

    case KEY_RIGHT:
	if (getMapTile(game->map, game->player->y,
		       game->player->x + 1) != MAP_WALL) {
	    game->player->x += 1;
	}
	break;
    
    case KEY_BACKSPACE:
	game->running = 0;
	break;
    case KEY_C1:
	drawMap(game);
	break;
    }
}


void startup(game_t *game)
{
    initscr();
    keypad(stdscr, TRUE);
    drawMap(game);
    game->running = 1;
    curs_set(0);
}


int main()
{	
    int ch;
    game_t *game;
    srand(time(NULL));

    game = malloc(sizeof(game_t));
    if (!game) {
	fprintf(stderr, "could not alloc game\n");
	exit(1);
    }
    memset(game, 0, sizeof(game_t));

    game->map = malloc(sizeof(map_t));
    if (!game->map) {
	fprintf(stderr, "could not alloc map\n");
	exit(1);
    }
    memset(game->map, 0, sizeof(map_t));

    game->map->width = 150;
    game->map->height = 50;

    game->map->data = malloc(game->map->width * game->map->height);
    if (!game->map->data) {
	fprintf(stderr, "could not alloc map data\n");
	exit(1);
    }
    memset(game->map->data, 0, game->map->width * game->map->height);

    game->player = malloc(sizeof(player_t));
    if (!game->player) {
	fprintf(stderr, "could not alloc player\n");
	exit(1);
    }
    memset(game->player, 0, sizeof(player_t));

    game->oldMap = malloc(sizeof(map_t));
    if (!game->oldMap) {
	fprintf(stderr, "could not alloc old map\n");
	exit(1);
    }
    memset(game->oldMap, 0, sizeof(map_t));

    game->oldMap->width = game->map->width;
    game->oldMap->height = game->map->height;

    game->oldMap->data = malloc(game->oldMap->width * game->oldMap->height);
    if (!game->oldMap->data) {
	fprintf(stderr, "could not alloc old map data\n");
	exit(1);
    }
    memset(game->oldMap->data, 0, game->oldMap->width * game->oldMap->height);

    startup(game);


    while (game->running == 1)
    {
	updateScreen(game);
	processInput(game);
    }
    endwin();

    return(0);
}
