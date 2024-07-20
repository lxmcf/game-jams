#define RAYSUITE_NO_GUI
#include <raysuite.h>

#include "../assets/sprite.h"
#include "../assets/font.h"
#include "../scenes.h"

// Scuffed :-)
#include "dungeon_layouts.h"

typedef enum CardType {
    FIREBALL,
    BLIZZARD,
    BOLT,
    HEAL,
    FREEZE,

    CARD_TYPE_COUNT,
    CARD_TYPE_NONE
} CardType;

#define FIREBALL_VALUE 3
#define FIREBALL_ICON "assets/card/fireball.png"

#define BLIZZARD_VALUE 2
#define BLIZZARD_ICON "assets/card/blizzard.png"

#define BOLT_VALUE 3
#define BOLT_ICON "assets/card/bolt.png"

#define FREEZE_VALUE 2
#define FREEZE_ICON "assets/card/freeze.png"

#define HEAL_VALUE 1
#define HEAL_ICON "assets/card/mini_heart.png"

#ifdef __cplusplus
extern "C" {
#endif

const char* CardGetName (CardType type);
void DrawCard (CardType type, Vector2 position);
void PlayCard (CardType type);

#ifdef __cplusplus
}
#endif


#define PLAYER_DAMAGE_GRACE_TIMER 1.0f
#define PLAYER_HAND_SIZE 4
#define PLAYER_SPEED 100.0f
typedef struct Player {
    Vector2 position;

    int current_card;
    int current_hand_size;
    CardType current_hand[PLAYER_HAND_SIZE];

    int life;
    int max_life;

    int closest_enemy_id;

    float speed_multiplier;
    float damage_timer;
} Player;

#define MAX_ENEMIES 8
typedef enum EnemyType {
    SLIME,
    GHOST_1,
    GHOST_2,
    SKELETON,
    IMP,
    GOBLIN,

    ENEMY_TYPE_COUNT
} EnemyType;

typedef struct Enemy {
    Vector2 position;
    EnemyType type;

    bool is_active;
    bool is_defeated;
    bool is_ghost;

    int life;

    float speed;
} Enemy;

#ifdef __cplusplus
extern "C" {
#endif

Enemy CreateEnemySlime (Vector2 position);
Enemy CreateEnemyGhost1 (Vector2 position);
Enemy CreateEnemyGhost2 (Vector2 position);
Enemy CreateEnemySkeleton (Vector2 position);
Enemy CreateEnemyImp (Vector2 position);
Enemy CreateEnemyGoblin (Vector2 position);

void DrawEnemy (Enemy enemy);

#ifdef __cplusplus
}
#endif


#define MAX_HAZARDS 24
typedef enum HazardType {
    WEB,
    FIRE,

    HAZARD_COUNT
} HazardType;

typedef struct Hazard {
    Vector2 position;
    HazardType type;

    bool is_active;
} Hazard;

#ifdef __cplusplus
extern "C" {
#endif

Hazard CreateHazardWeb (Vector2 position);
void DrawHazard (Hazard hazard);

#ifdef __cplusplus
}
#endif


static Enemy __dungeon_enemies[MAX_ENEMIES];
static Hazard __dungeon_hazards[MAX_HAZARDS];

static GameSceneId next_scene;

#define DUNGEON_TILE_SIZE 16
#define DUNGEON_TILES_WIDTH 16
#define DUNGEON_TILES_HEIGHT 8

static Rectangle __dungeon_bounds;
static Player player;

#define HAND_DEAL_TIMER 1.5f
struct DungeonState {
    bool room_complete;

    int enemy_count;
    int hazard_count;

    int rooms_completed;

    float dealer_timer;
} dungeon_state;

static void __ResetPlayerPosition (void) {
    player.position = CLITERAL(Vector2){ __dungeon_bounds.width / 2, __dungeon_bounds.height - (DUNGEON_TILE_SIZE / 4) };
}

