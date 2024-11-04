#include "raylib.h"

#define INTERP 0
#define CELL_SIZE 32
#define UPDATE_INTERVAL 0.25f

#define GOOD_COLOR GREEN
#define BAD_COLOR ORANGE
#define SNAKE_COLOR GRAY
#define MAP_COLOR DARKGRAY
#define OUTLINE_COLOR WHITE

#define zero_array(arr, len) for (int i = 0; i < len; i++) {arr[i] = 0;}

typedef struct iVector2 {
	int x, y;
} iVector2;

typedef struct Snake {
	int len, cap;
	int * segments;
	iVector2 dir;
} Snake;

typedef struct Game_State {
	enum { PLAY, GAMEOVER, WIN } state;
	float timer;

	int * map;
	int map_w, map_h, map_len;
	int food;

	iVector2 next_dir;
	Snake snake;
} Game_State;

void init_game(Game_State* game, int w, int h) {
	int map_len = w*h;
	int* map = MemRealloc(game->map, sizeof(int) * map_len);
	int* segments = MemRealloc(game->snake.segments, sizeof(int) * map_len);
	if (!map && !segments) {
		*game = (Game_State){0};
	}

	zero_array(map, map_len);
	*game = (Game_State) {
		.map_len = map_len,
		.map = map,
		.snake = {
			.cap = map_len,
			.segments = segments, 
			.len = 3
		},
		.state = PLAY
	};
	
	game->snake.segments[0] = w * (h/2) + (w/2);
	game->snake.segments[1] = game->snake.segments[0] - w;
	game->snake.segments[2] = game->snake.segments[1] - w;
	for (int s = 0; s < game->snake.len; s++) game->map[game->snake.segments[s]] = 1;
	
	int food = game->snake.segments[0];
	while (game->map[ (food = GetRandomValue(0, map_len-1)) ] != 0);
	game->map[food] = 2;
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
	if (!game.snake.segments) {
		TraceLog(LOG_FATAL, "Failed to allocate snake segments.\n");
		return 1;
	}

	while(!WindowShouldClose()) {
		Snake* snake = &game.snake;
		switch (game.state) {
			case PLAY:
				game.timer -= GetFrameTime() * (float)(game.timer > 0);

				if (IsKeyDown(KEY_W)) game.next_dir = (iVector2){ 0,-1};
				if (IsKeyDown(KEY_A)) game.next_dir = (iVector2){-1, 0};
				if (IsKeyDown(KEY_S)) game.next_dir = (iVector2){ 0, 1};
				if (IsKeyDown(KEY_D)) game.next_dir = (iVector2){ 1, 0};

				if (game.timer <= 0) {
					if (game.next_dir.x || game.next_dir.y) {
						int new_dir_index = 
							snake->segments[snake->len-1]
							+ game.next_dir.x
							+ (game.next_dir.y * map_w);

						if (new_dir_index != snake->segments[snake->len-2]) {
							snake->dir = game.next_dir;
						}
						game.next_dir = (iVector2){0};
					}

					if (snake->dir.x || snake->dir.y) {
						int x = snake->segments[snake->len-1] % map_w;
						int y = (snake->segments[snake->len-1] - x) / map_w;
						// Move
						x += snake->dir.x;
						y += snake->dir.y;

						int new_index = (y * map_w) + x;
						bool collision = 
							game.map[new_index] == 1 ||
							!CheckCollisionPointRec((Vector2){x,y}, (Rectangle){0,0, map_w, map_h});

						if (collision) {
							game.state = GAMEOVER;
						} else if (game.map[new_index] == 2) {
							snake->segments[snake->len++] = new_index;
								game.map[new_index] = 1;
								while (game.map[ (game.food = GetRandomValue(0, map_len)) ] != 0){}
						} else {
							zero_array(game.map, game.map_len);
							game.map[game.food] = 2;

							for (int i = 0; i < snake->len-1; i++) {
								snake->segments[i] = snake->segments[i+1];
								game.map[snake->segments[i]] = 1;
							}

							snake->segments[snake->len-1] = new_index;
						}

					}

					game.timer = UPDATE_INTERVAL;
					if (snake->len == game.map_len) {
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

#if INTERP
		float t = 0.5f-(game.timer/UPDATE_INTERVAL);
		t *= (float)(snake->dir.x || snake->dir.y);
		int x = snake->segments[0] % map_w;
		int y = (snake->segments[0] - x) / map_w;
		int next_x = x, next_y = y;
		for (int i = 0; i < snake->len; i++) {
			if (i < snake->len-1) {
				next_x = snake->segments[i+1] % map_w;
				next_y = (snake->segments[i+1] - next_x) / map_w;
			} else {
				next_x = x + snake->dir.x;
				next_y = y + snake->dir.y;
			}
			float fx = (float)x + t*(float)(next_x-x);
			float fy = (float)y + t*(float)(next_y-y);
			Color fill_color = 
				(i == snake->len-1) 
				? (game.state == GAMEOVER)
					? BAD_COLOR
					: GOOD_COLOR
				: SNAKE_COLOR;

			DrawRectangle(fx*CELL_SIZE, fy*CELL_SIZE, CELL_SIZE, CELL_SIZE, fill_color);
			DrawRectangleLines(fx*CELL_SIZE, fy*CELL_SIZE, CELL_SIZE, CELL_SIZE, OUTLINE_COLOR);

			x = next_x;
			y = next_y;
		}
#else
		int x,y;
		for (int i = 0; i < snake->len; i++) {
			x = snake->segments[i] % map_w;
			y = (snake->segments[i] - x) / map_w;
			Color fill_color = 
				(i == snake->len-1) 
				? (game.state == GAMEOVER)
					? BAD_COLOR
					: GOOD_COLOR
				: SNAKE_COLOR;

			DrawRectangle(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, fill_color);
			DrawRectangleLines(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, OUTLINE_COLOR);
		}
#endif
		x = game.food % map_w;
		y = (game.food - x) / map_w;
		DrawCircle((x*CELL_SIZE)+CELL_SIZE/2.0f, (y*CELL_SIZE)+CELL_SIZE/2.0f, CELL_SIZE/3.0f, GOOD_COLOR);

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

		int score = snake->len-3;
		const char* score_text = TextFormat("Score: %d", score);
		width = MeasureText(score_text, font_size);

		DrawText(score_text, (GetScreenWidth()/2) - (width/2), GetScreenHeight() - font_size, font_size, OUTLINE_COLOR);

		EndDrawing();
	}
}
