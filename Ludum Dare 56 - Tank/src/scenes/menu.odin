package scenes

import "core:os"

import "../assets"
import ls "shared:lspx"
import rl "vendor:raylib"

@(private = "file")
Menu_Context :: struct {
    save_file_exists: bool,
}

@(private = "file")
ctx: Menu_Context

init_menu :: proc() {
    global_tick_counter = 0
    next_scene = .Menu

    ctx.save_file_exists = os.is_file("save.json")
}

update_menu :: proc() -> Game_Scene {
    global_tick_counter += rl.GetFrameTime()

    return next_scene
}

draw_menu :: proc() {
    button_x := rl.EaseElasticOut(global_tick_counter, -256, 288, 0.75)

    if menu_button("New Tank", {button_x, 32, 256, 72}) {
        os.remove("save.json")
        next_scene = .Game
    }

    if menu_button("Load Tank", {button_x, 120, 256, 72}, !ctx.save_file_exists) {
        next_scene = .Game
    }

    if menu_button("Exit", {button_x, 616, 128, 72}) {
        next_scene = .Util_Close_Window
    }
}

unload_menu :: proc() {}

@(private = "file")
menu_button :: proc(label: string, bounds: rl.Rectangle, disabled: bool = false) -> bool {
    result: bool
    colour := disabled ? rl.GRAY : rl.WHITE

    if rl.CheckCollisionPointRec(rl.GetMousePosition(), bounds) && !disabled {
        ls.DrawSpriteNineSlice("menu_button_hover", bounds, colour)

        if rl.IsMouseButtonReleased(.LEFT) {
            result = true
            rl.PlaySound(assets.sounds["menu_button"])
        }
    } else {
        ls.DrawSpriteNineSlice("menu_button", bounds, colour)
    }

    rl.DrawTextEx(assets.fonts["menu_button"], assets.dictionary_lookup(label), {bounds.x + 12, bounds.y + 4}, 56, 4, colour)

    return result
}