static void __ResetDungeon (void) {
    for (size_t i = 0; i < MAX_ENEMIES; i++) __dungeon_enemies[i].is_active = false;
    for (size_t i = 0; i < MAX_HAZARDS; i++) __dungeon_hazards[i].is_active = false;

    dungeon_state.enemy_count = 0;
    dungeon_state.hazard_count = 0;
    dungeon_state.room_complete = false;

    __ResetPlayerPosition ();
}

static Enemy __SpawnEnemyFromDifficulty (Vector2 position, int difficulty) {
    int enemy_type = GetRandomValue (0, difficulty);

    switch (enemy_type) {
        case SLIME:     return CreateEnemySlime (position);
        case GHOST_1:   return CreateEnemyGhost1 (position);
        case GHOST_2:   return CreateEnemyGhost2 (position);
        case SKELETON:  return CreateEnemySkeleton (position);
        case IMP:       return CreateEnemyImp (position);
        case GOBLIN:    return CreateEnemyGoblin (position);

        // Defaul to gobbo
        default:
            return CreateEnemyGoblin (position);
    }
}

static void __LoadRandomDungeonLayout (void) {
    __ResetDungeon ();

    int dungeon_id = GetRandomValue (0, lengthof (__dungeon_layouts) - 1);

    const char* dungeon_string = __dungeon_layouts[dungeon_id];

    int tile_x = 0;
    int tile_y = 0;

    int i = 0;
    char character;

    #define FLOOR_MOD 5
    int difficulty = MIN (dungeon_state.rooms_completed / FLOOR_MOD, ENEMY_TYPE_COUNT - 1);

    while ((character = dungeon_string[i]) != '\0') {
        Vector2 position = CLITERAL(Vector2) {
            .x = ((tile_x + 1) * DUNGEON_TILE_SIZE) - (DUNGEON_TILE_SIZE / 2),
            .y = (tile_y * DUNGEON_TILE_SIZE) + (DUNGEON_TILE_SIZE / 2)
        };

        switch (character) {
            case '\n':
                tile_x = 0;
                tile_y++;
                i++;

                continue;

            case 'E':
                __dungeon_enemies[dungeon_state.enemy_count++] = __SpawnEnemyFromDifficulty (position, difficulty);
                break;

            case 'W':
                __dungeon_hazards[dungeon_state.hazard_count++] = CreateHazardWeb (position);
                break;
        }

        tile_x++;
        i++;
    }
}

void InitDungeon (void) {
    next_scene = SCENE_NULL;
    __dungeon_bounds = CLITERAL(Rectangle){ 0, 0, DUNGEON_TILES_WIDTH * DUNGEON_TILE_SIZE, DUNGEON_TILES_HEIGHT * DUNGEON_TILE_SIZE };

    Vector2 scene_view = SceneGetViewSize ();
    SceneSetViewOffset (CLITERAL(Vector2){ scene_view.x / 2, scene_view.y / 2 });
    SceneSetViewTarget (CLITERAL(Vector2){ __dungeon_bounds.width / 2, __dungeon_bounds.height / 2 });

    // Ensure texture's have correct data that need it, super scuffed
    // PLAYER
    CacheSprite ("assets/player.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/target.png", CLITERAL(Vector2){ 8, 16 }, WHITE);

    // CARD ICONS
    CacheSprite ("assets/card/mini_heart.png", CLITERAL(Vector2){ 8, 8}, WHITE);
    CacheSprite (FIREBALL_ICON, CLITERAL(Vector2){ 8, 8}, WHITE);
    CacheSprite (BLIZZARD_ICON, CLITERAL(Vector2){ 8, 8}, WHITE);
    CacheSprite (BOLT_ICON, CLITERAL(Vector2){ 8, 8}, WHITE);
    CacheSprite (HEAL_ICON, CLITERAL(Vector2){ 8, 8}, WHITE);
    CacheSprite (FREEZE_ICON, CLITERAL(Vector2){ 8, 8}, WHITE);

    // MISC
    CacheSprite ("assets/trap.png", CLITERAL(Vector2){ 8, 8 }, WHITE);
    CacheSprite ("assets/bones.png", CLITERAL(Vector2){ 8, 8 }, WHITE);

    // ENEMIES
    CacheSprite ("assets/enemies/slime.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/enemies/ghost_1.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/enemies/ghost_2.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/enemies/skeleton.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/enemies/imp.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/enemies/goblin.png", CLITERAL(Vector2){ 8, 16 }, WHITE);

    // DUNGEON ASSETS
    CacheSprite ("assets/dungeon/door_closed.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/dungeon/door_open.png", CLITERAL(Vector2){ 8, 16 }, WHITE);
    CacheSprite ("assets/dungeon/pointer_arrow.png", CLITERAL(Vector2){ 8, 0 }, WHITE);

    player = CLITERAL(Player) {
        .position = Vector2Zero (),
        .life = 3,
        .max_life = 6,

        .closest_enemy_id = 0,
        .speed_multiplier = 1.0f,

        .current_hand_size = PLAYER_HAND_SIZE
    };

    for (size_t i = 0; i < PLAYER_HAND_SIZE; i++) {
        player.current_hand[i] = GetRandomValue (0, CARD_TYPE_COUNT - 1);
    }

    __ResetDungeon ();

    dungeon_state.dealer_timer = HAND_DEAL_TIMER;
}

