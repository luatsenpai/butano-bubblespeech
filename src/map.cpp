#include "map.h"

#include "bn_math.h"
#include "bn_regular_bg_items_rshis.h"
#include "bn_regular_bg_items_rshis_overlay.h"
#include "bn_regular_bg_items_rshos.h"
#include "bn_regular_bg_items_rshos_overlay.h"

namespace rpg {

map_id g_current_map_for_offsets = map_id::houseoutside;

int map_width_px(map_id) {
    return MAP_WIDTH_PX;
}

int map_height_px(map_id) {
    return MAP_HEIGHT_PX;
}

int map_tiles_x(map_id mid) {
    return map_width_px(mid) / TILE;
}

int map_tiles_y(map_id mid) {
    return map_height_px(mid) / TILE;
}

int current_minh_visual_offset_x()
{
    return g_current_map_for_offsets == map_id::houseinside ? HOUSEINSIDE_MINH_VISUAL_OFFSET_X : 0;
}

int current_minh_visual_offset_y()
{
    return MINH_VISUAL_BASE_Y_OFFSET +
           (g_current_map_for_offsets == map_id::houseinside ? HOUSEINSIDE_MINH_VISUAL_OFFSET_Y : 0);
}

bn::fixed tile_center_x(int tx, map_id mid)
{
    return -bn::fixed(map_width_px(mid) / 2) + bn::fixed(tx * TILE + TILE / 2);
}

bn::fixed tile_center_y(int ty, map_id mid)
{
    return -bn::fixed(map_height_px(mid) / 2) + bn::fixed(ty * TILE + TILE / 2);
}

bn::fixed minh_tile_center_x(int tx)
{
    return tile_center_x(tx, g_current_map_for_offsets) + bn::fixed(current_minh_visual_offset_x());
}

bn::fixed minh_tile_center_y(int ty)
{
    return tile_center_y(ty, g_current_map_for_offsets) + bn::fixed(current_minh_visual_offset_y());
}

int to_tile_x(bn::fixed x)
{
    return bn::max(0, bn::min(map_tiles_x(g_current_map_for_offsets) - 1,
                              int((x + bn::fixed(map_width_px(g_current_map_for_offsets) / 2)) / TILE)));
}

int to_tile_y(bn::fixed y)
{
    return bn::max(0, bn::min(map_tiles_y(g_current_map_for_offsets) - 1,
                              int((y + bn::fixed(map_height_px(g_current_map_for_offsets) / 2)) / TILE)));
}

int minh_to_tile_x(bn::fixed sprite_x)
{
    return to_tile_x(sprite_x - bn::fixed(current_minh_visual_offset_x()));
}

int minh_to_tile_y(bn::fixed sprite_y)
{
    return to_tile_y(sprite_y - bn::fixed(current_minh_visual_offset_y()));
}

bn::fixed clamp_to_map_x(bn::fixed v)
{
    const bn::fixed min_x = -bn::fixed(MAP_HALF_WIDTH - 1);
    const bn::fixed max_x = bn::fixed(MAP_HALF_WIDTH - 1);

    if(v < min_x)
    {
        return min_x;
    }

    if(v > max_x)
    {
        return max_x;
    }

    return v;
}

bn::fixed clamp_to_map_y(bn::fixed v)
{
    const bn::fixed min_y = -bn::fixed(MAP_HALF_HEIGHT - 1);
    const bn::fixed max_y = bn::fixed(MAP_HALF_HEIGHT - 1);

    if(v < min_y)
    {
        return min_y;
    }

    if(v > max_y)
    {
        return max_y;
    }

    return v;
}

void sync_camera_to_position(bn::camera_ptr& cam, bn::fixed x, bn::fixed y)
{
    bn::fixed cx = x - bn::fixed(current_minh_visual_offset_x());
    bn::fixed cy = y - bn::fixed(current_minh_visual_offset_y());

    const bn::fixed min_cx = -bn::fixed(MAP_HALF_WIDTH - SCREEN_W_HALF);
    const bn::fixed max_cx = bn::fixed(MAP_HALF_WIDTH - SCREEN_W_HALF);
    const bn::fixed min_cy = -bn::fixed(MAP_HALF_HEIGHT - SCREEN_H_HALF);
    const bn::fixed max_cy = bn::fixed(MAP_HALF_HEIGHT - SCREEN_H_HALF);

    if(cx < min_cx)
    {
        cx = min_cx;
    }
    else if(cx > max_cx)
    {
        cx = max_cx;
    }

    if(cy < min_cy)
    {
        cy = min_cy;
    }
    else if(cy > max_cy)
    {
        cy = max_cy;
    }

    cam.set_position(cx, cy);
}

bool is_transition_tile(map_id current_map, int tx, int ty)
{
    if(current_map == map_id::houseoutside)
    {
        return tx == core_layout::houseoutside_door_tx && ty == core_layout::houseoutside_door_ty;
    }

    return tx == core_layout::houseinside_door_tx && ty == core_layout::houseinside_door_ty;
}

world_map::world_map() :
    _cam(bn::camera_ptr::create(0, 0))
{
    switch_to_houseoutside();
}

void world_map::switch_to_houseoutside()
{
    _current = map_id::houseoutside;
    _reload_current_map();
}

void world_map::switch_to_houseinside()
{
    _current = map_id::houseinside;
    _reload_current_map();
}

map_id world_map::current() const
{
    return _current;
}

bn::camera_ptr& world_map::cam()
{
    return _cam;
}

const bn::camera_ptr& world_map::cam() const
{
    return _cam;
}

const bn::optional<bn::regular_bg_ptr>& world_map::bg() const
{
    return _bg;
}

const bn::optional<bn::regular_bg_ptr>& world_map::overlay() const
{
    return _overlay;
}

void world_map::_reload_current_map()
{
    g_current_map_for_offsets = _current;
    _bg.reset();
    _overlay.reset();

    if(_current == map_id::houseoutside)
    {
        _bg = bn::regular_bg_items::rshos.create_bg(0, 0);
        _overlay = bn::regular_bg_items::rshos_overlay.create_bg(0, 0);
    }
    else
    {
        _bg = bn::regular_bg_items::rshis.create_bg(0, 0);
        _overlay = bn::regular_bg_items::rshis_overlay.create_bg(0, 0);
    }

    _bg->set_camera(_cam);
    _bg->set_priority(3);

    if(_overlay)
    {
        _overlay->set_camera(_cam);
        _overlay->set_priority(0);
    }
}

} // namespace rpg
