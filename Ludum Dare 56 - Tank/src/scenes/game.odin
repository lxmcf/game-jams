package scenes

import "core:encoding/json"
import "core:fmt"
import "core:math"
import "core:math/rand"
import "core:os"

import "../assets"
import ls "shared:lspx"
import rl "vendor:raylib"

Entity_Fish :: struct {
    position:   rl.Vector2,
    velocity:   rl.Vector2,
    size:       rl.Vector2,
    sprite:     string,
    is_rare:    bool,
    dragging:   bool,
    move_timer: f32,
    age:        f32,
    age_speed:  f32,
}

@(private = "file")
Game_Context :: struct {
    fish:        [dynamic]Entity_Fish,
    tank_bounds: rl.Rectangle,
    money:       int,
    max_fish:    int,
}

@(private = "file")
Fish_Info :: struct {
    price_buy:        int,
    price_sell:       int,
    rarity_factor:    int,
    speed_factor:     f32,
    speed_factor_age: f32,
}

@(private = "file")
Game_Internal :: struct {
    camera:               rl.Camera2D,
    mouse_world_position: rl.Vector2,
    ambiance_timer:       f32,
    fish_lookup:          map[string]Fish_Info,
}

@(private = "file")
ctx: Game_Context

@(private = "file")
internal: Game_Internal

init_game :: proc() {
    global_tick_counter = 0
    next_scene = .Game

    ctx.fish = make([dynamic]Entity_Fish)

    if fish_data, ok := os.read_entire_file("fish.json", context.temp_allocator); ok {
        json.unmarshal(fish_data, &internal.fish_lookup, allocator = context.temp_allocator)
    }

    if save_data, ok := os.read_entire_file("save.json", context.temp_allocator); ok {
        json.unmarshal(save_data, &ctx, allocator = context.temp_allocator)
    } else {
        ctx.money = 20
        ctx.tank_bounds = {0, 0, 1280, 720}
        ctx.max_fish = 18
    }

    internal.camera.zoom = 1
    internal.camera.offset = {f32(rl.GetRenderWidth()), f32(rl.GetRenderHeight())} / 2
    internal.camera.target = {ctx.tank_bounds.width, ctx.tank_bounds.height} / 2

    internal.ambiance_timer = math.round(rand.float32_range(15, 45))
}