void UpdateDungeon (void) {
    #ifdef BUILD_DEBUG
    {   // Debug Stuff
        if (IsKeyReleased (KEY_SPACE)) dungeon_state.room_complete = !dungeon_state.room_complete;

        // Refresh dungeon on demand
        if (IsKeyReleased (KEY_F1)) {
            __LoadRandomDungeonLayout ();
        }
    }
    #endif

    {   // Update Dungeon
        if (dungeon_state.enemy_count == 0) {
            if (!dungeon_state.room_complete) dungeon_state.room_complete = true;

            #define INTERACT_DISTANCE 20
            if (Vector2Distance (player.position, CLITERAL(Vector2){ __dungeon_bounds.width / 2, 0.0f }) <= INTERACT_DISTANCE) {
                dungeon_state.rooms_completed++;

                __LoadRandomDungeonLayout ();
            }
        }

        if (dungeon_state.dealer_timer > EPSILON) {
            dungeon_state.dealer_timer -= GetFrameTime ();
        } else {
            dungeon_state.dealer_timer = HAND_DEAL_TIMER;

            for (size_t i = 0; i < PLAYER_HAND_SIZE; i++) {
                if (player.current_hand[i] == CARD_TYPE_NONE) {
                    player.current_hand[i] = GetRandomValue (0, CARD_TYPE_COUNT - 1);

                    break;
                }
            }
        }
    }

    {   // Update Enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!__dungeon_enemies[i].is_active) continue;
            if (__dungeon_enemies[i].is_defeated) continue;

            if (__dungeon_enemies[i].life <= 0) {
                __dungeon_enemies[i].is_defeated = true;
                dungeon_state.enemy_count--;

                continue;
            }

            #define ATTACK_DISTANCE 16.0f
            if (Vector2Distance (__dungeon_enemies[i].position, player.position) > ATTACK_DISTANCE) {
                __dungeon_enemies[i].position = Vector2MoveTowards (__dungeon_enemies[i].position, player.position, __dungeon_enemies[i].speed * GetFrameTime ());
            } else {
                if (player.damage_timer <= EPSILON) {
                    player.life--;

                    player.damage_timer = PLAYER_DAMAGE_GRACE_TIMER;
                }
            }
        }
    }

    {   // Update Player Variables
        player.speed_multiplier = 1.0f;

        for (size_t i = 0; i < MAX_HAZARDS; i++) {
            if (!__dungeon_hazards[i].is_active) continue;

            #define COLLISION_DISTANCE 8
            if (Vector2Distance (player.position, __dungeon_hazards[i].position) < COLLISION_DISTANCE) player.speed_multiplier = 0.35f;
        }

        if (dungeon_state.enemy_count > 0) {
            float distance = 1000.0f;

            for (size_t i = 0; i < MAX_ENEMIES; i++) {
                if (!__dungeon_enemies[i].is_active) continue;
                if (__dungeon_enemies[i].is_defeated) continue;

                float temp_distance = Vector2Distance (player.position, __dungeon_enemies[i].position);

                if (temp_distance < distance) {
                    distance = temp_distance;

                    player.closest_enemy_id = i;
                }
            }
        }

        if (player.damage_timer > EPSILON) {
            player.damage_timer -= GetFrameTime ();

            SpriteSetColour ("assets/player.png", GRAY);
        } else {
            SpriteSetColour ("assets/player.png", WHITE);
        }
    }

    {   // Update Player Position
        int xaxis = IsKeyDown (KEY_D) - IsKeyDown (KEY_A);
        int yaxis = IsKeyDown (KEY_S) - IsKeyDown (KEY_W);

        Vector2 velocity = Vector2Normalize (CLITERAL(Vector2){ xaxis, yaxis });
        velocity = CLITERAL(Vector2){ velocity.x * ((PLAYER_SPEED * player.speed_multiplier) * GetFrameTime ()), velocity.y * ((PLAYER_SPEED * player.speed_multiplier) * GetFrameTime ()) };

        player.position = Vector2Add (player.position, velocity);

        // Confine Player
        #define PADDING_X 8
        #define PADDING_Y 16
        if (player.position.x < PADDING_X) player.position.x = PADDING_X;
        if (player.position.y < PADDING_Y) player.position.y = PADDING_Y;

        if (player.position.x > __dungeon_bounds.width - PADDING_X) player.position.x = __dungeon_bounds.width - PADDING_X;
        if (player.position.y > __dungeon_bounds.height) player.position.y = __dungeon_bounds.height;
    }

    {   // Card Selection
        if (player.current_hand_size > 0) {
            int xaxis = IsKeyPressed (KEY_RIGHT) - IsKeyPressed (KEY_LEFT);
            int activate = IsKeyPressed (KEY_UP);

            player.current_card += xaxis;

            if (player.current_card < 0) player.current_card = PLAYER_HAND_SIZE - 1;
            if (player.current_card > PLAYER_HAND_SIZE - 1) player.current_card = 0;

            if (dungeon_state.enemy_count > 0) {
                if (activate && player.current_hand[player.current_card] != CARD_TYPE_NONE) {
                    PlayCard (player.current_hand[player.current_card]);

                    player.current_hand[player.current_card] = CARD_TYPE_NONE;
                }
            }
        }
    }
}

