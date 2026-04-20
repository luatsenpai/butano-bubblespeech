#ifndef ASPR_ANIM_H
#define ASPR_ANIM_H

#include "bn_sprite_ptr.h"
#include "bn_sprite_items_aspr.h"
#include "bn_optional.h"
#include "bn_string.h"
#include "bn_camera_ptr.h"

namespace aspr_anim
{
    class AsprBlinker
    {
    public:
        AsprBlinker(const bn::string<16>& box_type, int y = 70, int priority = 2);
        void update();
        void hide();
        void set_position(int x, int y); // Set sprite position
        void set_camera(bn::camera_ptr& camera); // Set sprite camera

    private:
        bn::optional<bn::sprite_ptr> _sprite;
        int _counter = 0;
    };
}

#endif