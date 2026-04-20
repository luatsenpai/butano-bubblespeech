#include "minh_sprite.h"
#include "bn_sprites.h"

namespace rpg {

minh_sprite::minh_sprite(bn::fixed x, bn::fixed y, const bn::camera_ptr& cam) :
    _spr(bn::sprite_items::minh.create_sprite(x, y))
{
    _spr.set_camera(cam);
    _spr.set_bg_priority(2);
    _spr.set_z_order(bn::sprites::max_z_order());
    set_dir(dir::down);
    set_idle_frame();
}

bn::sprite_ptr& minh_sprite::sprite()
{
    return _spr;
}

const bn::sprite_ptr& minh_sprite::sprite() const
{
    return _spr;
}

void minh_sprite::set_dir(dir d)
{
    _dir = d;
    _spr.set_horizontal_flip(_dir == dir::right);
}

dir minh_sprite::get_dir() const
{
    return _dir;
}

int minh_sprite::_idle_index_for(dir d) const
{
    switch(d)
    {
        case dir::up:
            return 4;

        case dir::down:
            return 1;

        case dir::left:
        case dir::right:
            return 7;

        default:
            return 1;
    }
}

void minh_sprite::_make_walk_anim_for(dir d)
{
    _walk_anim.reset();

    const int wait = 5;
    auto tiles_item = bn::sprite_items::minh.tiles_item();

    switch(d)
    {
        case dir::up:
            _walk_anim = bn::create_sprite_animate_action_forever(_spr, wait, tiles_item, 3, 4, 5);
            break;

        case dir::down:
            _walk_anim = bn::create_sprite_animate_action_forever(_spr, wait, tiles_item, 0, 1, 2);
            break;

        case dir::left:
        case dir::right:
            _walk_anim = bn::create_sprite_animate_action_forever(_spr, wait, tiles_item, 6, 7, 8);
            break;

        default:
            _walk_anim = bn::create_sprite_animate_action_forever(_spr, wait, tiles_item, 0, 1, 2);
            break;
    }
}

void minh_sprite::start_walk()
{
    _make_walk_anim_for(_dir);
}

void minh_sprite::stop_walk()
{
    _walk_anim.reset();
    set_idle_frame();
}

void minh_sprite::update_anim()
{
    if(_walk_anim)
    {
        _walk_anim->update();
    }
}

void minh_sprite::set_idle_frame()
{
    _spr.set_tiles(bn::sprite_items::minh.tiles_item().create_tiles(_idle_index_for(_dir)));
}

} // namespace rpg