void DrawDungeon (void) {
    DrawSpriteNineSlice ("assets/dungeon/bounds.png", __dungeon_bounds);

    for (int i = 0; i < MAX_HAZARDS; i++) {
        if (!__dungeon_hazards[i].is_active) continue;

        DrawHazard (__dungeon_hazards[i]);
    }

    Vector2 door_position = CLITERAL(Vector2){ __dungeon_bounds.width / 2, 0 };

    if (dungeon_state.room_complete) {
        DrawSprite ("assets/dungeon/door_open.png", door_position);

        // Exit Arrow
        DrawSprite ("assets/dungeon/pointer_arrow.png", CLITERAL(Vector2){ __dungeon_bounds.width / 2, SINE (GetTime () * 8) * 4 });
    } else {
        DrawSprite ("assets/dungeon/door_closed.png", door_position);
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!__dungeon_enemies[i].is_active) continue;

        if (__dungeon_enemies[i].is_defeated) {
            DrawSprite ("assets/bones.png", __dungeon_enemies[i].position);
            continue;
        }

        DrawEnemy (__dungeon_enemies[i]);
    }

    if (dungeon_state.enemy_count > 0) {
        DrawSprite ("assets/target.png", __dungeon_enemies[player.closest_enemy_id].position);
    }

    DrawSprite ("assets/player.png", player.position);
}

