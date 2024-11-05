#include "raylib.h"

#define INTERP 0
#define CELL_SIZE 32
#define UPDATE_INTERVAL 0.25f

#define GOOD_COLOR GREEN
#define BAD_COLOR ORANGE
#define SNAKE_COLOR GRAY
#define MAP_COLOR DARKGRAY
#define OUTLINE_COLOR WHITE

typedef enum Tile_Types {
	SNAKE_HEAD = 1,
	SNAKE_UP,
	SNAKE_RIGHT,
	SNAKE_DOWN,
	SNAKE_LEFT,
	FOOD
} Tile_Types;

#define zero_array(arr, len) for (int i = 0; i < len; i++) {arr[i] = 0;}

typedef struct iVector2 {
	int x, y;
} iVector2;

typedef struct Game_State {
	enum { PLAY, GAMEOVER, WIN } state;
	float timer;

	int * map;
	int map_w, map_h, map_len, head, tail, score;

	iVector2 dir, next_dir;
} Game_State;

void init_game(Game_State* game, int w, int h) {
	int map_len = w*h;
	int* map = MemRealloc(game->map, sizeof(int) * map_len);
	if (!map) {
		*game = (Game_State){0};
	}

	zero_array(map, map_len);
	*game = (Game_State) {
		.map_len = map_len,
		.map_w = w, .map_h = h,
		.map = map,
		.state = PLAY
	};
	
	game->head = w * (h/2) + (w/2);
	game->tail = game->head - (2*w);
	game->map[game->head] = SNAKE_HEAD;
	game->map[game->head-w] = SNAKE_DOWN;
	game->map[game->tail] = SNAKE_DOWN;
	
	int food = game->head;
	while (game->map[ (food = GetRandomValue(0, map_len-1)) ] != 0);
	game->map[food] = FOOD;
}

Tile_Types ivec2_to_snake_dir(iVector2 dir) {
	if (dir.x == 1 && dir.y == 0){
		return SNAKE_RIGHT;
	}
	if (dir.x == -1 && dir.y == 0){
		return SNAKE_LEFT;
	}
	if (dir.y == 1 && dir.x == 0){
		return SNAKE_DOWN;
	}
	if (dir.y == -1 && dir.x == 0){
		return SNAKE_UP;
	}

	return 0;
}

iVector2 snake_dir_to_ivec2(Tile_Types dir) {
	iVector2 result = {0};
	switch(dir) {
		case SNAKE_DOWN: {
			result = (iVector2){0,1};
		} break;
		case SNAKE_UP: {
			result = (iVector2){0,-1};
		} break;
		case SNAKE_LEFT: {
			result = (iVector2){-1,0};
		} break;
		case SNAKE_RIGHT: {
			result = (iVector2){1,0};
		} break;

		default: break;
	}

	return result;
}


