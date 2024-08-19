#include "raylib.h"

#define cell_size 16
#define UPDATE_RATE 0.25f

struct snake {
	int len, cap;
	int * cells;
	enum {DIR_NONE, DIR_UP, DIR_RIGHT,DIR_DOWN, DIR_LEFT} dir;
};

int main(void) {
	const int map_w = 24;
	const int map_h = 24;
	const int map_len = map_w*map_h;

	InitWindow(map_w*cell_size, map_h*cell_size,"snaek");
	SetTargetFPS(60);

	struct snake snake = {
		.cap = map_len,
		.cells = MemAlloc(sizeof(int) * map_len),
		.len = 1
	};
	snake.cells[0] = GetRandomValue(0, map_len-1);

	float dt = 0;
	float update_timer = UPDATE_RATE;

	while(!WindowShouldClose()) {
		dt = GetFrameTime();
		update_timer -= dt;

		if	(IsKeyDown(KEY_W)) snake.dir = DIR_UP;
		else if (IsKeyDown(KEY_A)) snake.dir = DIR_LEFT;
		else if (IsKeyDown(KEY_S)) snake.dir = DIR_DOWN;
		else if (IsKeyDown(KEY_D)) snake.dir = DIR_RIGHT;

		if (update_timer <= 0 && snake.dir) {
			int x = snake.cells[snake.len-1] % map_w;
			int y = (snake.cells[snake.len-1] - x) / map_w;

			switch(snake.dir) {
				case DIR_UP:
					y--;
					break;
				case DIR_DOWN:
					y++;
					break;
				case DIR_RIGHT:
					x++;
					break;
				case DIR_LEFT:
					x--;
					break;
				default:
					break;
			}

			// Screen wrap
			x = (x + (int)(x < 0)*map_w) % map_w;
			y = (y + (int)(y < 0)*map_h) % map_h;
			int new_index = (y * map_w) + x;

			snake.cells[snake.len-1] = new_index;
		}

		BeginDrawing();
		ClearBackground(DARKGRAY);

		for (int i = 0; i < snake.len; i++) {
			// Update
			if (i < snake.len-1 && update_timer <= 0) {
				snake.cells[i] = snake.cells[i+1];
			}

			// Draw
			int x = snake.cells[i] % map_w;
			int y = (snake.cells[i] - x) / map_w;
			DrawRectangle(x*cell_size, y*cell_size, cell_size, cell_size, GRAY);
			DrawRectangleLines(x*cell_size, y*cell_size, cell_size, cell_size, WHITE);
		}

		update_timer += (float)(update_timer <= 0) * UPDATE_RATE;
		EndDrawing();
	}
}
