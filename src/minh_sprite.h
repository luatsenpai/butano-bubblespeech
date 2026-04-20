#pragma once

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_items_minh.h"
#include "bn_sprite_ptr.h"

namespace rpg {

enum class dir {
    up,
    down,
    left,
    right,
};

class minh_sprite
{
public:
    explicit minh_sprite(bn::fixed x, bn::fixed y, const bn::camera_ptr& cam);

    bn::sprite_ptr& sprite();
    const bn::sprite_ptr& sprite() const;

    void set_dir(dir d);
    dir get_dir() const;

    void start_walk();
    void stop_walk();
    void update_anim();
    void set_idle_frame();

private:
    int _idle_index_for(dir d) const;
    void _make_walk_anim_for(dir d);

    bn::sprite_ptr _spr;
    dir _dir = dir::down;
    bn::optional<bn::sprite_animate_action<3>> _walk_anim;
};

} // namespace rpg