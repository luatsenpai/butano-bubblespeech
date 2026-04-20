#pragma once

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"

namespace rpg {

enum class map_id : int {
    houseoutside,
    houseinside,
};

constexpr int TILE = 16;
constexpr int MAP_WIDTH_PX = 256;
constexpr int MAP_HEIGHT_PX = 256;
constexpr int MAP_HALF_WIDTH = MAP_WIDTH_PX / 2;
constexpr int MAP_HALF_HEIGHT = MAP_HEIGHT_PX / 2;
constexpr int SCREEN_W_HALF = 120;
constexpr int SCREEN_H_HALF = 80;

// Minh dùng sprite 16x32 nên giữ visual offset để đứng đúng nền map.
constexpr int MINH_VISUAL_BASE_Y_OFFSET = 7;
constexpr int HOUSEINSIDE_MINH_VISUAL_OFFSET_X = 0;
constexpr int HOUSEINSIDE_MINH_VISUAL_OFFSET_Y = -2;

// Các điểm spawn / cửa được gom lại để bạn đổi nhanh khi thay map art.
namespace core_layout {
    constexpr int houseoutside_spawn_tx = 6;
    constexpr int houseoutside_spawn_ty = 12;

    constexpr int houseinside_spawn_tx = 8;
    constexpr int houseinside_spawn_ty = 12;

    constexpr int exit_to_houseoutside_spawn_tx = 6;
    constexpr int exit_to_houseoutside_spawn_ty = 9;

    constexpr int houseoutside_door_tx = 6;
    constexpr int houseoutside_door_ty = 7;

    constexpr int houseinside_door_tx = 8;
    constexpr int houseinside_door_ty = 12;
}

extern map_id g_current_map_for_offsets;

int map_width_px(map_id mid);
int map_height_px(map_id mid);
int map_tiles_x(map_id mid);
int map_tiles_y(map_id mid);

int current_minh_visual_offset_x();
int current_minh_visual_offset_y();

bn::fixed tile_center_x(int tx, map_id mid);
bn::fixed tile_center_y(int ty, map_id mid);

bn::fixed minh_tile_center_x(int tx);
bn::fixed minh_tile_center_y(int ty);

int to_tile_x(bn::fixed x);
int to_tile_y(bn::fixed y);
int minh_to_tile_x(bn::fixed sprite_x);
int minh_to_tile_y(bn::fixed sprite_y);

bn::fixed clamp_to_map_x(bn::fixed v);
bn::fixed clamp_to_map_y(bn::fixed v);

void sync_camera_to_position(bn::camera_ptr& cam, bn::fixed x, bn::fixed y);

bool is_transition_tile(map_id current_map, int tx, int ty);

class world_map
{
public:
    world_map();

    void switch_to_houseoutside();
    void switch_to_houseinside();

    map_id current() const;
    bn::camera_ptr& cam();
    const bn::camera_ptr& cam() const;
    const bn::optional<bn::regular_bg_ptr>& bg() const;
    const bn::optional<bn::regular_bg_ptr>& overlay() const;

private:
    void _reload_current_map();

    map_id _current = map_id::houseoutside;
    bn::optional<bn::regular_bg_ptr> _bg;
    bn::optional<bn::regular_bg_ptr> _overlay;
    bn::camera_ptr _cam;
};

} // namespace rpg
