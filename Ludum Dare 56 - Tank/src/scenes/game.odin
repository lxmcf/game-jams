package scenes

import "core:encoding/json"
import "core:math"
import "core:math/rand"
import "core:os"

import "../assets"
import ls "shared:lspx"
import rl "vendor:raylib"

Move_Method :: enum {
    Jolt,
    Smooth,
    Random,
}

Fish :: struct {
    sprite:      string,
    position:    rl.Vector2,
    velocity:    rl.Vector2,
    move_method: Move_Method,
    move_timer:  f32,
    is_rare:     bool,
}

@(private = "file")
Game_Context :: struct {
    fish:        [dynamic]Fish,
    tank_bounds: rl.Rectangle,
}

@(private = "file")
Game_Internal :: struct {
    camera:         rl.Camera2D,
    ambiance_timer: f32,
}

@(private = "file")
ctx: Game_Context

@(private = "file")
internal: Game_Internal

init_game :: proc() {
    global_tick_counter = 0
    next_scene = .Game

    ctx.fish = make([dynamic]Fish)
    ctx.tank_bounds = {0, 0, 1280, 720}

    if save_data, ok := os.read_entire_file("save.json", context.temp_allocator); ok {
        json.unmarshal(save_data, &ctx, allocator = context.temp_allocator)
    } else {
        for _ in 0 ..< 8 {
            append(&ctx.fish, create_random_fish())
        }
    }

    internal.camera.zoom = 1
    internal.camera.offset = {}
    internal.camera.target = {}

    internal.ambiance_timer = math.round(rand.float32_range(15, 45))
}

update_game :: proc() -> Game_Scene {
    global_tick_counter += rl.GetFrameTime()

    if len(ctx.fish) > 0 {
        internal.ambiance_timer -= rl.GetFrameTime()
        if internal.ambiance_timer <= 0 {
            internal.ambiance_timer = math.round(rand.float32_range(15, 45))

            sounds := []string{"water_1", "water_2", "water_3"}
            rl.PlaySound(assets.sounds[rand.choice(sounds[:])])
        }
    }

    if rl.IsMouseButtonDown(.MIDDLE) || rl.IsKeyDown(.LEFT_ALT) {
        delta := rl.GetMouseDelta()

        delta *= -1.0 / internal.camera.zoom
        internal.camera.target += delta
    }

    @(static)
    target_zoom: f32 = 1

    mouse_wheel := rl.GetMouseWheelMove()
    if mouse_wheel != 0 && !rl.IsMouseButtonDown(.MIDDLE) {
        mouse_world_position := rl.GetScreenToWorld2D(rl.GetMousePosition(), internal.camera)

        internal.camera.offset = rl.GetMousePosition()
        internal.camera.target = mouse_world_position

        scale_factor := 1 + (0.25 * abs(mouse_wheel))
        if mouse_wheel < 0 do scale_factor = 1.0 / scale_factor

        target_zoom = clamp(target_zoom * scale_factor, 0.125, 64)
    }

    internal.camera.zoom = math.lerp(internal.camera.zoom, target_zoom, rl.GetFrameTime() * 10)


    when ODIN_DEBUG {
        if rl.IsKeyDown(.LEFT_CONTROL) && rl.IsKeyReleased(.S) {
            rl.TraceLog(.INFO, "Game saved")

            save_game()
        }
    }

    for &fish in ctx.fish {
        update_fish(&fish)
    }

    if rl.IsKeyReleased(.ESCAPE) {
        target_zoom = 1
        next_scene = .Menu
    }

    return next_scene
}

