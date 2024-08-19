#include "raylib.h"

#define cell_size 16
#define UPDATE_RATE 0.25f/2

typedef struct iVector2 {
	int x, y;
} iVector2;

struct snake {
	int len, cap;
	int * cells;
	iVector2 dir;
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
	int food = snake.cells[0];
	while (food == snake.cells[0]) {
		food = GetRandomValue(0, map_len-1);
	}

	float dt = 0;
	float update_timer = UPDATE_RATE;

	while(!WindowShouldClose()) {
		dt = GetFrameTime();
		update_timer -= dt * (float)(update_timer > 0);

		iVector2 new_dir = {
			.x = (int)IsKeyDown(KEY_D) - (int)IsKeyDown(KEY_A),
			.y = (int)IsKeyDown(KEY_S) - (int)IsKeyDown(KEY_W),
		};
		// Enforce 4 directions prioritizing horizontal axis
		new_dir.y *= (int)(new_dir.x == 0);
		if ( (new_dir.x && !snake.dir.x) || (new_dir.y && !snake.dir.y) ) {
			snake.dir = new_dir;
		}

		BeginDrawing();
		ClearBackground(DARKGRAY);

		for (int i = 0; i < snake.len; i++) {
			if (i == snake.len-1) {
				if ((snake.dir.x||snake.dir.y) && update_timer <= 0) {
					int x = snake.cells[snake.len-1] % map_w;
					int y = (snake.cells[snake.len-1] - x) / map_w;

					x += snake.dir.x;
					y += snake.dir.y;
					
					// Screen wrap
					x = (x + (int)(x < 0)*map_w) % map_w;
					y = (y + (int)(y < 0)*map_h) % map_h;
					int new_index = (y * map_w) + x;

					if (new_index == food) {
						snake.cells[snake.len++] = new_index;
						i++; // Avoid moving newly grown snake segment
						
						food = GetRandomValue(0, map_len);
						while (true) {
							bool valid = true;
							for (int j=0; j<snake.len;j++) {
								if (snake.cells[j] == food) {
									valid = false;
									break;
								}
							}
							
							if (valid) {
								break;
							}
						}
					} else {
						snake.cells[snake.len-1] = new_index;
					}
				}
			} else if (update_timer <= 0) {
				snake.cells[i] = snake.cells[i+1];
			}

			// Draw
			int x = snake.cells[i] % map_w;
			int y = (snake.cells[i] - x) / map_w;
			DrawRectangle(x*cell_size, y*cell_size, cell_size, cell_size, GRAY);
			DrawRectangleLines(x*cell_size, y*cell_size, cell_size, cell_size, WHITE);
		}

		int x = food % map_w;
		int y = (food - x) / map_w;
		DrawCircle((x*cell_size)+cell_size/2.0f, (y*cell_size)+cell_size/2.0f, cell_size/3.0f, GREEN);

		update_timer += (float)(update_timer <= 0) * UPDATE_RATE;
		EndDrawing();
	}
}
