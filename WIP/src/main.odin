package main

import "core:math"
import rl "vendor:raylib"

main :: proc() {
	rl.InitWindow(1280, 720, "")
	defer rl.CloseWindow()

	rl.SetTargetFPS(60)

	camera: rl.Camera2D
	camera.zoom = 1
	camera.offset = {640, 320}

	player_rotation: f32
	player_radius: f32 = 32

	for !rl.WindowShouldClose() {
		xaxis := i32(rl.IsKeyDown(.D)) - i32(rl.IsKeyDown(.A))

		player_rotation += (f32(xaxis) * 180) * rl.GetFrameTime()

		player_position: rl.Vector2 = {
			lengthdir_x(128 + 16 + player_radius, player_rotation),
			lengthdir_y(128 + 16 + player_radius, player_rotation),
		}

		camera.target = player_position
		camera.rotation = -player_rotation - 90

		rl.BeginDrawing()
		rl.ClearBackground(rl.WHITE)

		rl.BeginMode2D(camera)
		rl.DrawRing({}, 128, 128 + 16, 0, 360, 0, rl.GRAY)
		rl.DrawLineV({}, player_position, rl.GRAY)
		rl.DrawCircleV(player_position, player_radius, rl.DARKGRAY)
		rl.EndMode2D()

		rl.EndDrawing()
	}
}

lengthdir_x :: proc(len, dir_degrees: f32) -> f32 {
	rad := dir_degrees * (math.PI / 180.0)
	return len * math.cos(rad)
}

lengthdir_y :: proc(len, dir_degrees: f32) -> f32 {
	rad := dir_degrees * (math.PI / 180.0)
	return len * math.sin(rad)
}
