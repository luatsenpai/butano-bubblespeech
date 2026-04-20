#include "aspr_anim.h"

namespace aspr_anim
{
    AsprBlinker::AsprBlinker(const bn::string<16>& box_type, int y, int priority)
    {
        int x = box_type.ends_with("right") ? 64 : 100;
        _sprite = bn::sprite_items::aspr.create_sprite(x, y);
        _sprite->set_bg_priority(priority);
    }

    void AsprBlinker::update()
    {
        if(!_sprite.has_value())
            return;

        ++_counter;
        if(_counter % 30 == 0)
        {
            int new_tile_index = (_counter / 30) % 2;
            _sprite->set_tiles(bn::sprite_items::aspr.tiles_item(), new_tile_index);
        }
    }

    void AsprBlinker::hide()
    {
        if(_sprite.has_value())
        {
            _sprite->set_visible(false);
        }
    }

    void AsprBlinker::set_position(int x, int y)
    {
        if(_sprite.has_value())
        {
            _sprite->set_position(x, y);
        }
    }

    void AsprBlinker::set_camera(bn::camera_ptr& camera)
    {
        if(_sprite.has_value())
        {
            _sprite->set_camera(camera);
        }
    }
}