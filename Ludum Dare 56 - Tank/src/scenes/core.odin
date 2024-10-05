package scenes

Game_Scene :: enum {
    Util_Close_Window,
    Menu,
    Game,
}

@(private)
global_tick_counter: f32

next_scene: Game_Scene