void DrawDungeonGui (void) {
    Vector2 scene_view = SceneGetViewSize ();

    #define UI_PADDING 2

    for (int i = 0; i < player.max_life; i++) {
        Vector2 position = CLITERAL(Vector2) {
            .x = UI_PADDING + i * (16 + UI_PADDING),
            .y = UI_PADDING
        };

        if (i > player.life - 1) {
            DrawSprite ("assets/heart_empty.png", position);
            continue;
        }

        DrawSprite ("assets/heart.png", position);
    }

    Rectangle card_bounds = SpriteGetBounds ("assets/card/card.png");
    for (int i = 0; i < PLAYER_HAND_SIZE; i++) {
        int offset = 16;

        if (player.current_hand[i] == CARD_TYPE_NONE) {
            offset = 8;

            if (player.current_card == i) offset = 12;
        } else if (player.current_card == i) offset = 32;


        Vector2 position = CLITERAL(Vector2) {
            .x = 8 + i * (card_bounds.width + (UI_PADDING * 2)),
            .y = scene_view.y - offset,
        };

        DrawCard (player.current_hand[i], position);
    }
}

void UnloadDungeon (void) { }

GameSceneId FinishDungeon (void) {
    return next_scene;
}

const char* CardGetName (CardType type) {
    switch (type) {
        case FIREBALL:  return "FIREBALL";
        case BLIZZARD:  return "BLIZZARD";
        case BOLT:      return "BOLT";
        case HEAL:      return "HEAL";
        case FREEZE:    return "FREEZE";

        default:
            return "...";
    }
}

void DrawCard (CardType type, Vector2 position) {
    DrawSprite ("assets/card/card.png", position);

    Rectangle bounds = SpriteGetBounds ("assets/card/card.png");

    // Yes I am doing this twice
    Vector2 card_offset = CLITERAL(Vector2) { bounds.width / 2, 8 };

    FontDrawTextCentered ("assets/font/four_pixels_font.ttf", CardGetName (type), Vector2Add (position, card_offset), WHITE, 4);

    card_offset.y = 22;

    switch (type) {
        case FIREBALL:
            card_offset.x = (bounds.width / 2) - 8;
            DrawSprite (FIREBALL_ICON, Vector2Add (position, card_offset));

            card_offset.x = (bounds.width / 2) + 8;
            FontDrawTextCentered ("assets/font/four_pixels_font.ttf", TextFormat ("%d", FIREBALL_VALUE), Vector2Add (position, card_offset), WHITE, 8);
            break;

        case BLIZZARD:
            card_offset.x = (bounds.width / 2) - 8;
            DrawSprite (BLIZZARD_ICON, Vector2Add (position, card_offset));

            card_offset.x = (bounds.width / 2) + 8;
            FontDrawTextCentered ("assets/font/four_pixels_font.ttf", TextFormat ("%d", BLIZZARD_VALUE), Vector2Add (position, card_offset), WHITE, 8);
            break;

        case BOLT:
            card_offset.x = (bounds.width / 2) - 8;
            DrawSprite (BOLT_ICON, Vector2Add (position, card_offset));

            card_offset.x = (bounds.width / 2) + 8;
            FontDrawTextCentered ("assets/font/four_pixels_font.ttf", TextFormat ("%d", BOLT_VALUE), Vector2Add (position, card_offset), WHITE, 8);
            break;

        case HEAL:
            DrawSprite (HEAL_ICON, Vector2Add (position, card_offset));
            break;

        case FREEZE:
            DrawSprite (FREEZE_ICON, Vector2Add (position, card_offset));
            break;

        default: break;
    }
}

