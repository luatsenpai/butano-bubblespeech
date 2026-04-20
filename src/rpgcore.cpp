#include "rpgcore.h"

#include "bn_core.h"
#include "bn_fixed.h"
#include "bn_keypad.h"
#include "bn_optional.h"
#include "dialogsystem.h"
#include "map.h"
#include "minh_sprite.h"

namespace rpg {

namespace {

constexpr bn::fixed player_speed = 1;

void move_player(world_map& world, minh_sprite& minh, bool& walking)
{
    bn::fixed dx = 0;
    bn::fixed dy = 0;
    dir next_dir = minh.get_dir();

    if(bn::keypad::left_held())
    {
        dx = -player_speed;
        next_dir = dir::left;
    }
    else if(bn::keypad::right_held())
    {
        dx = player_speed;
        next_dir = dir::right;
    }
    else if(bn::keypad::up_held())
    {
        dy = -player_speed;
        next_dir = dir::up;
    }
    else if(bn::keypad::down_held())
    {
        dy = player_speed;
        next_dir = dir::down;
    }

    const bool moving = dx != 0 || dy != 0;

    if(next_dir != minh.get_dir())
    {
        minh.set_dir(next_dir);

        if(walking)
        {
            minh.start_walk();
        }
    }

    if(moving)
    {
        if(! walking)
        {
            minh.start_walk();
            walking = true;
        }

        minh.sprite().set_x(clamp_to_map_x(minh.sprite().x() + dx));
        minh.sprite().set_y(clamp_to_map_y(minh.sprite().y() + dy));
        minh.update_anim();
    }
    else if(walking)
    {
        minh.stop_walk();
        walking = false;
    }

    sync_camera_to_position(world.cam(), minh.sprite().x(), minh.sprite().y());
}

void try_change_map(world_map& world, minh_sprite& minh, bool& walking)
{
    if(! bn::keypad::a_pressed())
    {
        return;
    }

    const int tx = minh_to_tile_x(minh.sprite().x());
    const int ty = minh_to_tile_y(minh.sprite().y());

    if(! is_transition_tile(world.current(), tx, ty))
    {
        return;
    }

    if(walking)
    {
        minh.stop_walk();
        walking = false;
    }

    if(world.current() == map_id::houseoutside)
    {
        world.switch_to_houseinside();
        minh.sprite().set_position(minh_tile_center_x(core_layout::houseinside_spawn_tx),
                                   minh_tile_center_y(core_layout::houseinside_spawn_ty));
        minh.set_dir(dir::down);
    }
    else
    {
        world.switch_to_houseoutside();
        minh.sprite().set_position(minh_tile_center_x(core_layout::exit_to_houseoutside_spawn_tx),
                                   minh_tile_center_y(core_layout::exit_to_houseoutside_spawn_ty));
        minh.set_dir(dir::down);
    }

    minh.set_idle_frame();
    sync_camera_to_position(world.cam(), minh.sprite().x(), minh.sprite().y());
}

} // namespace

void run()
{
    bn::core::init();

    static bn::optional<world_map> world_storage;
    static bn::optional<minh_sprite> minh_storage;
    static bn::optional<dialog_system> dialog_storage;

    world_storage.emplace();
    world_map& world = *world_storage;

    minh_storage.emplace(minh_tile_center_x(core_layout::houseoutside_spawn_tx),
                         minh_tile_center_y(core_layout::houseoutside_spawn_ty),
                         world.cam());
    minh_sprite& minh = *minh_storage;

    sync_camera_to_position(world.cam(), minh.sprite().x(), minh.sprite().y());

    dialog_storage.emplace(world.cam());
    dialog_system& dialog = *dialog_storage;
    dialog.register_actor("minh", minh.sprite(), 8);

    bool walking = false;

    while(true)
    {
        if(! dialog.active())
        {
            move_player(world, minh, walking);
            try_change_map(world, minh, walking);

            if(bn::keypad::b_pressed())
            {
                dialog.queue_script_line("[Minh]Đây là hộp thoại nói bình thường để test speech box sau khi chỉnh vị trí và 9-slice., speech, minh");
            }
            else if(bn::keypad::l_pressed())
            {
                dialog.queue_script_line("[Minh]Mình đang suy nghĩ xem nên đi dạo tiếp hay quay vào nhà nghỉ một chút đây..., think, minh");
            }
            else if(bn::keypad::r_pressed())
            {
                dialog.queue_script_line("[Minh]CẨN THẬN! ĐỪNG CHẠY RA NGOÀI QUÁ XA!, shout, minh");
            }
            else if(bn::keypad::start_pressed())
            {
                dialog.queue_script_line("[Minh]Đây là hộp thoại nói bình thường để test speech box., speech, minh");
                dialog.queue_script_line("[Minh]Còn đây là hộp thoại suy nghĩ để test think box và think tail., think, minh");
                dialog.queue_script_line("[Minh]VÀ ĐÂY LÀ HỘP THOẠI HÉT ĐỂ TEST SHOUT BOX!, shout, minh");
            }
        }
        else if(walking)
        {
            minh.stop_walk();
            walking = false;
        }

        dialog.update();
        bn::core::update();
    }
}

} // namespace rpg
