package main

import "core:math/rand"

import rl "vendor:raylib"

Background_Circle :: struct {
    position:     rl.Vector2,
    transparency: f32,
    radius:       f32,
    speed:        f32,
    fade_out:     bool,
    colour:       rl.Color,
}

create_background_circle :: proc() -> Background_Circle {
    circle: Background_Circle

    circle.radius = rand.float32_range(48, 128)
    circle.position = {rand.float32() * f32(rl.GetRenderWidth()), rand.float32() * f32(rl.GetRenderHeight())}
    circle.speed = rand.float32_range(0.1, 0.5)
    circle.colour = {39, 61, 81, 255}

    return circle
}

update_background_circle :: proc(circle: ^Background_Circle) {
    circle.position.y -= (64 * rl.GetFrameTime()) * circle.speed

    if !circle.fade_out {
        circle.transparency += rl.GetFrameTime() * circle.speed

        if circle.transparency >= 1 {
            circle.fade_out = true
        }
    } else {
        circle.transparency -= rl.GetFrameTime() * circle.speed

        if circle.transparency <= 0 {
            circle.fade_out = false
            circle.radius = rand.float32_range(48, 128)
            circle.speed = rand.float32_range(0.1, 0.5)
            circle.position = {rand.float32() * f32(rl.GetRenderWidth()), rand.float32() * f32(rl.GetRenderHeight())}
        }
    }

}

draw_background_circle :: proc(circle: Background_Circle) {
    rl.DrawCircleV(circle.position, circle.radius, rl.Fade(circle.colour, circle.transparency))
}