void PlayCard (CardType type) {
    switch (type) {
        case FIREBALL:
            __dungeon_enemies[player.closest_enemy_id].life -= FIREBALL_VALUE;
            break;

        case BLIZZARD:
            __dungeon_enemies[player.closest_enemy_id].life -= BLIZZARD_VALUE;
            break;

        case BOLT:
            __dungeon_enemies[player.closest_enemy_id].life -= BOLT_VALUE;
            break;

        case HEAL:
            if (player.life < player.max_life) player.life += HEAL_VALUE;
            break;

        case FREEZE:
            // __dungeon_enemies[player.closest_enemy_id].life -= BLIZZARD_VALUE;
            break;

        default: break;
    }
}

Enemy CreateEnemySlime (Vector2 position) {
    return CLITERAL(Enemy) {
        .position = position,
        .type = SLIME,

        .is_active = true,
        .is_defeated = false,
        .is_ghost = false,

        .life = 1,
        .speed = PLAYER_SPEED * 0.25f
    };
}

Enemy CreateEnemyGhost1 (Vector2 position) {
    return CLITERAL(Enemy) {
        .position = position,
        .type = GHOST_1,

        .is_active = true,
        .is_defeated = false,
        .is_ghost = true,

        .life = GetRandomValue (1, 2),
        .speed = PLAYER_SPEED * 0.3f
    };
}

Enemy CreateEnemyGhost2 (Vector2 position) {
    return CLITERAL(Enemy) {
        .position = position,
        .type = GHOST_2,

        .is_active = true,
        .is_defeated = false,
        .is_ghost = true,

        .life = GetRandomValue (1, 3),
        .speed = PLAYER_SPEED * 0.25f
    };
}

Enemy CreateEnemySkeleton (Vector2 position) {
    return CLITERAL(Enemy) {
        .position = position,
        .type = SKELETON,

        .is_active = true,
        .is_defeated = false,
        .is_ghost = false,

        .life = 3,
        .speed = PLAYER_SPEED * 0.5f
    };
}

Enemy CreateEnemyImp (Vector2 position) {
    return CLITERAL(Enemy) {
        .position = position,
        .type = IMP,

        .is_active = true,
        .is_defeated = false,
        .is_ghost = false,

        .life = 5,
        .speed = PLAYER_SPEED * 0.7f
    };
}

Enemy CreateEnemyGoblin (Vector2 position) {
    return CLITERAL(Enemy) {
        .position = position,
        .type = GOBLIN,

        .is_active = true,
        .is_defeated = false,
        .is_ghost = false,

        .life = 4,
        .speed = PLAYER_SPEED * 0.5f
    };
}

void DrawEnemy (Enemy enemy) {
    float vertical_float = enemy.is_ghost ? SINE(GetTime () * 8) * 3 : 0;

    Vector2 position = CLITERAL(Vector2){ enemy.position.x, enemy.position.y - vertical_float };

    switch (enemy.type) {
        case SLIME:
            DrawSprite ("assets/enemies/slime.png", position);
            break;

        case GHOST_1:
            DrawSprite ("assets/enemies/ghost_1.png", position);
            break;

        case GHOST_2:
            DrawSprite ("assets/enemies/ghost_2.png", position);
            break;

        case SKELETON:
            DrawSprite ("assets/enemies/skeleton.png", position);
            break;

        case IMP:
            DrawSprite ("assets/enemies/imp.png", position);
            break;

        case GOBLIN:
            DrawSprite ("assets/enemies/goblin.png", position);
            break;

        default:
            DrawCircleV (position, 4, PURPLE);
    }
}

Hazard CreateHazardWeb (Vector2 position) {
    return CLITERAL(Hazard){
        .position = position,
        .type = WEB,

        .is_active = true
    };
}

void DrawHazard (Hazard hazard) {
    switch (hazard.type) {
        case WEB:
            DrawSprite ("assets/trap.png", hazard.position);
            break;

        default:
            DrawCircleV (hazard.position, 4, GREEN);
    }
}