int main(void) {
	const int map_w = 16;
	const int map_h = 16;
	const int map_len = map_w*map_h;

	InitWindow(map_w*CELL_SIZE, map_h*CELL_SIZE,"snaek");
	SetTargetFPS(60);

	Game_State game = {0};
	init_game(&game, map_w, map_h);
	if (!game.map) {
		TraceLog(LOG_FATAL, "Failed to allocate game map.\n");
		return 1;
	}

	while(!WindowShouldClose()) {
		switch (game.state) {
			case PLAY:
				game.timer -= GetFrameTime() * (float)(game.timer > 0);

				if (IsKeyDown(KEY_W)) game.next_dir = (iVector2){ 0,-1};
				if (IsKeyDown(KEY_A)) game.next_dir = (iVector2){-1, 0};
				if (IsKeyDown(KEY_S)) game.next_dir = (iVector2){ 0, 1};
				if (IsKeyDown(KEY_D)) game.next_dir = (iVector2){ 1, 0};

				if (game.timer <= 0) {
					// Check for new input direction and update snake direction
					if (game.next_dir.x || game.next_dir.y) {
						// If nonzero value in both current and next dir x or y
						// input is either 180 degrees (invalid) or the same direction (redundant)
						// TODO: Fix for first move game over
						if (!(game.next_dir.x && game.dir.x) && !(game.next_dir.y && game.dir.y)) {
							game.dir = game.next_dir;
						}

						game.next_dir = (iVector2){0};
					}

					if (game.dir.x || game.dir.y) {
						int x = game.head % map_w;
						int y = (game.head - x) / map_w + game.dir.y;
						x += game.dir.x;

						int new_head = (y * game.map_w) + x;

						bool collision = 
							!CheckCollisionPointRec((Vector2){x,y}, (Rectangle){0,0, game.map_w, game.map_h}) ||
							(game.map[new_head] > 0 && game.map[new_head] != FOOD);

						if (collision) {
							game.state = GAMEOVER;
						} else if (game.map[new_head] == FOOD) {
							game.map[game.head] = ivec2_to_snake_dir(game.dir);
							game.map[new_head] = SNAKE_HEAD;
							game.head = new_head;
							
							int food = game.head;
							while (game.map[ (food = GetRandomValue(0, map_len)) ] != 0){}
							game.map[food] = FOOD;
							game.score++;
						} else {
							game.map[game.head] = ivec2_to_snake_dir(game.dir);
							game.map[new_head] = SNAKE_HEAD;
							game.head = new_head;

							iVector2 tail_dir = snake_dir_to_ivec2(game.map[game.tail]);
							game.map[game.tail] = 0;
							game.tail += (tail_dir.y * game.map_w) + tail_dir.x;
						}
					}

					game.timer = UPDATE_INTERVAL;
					if (game.score == game.map_len-2) {
						game.state = WIN;
					}
				}
				break;
			default:
				if (GetKeyPressed() != 0) {
					init_game(&game, map_w, map_h);
				}
				break;
		}

		BeginDrawing();
		ClearBackground(MAP_COLOR);

		int i = 0;
		for (int y = 0; y < game.map_h; y++) {
			for (int x = 0; x < game.map_w; x++) {
				Color fill_color;
				switch(game.map[i]) {
					case 0: {
						i++;
					} continue;

					case FOOD: {
						DrawCircle((x*CELL_SIZE)+CELL_SIZE/2.0f, (y*CELL_SIZE)+CELL_SIZE/2.0f, CELL_SIZE/3.0f, GOOD_COLOR);
					} break;

					default: {
						if (i == game.head) {
							fill_color = 
								(game.state == GAMEOVER) ?
								BAD_COLOR : GOOD_COLOR;
						} else {
							fill_color = SNAKE_COLOR;
						}

						DrawRectangle(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, fill_color);
						DrawRectangleLines(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, OUTLINE_COLOR);
					} break;
				}
				i++;
			}
		}

		int font_size = CELL_SIZE;
		int width;
		switch (game.state) {
			case GAMEOVER: {
				const char* go_text = "GAME OVER";
				width = MeasureText(go_text, font_size);
				DrawText(go_text, (GetScreenWidth()/2) - (width/2), (GetScreenHeight()/2) - (font_size/2), font_size, BAD_COLOR);

				const char* instructions = "Press any key to restart";
				width = MeasureText(instructions, font_size);
				DrawText(instructions,
					GetScreenWidth()/2 - width/2,
					GetScreenHeight()/2 - font_size/2+font_size,
					font_size, OUTLINE_COLOR
				);
			} break;
			
			case WIN: {
				const char* w_text = "You Win!";
				width = MeasureText(w_text, font_size);
				DrawText(w_text, (GetScreenWidth()/2) - (width/2), (GetScreenHeight()/2) - (font_size/2), font_size, GOOD_COLOR);

				const char* instructions = "Press any key to restart";
				width = MeasureText(instructions, font_size);
				DrawText(instructions,
					GetScreenWidth()/2 - width/2,
					GetScreenHeight()/2 - font_size/2+font_size,
					font_size, OUTLINE_COLOR
				);
			}

			default: break;
		}

		const char* score_text = TextFormat("Score: %d", game.score);
		width = MeasureText(score_text, font_size);

		DrawText(score_text, (GetScreenWidth()/2) - (width/2), GetScreenHeight() - font_size, font_size, OUTLINE_COLOR);

		EndDrawing();
	}
}