draw_game :: proc() {
    rl.BeginMode2D(internal.camera)

    for fish in ctx.fish {
        draw_fish(fish)
    }

    top_left := rl.GetWorldToScreen2D({ctx.tank_bounds.x, ctx.tank_bounds.y}, internal.camera)
    bottom_right := rl.GetWorldToScreen2D({ctx.tank_bounds.x + ctx.tank_bounds.width, ctx.tank_bounds.y + ctx.tank_bounds.height}, internal.camera)

    scissor_width := i32(bottom_right.x - top_left.x)
    scissor_height := i32(bottom_right.y - top_left.y)

    rl.BeginScissorMode(i32(top_left.x), i32(top_left.y), scissor_width, scissor_height)
    size := ls.GetSpriteSize("sand_1")
    tiles_x := ctx.tank_bounds.width / size.x

    for i in 0 ..< i32(tiles_x + 1) {
        ls.DrawSprite("sand_1", {f32(i) * size.x, ctx.tank_bounds.y + ctx.tank_bounds.height + 32})
    }

    rl.EndScissorMode()

    world_mouse := rl.GetScreenToWorld2D(rl.GetMousePosition(), internal.camera)

    @(static)
    tank_alpha: f32 = 1

    if rl.CheckCollisionPointRec(world_mouse, ctx.tank_bounds) {
        tank_alpha = math.lerp(tank_alpha, 0, rl.GetFrameTime() * 10)
    } else {
        tank_alpha = math.lerp(tank_alpha, 1, rl.GetFrameTime() * 10)
    }

    ls.DrawSpriteNineSlice("tank_glass_frame", ctx.tank_bounds, rl.Fade(rl.WHITE, 1 - tank_alpha))
    ls.DrawSpriteNineSlice("tank_glass", ctx.tank_bounds, rl.Fade(rl.WHITE, tank_alpha))
    rl.EndMode2D()
}

unload_game :: proc() {
    delete(ctx.fish)
}

@(private = "file")
create_random_fish :: proc() -> Fish {
    fish_names := []string{"fish_1", "fish_2", "fish_3", "fish_4", "fish_5"}

    fish: Fish
    fish.sprite = rand.choice(fish_names[:])
    fish.move_method = .Jolt
    fish.position.x = rand.float32_range(64, ctx.tank_bounds.width - 128)
    fish.position.y = rand.float32_range(64, ctx.tank_bounds.height - 128)
    fish.move_timer = rand.float32_range(2.5, 10)
    fish.is_rare = rand.int_max(100) == 99

    return fish
}

@(private = "file")
update_fish :: proc(fish: ^Fish) {
    fish.move_timer -= rl.GetFrameTime()

    if fish.move_timer <= 0 {
        #partial switch fish.move_method {
        case .Jolt:
            fish.velocity = rl.Vector2Normalize({rand.float32_range(-1, 1), rand.float32_range(-1, 1)})
            break

        case .Smooth:
            break
        }

        fish.move_timer = rand.float32_range(2.5, 10)
    }

    size := ls.GetSpriteOrigin(fish.sprite)

    if fish.position.x <= size.x || fish.position.x >= ctx.tank_bounds.width - (size.x * 2) {
        fish.velocity.x *= -1
        fish.move_timer += 2
    }

    if fish.position.y <= size.y || fish.position.y >= ctx.tank_bounds.height - (size.y * 2) {
        fish.velocity.y *= -1
        fish.move_timer += 2
    }

    fish.position += (fish.velocity * rl.GetFrameTime()) * 40

    fish.position.x = clamp(fish.position.x, size.x, ctx.tank_bounds.width - (size.x * 2))
    fish.position.y = clamp(fish.position.y, size.y, ctx.tank_bounds.height - (size.y * 2))
}

@(private = "file")
draw_fish :: proc(fish: Fish) {
    ls.DrawSpritePro(fish.sprite, fish.position, rl.Vector2(1), 0, fish.velocity.x > 0 ? {} : {.Flip_Horizontal}, fish.is_rare ? rl.GOLD : rl.WHITE)

    when ODIN_DEBUG {
        rl.DrawLineV(fish.position, fish.position + fish.velocity * 64, rl.GREEN)
    }
}

@(private = "file")
save_game :: proc() {
    options: json.Marshal_Options = {
        use_spaces = true,
        pretty     = true,
        spaces     = 4,
    }
    if game_data, error := json.marshal(ctx, options, context.temp_allocator); error == nil {
        os.write_entire_file("save.json", game_data)
    }
}
