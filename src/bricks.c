#include "raylib.h"
#include "math.h"

#define rect_pos(rect) (Vector2){rect.x, rect.y}
#define rect_dim(rect) (Vector2){rect.width, rect.height}

#define GRID_W 12
#define GRID_H 6
#define BALL_SPEED 240
#define PADDLE_SPEED 240

typedef struct Bricks_Game_State {
	int bricks[GRID_W*GRID_H];

	Rectangle paddle;

	bool ball_active;
	Vector2 ball_pos;
	Vector2 ball_dir;
	float ball_radius;
} Bricks_Game_State;

Vector2 Vector2Normalize(Vector2 v);

int main(void) {
	int width = 400;
	int height = 300;

	InitWindow(width, height, "Brick Breaker");

	float brick_w = (float)width/GRID_W;
	float brick_h = brick_w/2;
	Bricks_Game_State state = {
		.paddle = {
			.width = brick_w * 3,
			.height = brick_h/2.0f,
		},
		.ball_pos = {
			.x = (float)width/2.0f,
			.y = (float)height/2.0f + brick_h,
		},
		.ball_radius = brick_h/4,
	};
	
	state.paddle.x = ((float)width/2) - (state.paddle.width/2);
	state.paddle.y = height - (state.paddle.height);
	
	for (int i = 0; i < GRID_W*GRID_H; i++) {
		state.bricks[i] = 1;
	}

	while(!WindowShouldClose()){
		float dt = GetFrameTime();
		// Paddle movement
		if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
			state.paddle.x -= PADDLE_SPEED * dt;
		}
		if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
			state.paddle.x += PADDLE_SPEED * dt;
		}
		if (IsKeyDown(KEY_SPACE) && !state.ball_active) {
			float angle = PI * (float)GetRandomValue(250, 750) / 1000.0f;
			state.ball_dir = (Vector2) {
				.x = cosf(angle),
				.y = sinf(angle)
			};
			state.ball_active = true;
		}
		// Clamp paddle to screen
		if (state.paddle.x < 0) {
			state.paddle.x = 0;
		} else if (state.paddle.x > width - state.paddle.width) {
			state.paddle.x = width - state.paddle.width;
		}

		// Ball movement
		state.ball_pos = (Vector2){
			.x = state.ball_pos.x + state.ball_dir.x * BALL_SPEED * dt,
			.y = state.ball_pos.y + state.ball_dir.y * BALL_SPEED * dt,
		};
		// Ball/Paddle collision
		if (CheckCollisionCircleRec(state.ball_pos, state.ball_radius, state.paddle)) {
			//TODO: Improved ball control based on collision location
			state.ball_dir.y = -1;
		}
		// Constrain ball to screen
		if (state.ball_pos.x < state.ball_radius) {
			state.ball_dir.x = 1;
		} else if (state.ball_pos.x > width-state.ball_radius) {
			state.ball_dir.x = -1;
		}
		if (state.ball_pos.y < state.ball_radius) {
			state.ball_dir.y = 1;
		} else if (state.ball_pos.y > height) {
			state.ball_active = false;
			state.ball_dir = (Vector2){0};
			state.ball_pos = (Vector2){
				.x = (float)width/2.0f,
				.y = (float)height/2.0f + brick_h,
			};
		}

		BeginDrawing();
		ClearBackground(BLACK);

		for (int y = 0; y < GRID_H; y++) {
			for (int x = 0; x < GRID_W; x++) {
				int index = y*GRID_W+x; 
				if (state.bricks[index] == 0) { continue; }

				Rectangle brick_rect = {x*brick_w, y*brick_h, brick_w, brick_h};
				if (CheckCollisionCircleRec(state.ball_pos, state.ball_radius, brick_rect)) {
					state.bricks[index] = 0;
					// TODO: More accurate bounce based on brick side hit
					Vector2 delta = {
						.x = state.ball_pos.x - brick_rect.x,
						.y = state.ball_pos.y - brick_rect.y,
					};
					state.ball_dir = Vector2Normalize(delta);
				}

				DrawRectangleRec(brick_rect, BLUE);
				DrawRectangleLines(brick_rect.x, brick_rect.y, brick_rect.width, brick_rect.height, GREEN);
			}
		}
		DrawCircleV(state.ball_pos, state.ball_radius, WHITE);
		DrawRectangleV(rect_pos(state.paddle), rect_dim(state.paddle), GREEN);
			
		EndDrawing();
	}

	return 0;
}
