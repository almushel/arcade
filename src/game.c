#include "raylib.h"

#define CELL_SIZE 16
#define UPDATE_INTERVAL 0.25f

typedef struct iVector2 {
	int x, y;
} iVector2;

struct snake {
	int len, cap;
	int * segments;
	iVector2 dir;
};

int main(void) {
	const int map_w = 24;
	const int map_h = 24;
	const int map_len = map_w*map_h;

	InitWindow(map_w*CELL_SIZE, map_h*CELL_SIZE,"snaek");
	SetTargetFPS(60);

	int* map = MemAlloc(sizeof(int) * map_len);
	struct snake snake = {
		.cap = map_len,
		.segments = MemAlloc(sizeof(int) * map_len),
		.len = 3
	};
	snake.segments[0] = map_w * (map_h/2) + (map_w/2);
	snake.segments[1] = snake.segments[0] - map_w;
	snake.segments[2] = snake.segments[1] - map_w;
	for (int s = 0; s < snake.len; s++) map[snake.segments[s]] = 1;
	
	int food = snake.segments[0];
	while (map[ (food = GetRandomValue(0, map_len-1)) ] != 0);
	map[food] = 2;

	bool game_over = false;
	float update_timer = 0;
	iVector2 next_dir = {0};

	while(!WindowShouldClose()) {
		float dt = GetFrameTime();
		update_timer -= dt * (float)(update_timer > 0);

		if (IsKeyDown(KEY_W)) next_dir.y = -1;
		if (IsKeyDown(KEY_A)) next_dir.x = -1;
		if (IsKeyDown(KEY_S)) next_dir.y =  1;
		if (IsKeyDown(KEY_D)) next_dir.x =  1;

		if (!game_over && update_timer <= 0) {
			next_dir.y *= (int)(next_dir.x == 0);
			if ( (next_dir.x && !snake.dir.x) || (next_dir.y && !snake.dir.y) ) {
				snake.dir = next_dir;
			}
			next_dir = (iVector2){0};

			if (snake.dir.x || snake.dir.y) {
				int x = snake.segments[snake.len-1] % map_w;
				int y = (snake.segments[snake.len-1] - x) / map_w;
				// Move
				x += snake.dir.x;
				y += snake.dir.y;
				// Screen wrap
				x = (x + (int)(x < 0)*map_w) % map_w;
				y = (y + (int)(y < 0)*map_h) % map_h;

				int new_index = (y * map_w) + x;
				if (map[new_index] == 1) {
					game_over = true;
				} else if (map[new_index] == 2) {
					snake.segments[snake.len++] = new_index;
					map[new_index] = 1;
					while (map[ (food = GetRandomValue(0, map_len)) ] != 0){}
				} else {
					for (int m = 0; m < map_len; m++) map[m] = 0;
					map[food] = 2;

					for (int i = 0; i < snake.len-1; i++) {
						snake.segments[i] = snake.segments[i+1];
						map[snake.segments[i]] = 1;
					}

					snake.segments[snake.len-1] = new_index;
				}

			}
			update_timer = UPDATE_INTERVAL;
		}

		BeginDrawing();
		ClearBackground(DARKGRAY);
		int x, y;
		for (int i = 0; i < snake.len; i++) {
			x = snake.segments[i] % map_w;
			y = (snake.segments[i] - x) / map_w;
			Color fill_color = (game_over && i == snake.len-1) ? RED : GRAY;

			DrawRectangle(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, fill_color);
			DrawRectangleLines(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
		}

		x = food % map_w;
		y = (food - x) / map_w;
		DrawCircle((x*CELL_SIZE)+CELL_SIZE/2.0f, (y*CELL_SIZE)+CELL_SIZE/2.0f, CELL_SIZE/3.0f, GREEN);

		if (game_over) {
			const char* go_text = "GAME OVER";

			int font_size = GetScreenHeight()/8;
			int width;
			while((width = MeasureText(go_text, font_size)) >= GetScreenWidth()) {
				font_size /= 2;
			}

			DrawText(go_text, GetScreenWidth()/2 - width/2, GetScreenHeight()/2 - font_size/2, font_size, RED);
		}

		EndDrawing();
	}
}
