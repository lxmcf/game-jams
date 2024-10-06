package assets

import ls "shared:lspx"
import rl "vendor:raylib"

import "core:fmt"
import "core:path/filepath"
import "core:strings"

fonts: map[string]rl.Font
sounds: map[string]rl.Sound
dictionary: map[string]cstring

sprites: ls.Bundle
sprites_loaded: bool

load_assets :: proc() {
    sprites, sprites_loaded = ls.LoadBundle("sprite_project/export/bundle.lspx" when ODIN_DEBUG else "bundle.lspx")
    if sprites_loaded {
        ls.SetActiveBundle(sprites)
    }

    // Fonts
    fonts["menu_button"] = rl.LoadFontEx("font/concert_one.ttf", 56, nil, 255)

    sounds = load_sounds("sound/*.ogg")
}

unload_assets :: proc() {
    ls.UnloadBundle(sprites)

    for _, value in dictionary do delete(value)
    delete(dictionary)

    for _, value in fonts do rl.UnloadFont(value)
    delete(fonts)

    for key, sound in sounds {
        rl.UnloadSound(sound)
        delete(key)
    }

    delete(sounds)
}

dictionary_lookup :: proc(key: string) -> cstring {
    exists := key in dictionary

    if !exists {
        dictionary[key] = strings.clone_to_cstring(key)
    }

    return dictionary[key]
}

@(private)
load_sounds :: proc(pattern: string) -> map[string]rl.Sound {
    local_sounds := make(map[string]rl.Sound)

    matches, err := filepath.glob(pattern)
    if err == .Syntax_Error {
        rl.TraceLog(.ERROR, "[ASSET] Invalid syntax")
        return sounds
    }

    for match in matches {
        index := strings.last_index_any(match, filepath.SEPARATOR_STRING)
        path := strings.clone(filepath.stem(match[index + 1:]))
        local_sounds[path] = rl.LoadSound(fmt.ctprintf("%s", match))

        delete(match)
    }

    delete(matches)

    return local_sounds
}
