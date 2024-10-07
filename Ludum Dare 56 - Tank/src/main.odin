package main

import "assets"
import "scenes"
import ls "shared:lspx"
import rl "vendor:raylib"

import "core:fmt"
import "core:mem"
_ :: mem
_ :: fmt

FPS_MINIMUM :: 60

Main_Context :: struct {
    transition_scale:    f32,
    transition_in:       bool,
    transition_out:      bool,
    transition_active:   bool,
    window_should_close: bool,
    current_scene:       scenes.Game_Scene,
    tick_counter:        f32,
    circles:             [10]Background_Circle,
}

ctx: Main_Context

main :: proc() {
    when ODIN_DEBUG {
        track: mem.Tracking_Allocator
        mem.tracking_allocator_init(&track, context.allocator)
        context.allocator = mem.tracking_allocator(&track)

        defer {
            if len(track.allocation_map) > 0 {
                fmt.eprintfln("<------ %v leaked allocations ------>", len(track.allocation_map))
                for _, entry in track.allocation_map do fmt.eprintfln("%v leaked %v bytes", entry.location, entry.size)
            }

            if len(track.bad_free_array) > 0 {
                fmt.eprintfln("<------ %v bad frees          ------>", len(track.bad_free_array))
                for entry in track.bad_free_array do fmt.eprintfln("%v bad free", entry.location)
            }

            mem.tracking_allocator_destroy(&track)
        }
    }

    rl.InitWindow(1280, 720, "Ludum Dare 56 | Tank")
    defer rl.CloseWindow()

    rl.InitAudioDevice()
    defer rl.CloseAudioDevice()

    max_fps := rl.GetMonitorRefreshRate(rl.GetCurrentMonitor())
    rl.SetTargetFPS(max_fps <= 0 ? FPS_MINIMUM : max_fps)

    ctx.current_scene = .Menu
    init_current_scene()
    defer unload_current_scene()

    assets.load_assets()
    defer assets.unload_assets()

    for i in 0 ..< len(ctx.circles) {
        ctx.circles[i] = create_background_circle()
    }

    if assets.sprites_loaded {
        rl.SetExitKey(.KEY_NULL)
        rl.HideCursor()
    }

    for !rl.WindowShouldClose() && !ctx.window_should_close {
        for &circle in ctx.circles {
            update_background_circle(&circle)
        }

        update_current_scene()

        rl.BeginDrawing()
        defer rl.EndDrawing()

        for circle in ctx.circles {
            draw_background_circle(circle)
        }

        if !assets.sprites_loaded {
            show_no_bundle_screen()
            continue
        }

        draw_current_scene()
        ls.DrawSprite("pointer", rl.GetMousePosition())

        when ODIN_DEBUG {
            debug_draw_fps()
        }
    }
}

init_current_scene :: #force_inline proc() {
    #partial switch ctx.current_scene {
    case .Menu:
        scenes.init_menu()
        break

    case .Game:
        scenes.init_game()
        break
    }
}

// NOTE: I don't know
update_current_scene :: #force_inline proc() {
    @(static)
    next_scene: scenes.Game_Scene

    if !ctx.transition_active {
        #partial switch ctx.current_scene {
        case .Menu:
            next_scene = scenes.update_menu()
            break

        case .Game:
            next_scene = scenes.update_game()
            break
        }

        if next_scene == .Util_Close_Window {
            ctx.window_should_close = true
        }

        if next_scene != ctx.current_scene {
            ctx.transition_active = true
        }
    } else {
        ctx.tick_counter += rl.GetFrameTime()

        if !ctx.transition_out {
            ctx.transition_scale = rl.EaseExpoIn(ctx.tick_counter, 0, 1, 0.25)

            if ctx.transition_scale >= 1 {
                ctx.transition_scale = 1
                ctx.transition_out = true
                ctx.tick_counter = 0

                unload_current_scene()
                ctx.current_scene = next_scene

                init_current_scene()
            }
        } else {
            offset := rl.EaseExpoOut(ctx.tick_counter, 0, 1, 0.25)
            ctx.transition_scale = 1 - offset

            if ctx.transition_scale <= 0 {
                ctx.transition_scale = 0
                ctx.transition_out = false
                ctx.transition_active = false
                ctx.tick_counter = 0
            }
        }
    }
}

draw_current_scene :: #force_inline proc() {
    rl.ClearBackground({33, 52, 69, 255})

    #partial switch ctx.current_scene {
    case .Menu:
        scenes.draw_menu()
        break

    case .Game:
        scenes.draw_game()
        break
    }

    if ctx.transition_active {
        mouse := rl.GetMousePosition()

        rl.DrawCircleV(mouse, (f32(rl.GetRenderWidth()) * 1.25) * ctx.transition_scale, rl.ColorBrightness({33, 52, 69, 255}, -0.25))
    }
}

unload_current_scene :: #force_inline proc() {
    #partial switch ctx.current_scene {
    case .Menu:
        scenes.unload_menu()
        break

    case .Game:
        scenes.unload_game()
        break
    }
}

update_transition :: proc() {

}

show_no_bundle_screen :: #force_inline proc() {
    rl.ClearBackground(rl.RAYWHITE)

    MESSAGE :: "No asset bundle found! :("
    FONT_SIZE :: 80
    FONT_SPACING :: 4

    text_size := rl.MeasureTextEx(rl.GetFontDefault(), MESSAGE, FONT_SIZE, FONT_SPACING)
    window_middle: rl.Vector2 = {f32(rl.GetRenderWidth()), f32(rl.GetRenderHeight())} / 2

    rl.DrawTextEx(rl.GetFontDefault(), MESSAGE, window_middle - (text_size / 2), FONT_SIZE, FONT_SPACING, rl.GRAY)
}

debug_draw_fps :: #force_inline proc() {
    DEBUG_FONT_SIZE :: 20

    @(static)
    debug_show_fps := true

    if rl.IsKeyPressed(.GRAVE) do debug_show_fps = !debug_show_fps

    if debug_show_fps {
        current_fps := rl.TextFormat("FPS: %d", rl.GetFPS())
        text_width := rl.MeasureText(current_fps, DEBUG_FONT_SIZE)
        text_colour := rl.GetFPS() < FPS_MINIMUM ? rl.ORANGE : rl.GREEN

        rl.DrawRectangle(0, 0, text_width + 16, 32, rl.Fade(rl.BLACK, 0.5))
        rl.DrawText(current_fps, 8, 8, DEBUG_FONT_SIZE, text_colour)
    }
}