update_game :: proc() -> Game_Scene {
    global_tick_counter += rl.GetFrameTime()
    internal.mouse_world_position = rl.GetScreenToWorld2D(rl.GetMousePosition(), internal.camera)

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
        internal.camera.offset = rl.GetMousePosition()
        internal.camera.target = internal.mouse_world_position

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

    for &fish, index in ctx.fish {
        @(static)
        drag_offset: rl.Vector2

        if !fish.dragging {
            update_fish(&fish)

            if rl.Vector2Distance(fish.position, internal.mouse_world_position) <= fish.size.y / 2 {
                if rl.IsMouseButtonPressed(.LEFT) {
                    fish.dragging = true

                    drag_offset = internal.mouse_world_position - fish.position
                    rl.PlaySound(assets.sounds["water_2"])
                }
            }
        } else {
            fish.position = internal.mouse_world_position - drag_offset

            if rl.IsMouseButtonReleased(.LEFT) {

                if rl.CheckCollisionPointRec(internal.mouse_world_position, ctx.tank_bounds) {
                    rl.PlaySound(assets.sounds["water_1"])
                    fish.dragging = false
                } else {
                    ctx.money += sell_fish(&fish)

                    rl.PlaySound(assets.sounds["sell_fish"])
                    unordered_remove(&ctx.fish, index)
                }
            }
        }
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

    if rl.CheckCollisionPointRec(world_mouse, ctx.tank_bounds) && rl.IsWindowFocused() {
        tank_alpha = math.lerp(tank_alpha, 0, rl.GetFrameTime() * 10)
    } else {
        tank_alpha = math.lerp(tank_alpha, 1, rl.GetFrameTime() * 10)
    }

    ls.DrawSpriteNineSlice("tank_glass_frame", ctx.tank_bounds, rl.Fade(rl.WHITE, 1 - tank_alpha))
    ls.DrawSpriteNineSlice("tank_glass", ctx.tank_bounds, rl.Fade(rl.WHITE, tank_alpha))
    rl.EndMode2D()

    if rl.IsWindowFocused() {
        draw_game_ui()
    }
}

unload_game :: proc() {
    delete(ctx.fish)
    delete(internal.fish_lookup)
}

@(private = "file")
create_fish :: proc(fish_name: string, position: rl.Vector2) -> Entity_Fish {
    lookup := internal.fish_lookup[fish_name]

    fish: Entity_Fish
    fish.sprite = fish_name
    fish.position = position
    fish.move_timer = 1
    fish.is_rare = rand.int_max(100) == 99
    fish.size = ls.GetSpriteOrigin(fish.sprite)
    fish.age = 0.1
    fish.age_speed = 40
    fish.is_rare = rand.int_max(lookup.rarity_factor) == 2

    return fish
}

@(private = "file")
update_fish :: proc(fish: ^Entity_Fish) {
    lookup := internal.fish_lookup[fish.sprite]

    fish.move_timer -= rl.GetFrameTime()

    if fish.age < 1 {
        fish.age += (rl.GetFrameTime() / 50) * lookup.speed_factor_age
    } else {
        fish.age = 1
    }

    size := fish.size * fish.age

    if fish.move_timer <= 0 {
        fish.velocity = rl.Vector2Normalize({rand.float32_range(-1, 1), rand.float32_range(-1, 1)})

        fish.move_timer = rand.float32_range(2.5, 10)
    }

    if fish.position.x <= size.x || fish.position.x >= ctx.tank_bounds.width - (size.x * 2) {
        fish.velocity.x *= -1
        fish.move_timer += 2
    }

    if fish.position.y <= size.y || fish.position.y >= ctx.tank_bounds.height - 96 {
        fish.velocity.y *= -1
        fish.move_timer += 2
    }

    fish.position += ((fish.velocity * rl.GetFrameTime()) * 40) * lookup.speed_factor

    fish.position.x = clamp(fish.position.x, size.x, ctx.tank_bounds.width - (size.x * 2))
    fish.position.y = clamp(fish.position.y, size.y, ctx.tank_bounds.height - 96)
}

@(private = "file")
sell_fish :: proc(fish: ^Entity_Fish) -> int {
    price_factor: f32 = 1
    lookup := internal.fish_lookup[fish.sprite]

    price_factor *= fish.age
    price_factor *= fish.is_rare ? 2.5 : 1

    return int(f32(lookup.price_sell) * price_factor)
}

@(private = "file")
draw_fish :: proc(fish: Entity_Fish) {
    scale: f32 = fish.dragging ? 1.1 : 1

    ls.DrawSpritePro(fish.sprite, fish.position, rl.Vector2(fish.age * scale), 0, fish.velocity.x > 0 ? {} : {.Flip_Horizontal}, fish.is_rare ? rl.GOLD : rl.WHITE)

    when ODIN_DEBUG {
        rl.DrawCircleLinesV(fish.position, fish.size.y / 2, rl.RED)
        rl.DrawLineV(fish.position, fish.position + fish.velocity * 64, rl.GREEN)
    }
}

@(private = "file")
draw_game_ui :: proc() {
    BUTTON_SIZE :: 64
    BUTTON_PADDING :: 16

    @(static)
    button_x: f32 = -(BUTTON_SIZE - BUTTON_PADDING)

    rl.DrawTextEx(assets.fonts["game_button"], rl.TextFormat("Coins: %d", ctx.money), {BUTTON_PADDING, BUTTON_PADDING}, 48, 2, rl.WHITE)

    fish_count := rl.TextFormat("Fish: %d/%d", len(ctx.fish), ctx.max_fish)

    width := rl.MeasureTextEx(assets.fonts["game_button"], fish_count, 48, 2)
    rl.DrawTextEx(assets.fonts["game_button"], fish_count, {f32(rl.GetRenderWidth()) - (width.x + BUTTON_PADDING), BUTTON_PADDING}, 48, 2, rl.WHITE)

    mouse := rl.GetMousePosition()
    button_x = math.lerp(button_x, mouse.x <= BUTTON_SIZE + (BUTTON_PADDING * 2) ? BUTTON_PADDING : -(BUTTON_SIZE - BUTTON_PADDING), rl.GetFrameTime() * 10)

    for i in 0 ..< len(internal.fish_lookup) {
        fish := fmt.tprint("fish_", i + 1, sep = "")
        lookup := internal.fish_lookup[fish]

        if buy_fish_button(fish, {button_x, (BUTTON_SIZE + BUTTON_PADDING) * f32(i + 1), BUTTON_SIZE, BUTTON_SIZE}) {
            if ctx.money >= lookup.price_buy && len(ctx.fish) < ctx.max_fish {
                ctx.money -= lookup.price_buy

                spawn_bounds: rl.Vector2 = {ctx.tank_bounds.width / 3, ctx.tank_bounds.height / 3}

                position: rl.Vector2 = {rand.float32_range(spawn_bounds.x, spawn_bounds.x * 2), rand.float32_range(spawn_bounds.y, spawn_bounds.y * 2)}

                append(&ctx.fish, create_fish(fish, position))

                rl.PlaySound(assets.sounds["water_1"])
            } else {
                rl.PlaySound(assets.sounds["decline"])
            }
        }
    }
}

@(private = "file")
buy_fish_button :: proc(fish: string, bounds: rl.Rectangle, text_alpha: f32 = 1) -> bool {
    lookup := internal.fish_lookup[fish]

    result := false

    rl.DrawRectangleRounded(bounds, 0.25, 30, {33, 52, 69, 192})
    rl.DrawRectangleRoundedLines(bounds, 0.25, 30, 4, rl.Fade(rl.DARKBLUE, 0.5))

    ls.DrawSpriteEx(fish, {bounds.x + (bounds.width / 2), bounds.y + (bounds.height / 2)}, 0.5, 0, ctx.money >= lookup.price_buy ? rl.WHITE : {0, 0, 0, 128})

    if rl.CheckCollisionPointRec(rl.GetMousePosition(), bounds) && ctx.money >= lookup.price_buy {
        rl.DrawTextEx(assets.fonts["game_button"], assets.dictionary_lookup(fmt.tprint("$", lookup.price_buy, sep = "")), {bounds.x + bounds.width + 8, bounds.y + 8}, 48, 2, rl.WHITE)

        if rl.IsMouseButtonReleased(.LEFT) {
            result = true
        }
    }

    return result
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
