#include "dialogsystem.h"

#include "bn_fixed.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_math.h"
#include "bn_sprite_items_namebox.h"
#include "bn_sprite_items_shoutbox.h"
#include "bn_sprite_items_speechbox.h"
#include "bn_sprite_items_speechtail.h"
#include "bn_sprite_items_thinkbox.h"
#include "bn_sprite_items_thinktail.h"
#include "common_variable_8x16_sprite_font.h"

namespace rpg {
namespace {

bn::string_view trim_view(bn::string_view value)
{
    while(! value.empty() && (value.front() == ' ' || value.front() == '\t'))
    {
        value.remove_prefix(1);
    }

    while(! value.empty() && (value.back() == ' ' || value.back() == '\t'))
    {
        value.remove_suffix(1);
    }

    return value;
}

int last_index_of(bn::string_view value, char ch)
{
    for(int index = int(value.size()) - 1; index >= 0; --index)
    {
        if(value[unsigned(index)] == ch)
        {
            return index;
        }
    }

    return -1;
}

const bn::sprite_item& sprite_item_for_box(dialog_system::box_type type)
{
    switch(type)
    {
        case dialog_system::box_type::think:
            return bn::sprite_items::thinkbox;

        case dialog_system::box_type::speech:
            return bn::sprite_items::speechbox;

        case dialog_system::box_type::shout:
            return bn::sprite_items::shoutbox;

        default:
            return bn::sprite_items::speechbox;
    }
}

const bn::sprite_item& sprite_item_for_tail(dialog_system::box_type type)
{
    if(type == dialog_system::box_type::think)
    {
        return bn::sprite_items::thinktail;
    }

    return bn::sprite_items::speechtail;
}


int box_tile_index_for_cell(int row, int rows, int column, int columns)
{
    const bool top = row == 0;
    const bool bottom = row == rows - 1;
    const bool left = column == 0;
    const bool right = column == columns - 1;

    if(left)
    {
        if(top)
        {
            return 0;   // frame 1: top left
        }

        if(bottom)
        {
            return 2;   // frame 3: bottom left
        }

        return 1;       // frame 2: middle left
    }

    if(right)
    {
        if(top)
        {
            return 6;   // frame 7: top right
        }

        if(bottom)
        {
            return 8;   // frame 9: bottom right
        }

        return 7;       // frame 8: middle right
    }

    if(top)
    {
        return 3;       // frame 4: top middle
    }

    if(bottom)
    {
        return 5;       // frame 6: bottom middle
    }

    return 4;           // frame 5: middle middle
}

}

dialog_system::dialog_system(const bn::camera_ptr& camera) :
    _camera(camera),
    _text_generator(common::variable_8x16_sprite_font)
{
    _text_generator.set_bg_priority(0);
    _text_generator.set_z_order(-10);
    _text_generator.set_one_sprite_per_character(false);
}

void dialog_system::register_actor(const bn::string_view& actor_id, bn::sprite_ptr& sprite, int tail_offset_y)
{
    for(actor_anchor& actor : _actors)
    {
        if(actor.id == actor_id)
        {
            actor.sprite = &sprite;
            actor.tail_offset_y = tail_offset_y;
            return;
        }
    }

    if(_actors.full())
    {
        return;
    }

    actor_anchor actor;
    actor.id = actor_id;
    actor.sprite = &sprite;
    actor.tail_offset_y = tail_offset_y;
    _actors.push_back(actor);
}

bool dialog_system::queue_script_line(const bn::string_view& script_line)
{
    const int last_comma = last_index_of(script_line, ',');

    if(last_comma < 0)
    {
        return false;
    }

    const int second_last_comma = last_index_of(script_line.substr(0, unsigned(last_comma)), ',');

    if(second_last_comma < 0)
    {
        return false;
    }

    const bn::string_view header_and_text = trim_view(script_line.substr(0, unsigned(second_last_comma)));
    const bn::string_view type_view = trim_view(script_line.substr(unsigned(second_last_comma + 1),
                                                                    unsigned(last_comma - second_last_comma - 1)));
    const bn::string_view actor_view = trim_view(script_line.substr(unsigned(last_comma + 1)));

    bn::string<24> speaker_name;
    bn::string<192> dialog_text;

    if(! _parse_header_and_text(header_and_text, speaker_name, dialog_text))
    {
        return false;
    }

    box_type parsed_type = box_type::speech;

    if(! _parse_box_type(type_view, parsed_type))
    {
        return false;
    }

    _wrap_and_append_pages(dialog_text, speaker_name, bn::string<16>(actor_view), parsed_type);

    if(_current_page_index < 0 && ! _pages.empty())
    {
        _current_page_index = 0;
        _show_current_page();
    }

    return true;
}

void dialog_system::clear()
{
    _pages.clear();
    _current_page_index = -1;
    _hide_visuals();
}

void dialog_system::update()
{
    if(! active())
    {
        return;
    }

    if(bn::keypad::a_pressed())
    {
        ++_current_page_index;

        if(_current_page_index >= _pages.size())
        {
            clear();
        }
        else
        {
            _show_current_page();
        }
    }
}

bool dialog_system::active() const
{
    return _current_page_index >= 0 && _current_page_index < _pages.size();
}

bool dialog_system::_parse_header_and_text(const bn::string_view& header_and_text, bn::string<24>& speaker_name,
                                           bn::string<192>& dialog_text) const
{
    if(! header_and_text.starts_with("["))
    {
        return false;
    }

    const int close_bracket = int(header_and_text.find(']'));

    if(close_bracket <= 1)
    {
        return false;
    }

    speaker_name = bn::string<24>(header_and_text.substr(1, unsigned(close_bracket - 1)));
    dialog_text = bn::string<192>(trim_view(header_and_text.substr(unsigned(close_bracket + 1))));
    return ! speaker_name.empty() && ! dialog_text.empty();
}

bool dialog_system::_parse_box_type(const bn::string_view& value, box_type& output_type) const
{
    if(value == "think")
    {
        output_type = box_type::think;
        return true;
    }

    if(value == "speech")
    {
        output_type = box_type::speech;
        return true;
    }

    if(value == "shout")
    {
        output_type = box_type::shout;
        return true;
    }

    return false;
}

int dialog_system::_tiles_for_length(int length) const
{
    if(length <= _tile_size)
    {
        return 1;
    }

    const int remaining = length - _tile_size;
    return 1 + (remaining + _tile_step - 1) / _tile_step;
}

int dialog_system::_length_for_tiles(int tiles) const
{
    if(tiles <= 1)
    {
        return _tile_size;
    }

    return _tile_size + (tiles - 1) * _tile_step;
}

bool dialog_system::_find_actor(const bn::string_view& actor_id, actor_anchor*& output_actor)
{
    for(actor_anchor& actor : _actors)
    {
        if(actor.id == actor_id)
        {
            output_actor = &actor;
            return true;
        }
    }

    output_actor = nullptr;
    return false;
}

void dialog_system::_append_page(const bn::string<24>& speaker_name, const bn::string<16>& actor_id, box_type type,
                                 const bn::string<96>& line1, const bn::string<96>& line2, const bn::string<96>& line3)
{
    if(_pages.full())
    {
        return;
    }

    dialog_page page;
    page.speaker_name = speaker_name;
    page.actor_id = actor_id;
    page.type = type;
    page.line1 = line1;
    page.line2 = line2;
    page.line3 = line3;
    _pages.push_back(page);
}

void dialog_system::_wrap_and_append_pages(const bn::string_view& text, const bn::string<24>& speaker_name,
                                           const bn::string<16>& actor_id, box_type type)
{
    bn::string<96> current_line;
    bn::string<96> current_word;
    bn::string<96> page_line1;
    bn::string<96> page_line2;
    bn::string<96> page_line3;
    int page_line_count = 0;

    auto push_page = [&]() {
        if(page_line_count > 0)
        {
            _append_page(speaker_name, actor_id, type, page_line1, page_line2, page_line3);
            page_line1.clear();
            page_line2.clear();
            page_line3.clear();
            page_line_count = 0;
        }
    };

    auto push_line = [&](const bn::string<96>& line) {
        if(line.empty())
        {
            return;
        }

        if(page_line_count == 3)
        {
            push_page();
        }

        if(page_line_count == 0)
        {
            page_line1 = line;
        }
        else if(page_line_count == 1)
        {
            page_line2 = line;
        }
        else
        {
            page_line3 = line;
        }

        ++page_line_count;
    };

    auto flush_word = [&]() {
        if(current_word.empty())
        {
            return;
        }

        bn::string<96> candidate = current_line;

        if(! candidate.empty())
        {
            candidate.push_back(' ');
        }

        candidate += current_word;

        if(current_line.empty() || _text_generator.width(candidate) <= _content_max_width)
        {
            current_line = candidate;
        }
        else
        {
            push_line(current_line);
            current_line = current_word;
        }

        current_word.clear();
    };

    for(char character : text)
    {
        if(character == '\n')
        {
            flush_word();
            push_line(current_line);
            current_line.clear();
            continue;
        }

        if(character == ' ')
        {
            flush_word();
            continue;
        }

        if(! current_word.full())
        {
            current_word.push_back(character);
        }
    }

    flush_word();
    push_line(current_line);
    push_page();
}

void dialog_system::_show_current_page()
{
    _hide_visuals();

    if(! active())
    {
        return;
    }

    const dialog_page& page = _pages[_current_page_index];
    actor_anchor* actor = nullptr;

    if(! _find_actor(page.actor_id, actor) || ! actor || ! actor->sprite)
    {
        return;
    }

    const bn::sprite_ptr& actor_sprite = *actor->sprite;

    const int line1_width = _text_generator.width(page.line1);
    const int line2_width = _text_generator.width(page.line2);
    const int line3_width = _text_generator.width(page.line3);
    const int max_line_width = bn::max(line1_width, bn::max(line2_width, line3_width));
    const int box_columns = _tiles_for_length(bn::max(_tile_size, max_line_width + _text_padding * 2));
    const int box_rows = _tiles_for_length(_box_height);
    const int name_columns = _tiles_for_length(bn::max(_tile_size, _text_generator.width(page.speaker_name) + _text_padding * 2));
    const int box_width = _length_for_tiles(box_columns);
    const int box_height = _length_for_tiles(box_rows);
    const int name_width = _length_for_tiles(name_columns);

    constexpr bn::fixed screen_left = 0;
    constexpr bn::fixed screen_right = 240;
    constexpr bn::fixed screen_top = 0;
    constexpr bn::fixed screen_bottom = 160;
    constexpr bn::fixed screen_center_x = 120;
    constexpr bn::fixed screen_center_y = 80;
    constexpr bn::fixed actor_width = 32;
    constexpr bn::fixed actor_height = 32;
    const bn::fixed name_height = _tile_size;

    const bn::fixed actor_world_center_x = actor_sprite.x();
    const bn::fixed actor_world_center_y = actor_sprite.y();
    const bn::fixed actor_screen_center_x = actor_world_center_x - _camera.x() + screen_center_x;
    const bn::fixed actor_screen_center_y = actor_world_center_y - _camera.y() + screen_center_y;
    const bn::fixed actor_screen_top_left_x = actor_screen_center_x - (actor_width / 2);
    const bn::fixed actor_screen_top_left_y = actor_screen_center_y - (actor_height / 2);

    constexpr bn::fixed tail_overlap_with_box = 2;
    constexpr bn::fixed tail_gap_above_actor = 4;
    constexpr bn::fixed minimum_dialog_gap_above_actor = 12;
    constexpr bn::fixed tail_side_margin = 4;

    const bn::fixed actor_head_anchor_y = actor_screen_top_left_y;

    bn::fixed box_screen_top_left_x = actor_screen_center_x - (box_width / 2);
    bn::fixed tail_screen_top_left_y = actor_head_anchor_y - _tail_size - tail_gap_above_actor;
    bn::fixed box_screen_top_left_y = tail_screen_top_left_y - box_height + tail_overlap_with_box;

    const bn::fixed dialog_bottom_y = tail_screen_top_left_y + _tail_size;
    const bn::fixed max_dialog_bottom_y = actor_screen_top_left_y - minimum_dialog_gap_above_actor;

    if(dialog_bottom_y > max_dialog_bottom_y)
    {
        const bn::fixed shift_up = dialog_bottom_y - max_dialog_bottom_y;
        tail_screen_top_left_y -= shift_up;
        box_screen_top_left_y -= shift_up;
    }

    if(box_screen_top_left_x < screen_left)
    {
        box_screen_top_left_x = screen_left;
    }
    else
    {
        const bn::fixed box_right = box_screen_top_left_x + box_width;

        if(box_right > screen_right)
        {
            box_screen_top_left_x -= box_right - screen_right;
        }
    }

    const bn::fixed total_top = box_screen_top_left_y - (name_height - 4);

    if(total_top < screen_top)
    {
        const bn::fixed shift_down = screen_top - total_top;
        box_screen_top_left_y += shift_down;
        tail_screen_top_left_y += shift_down;
    }
    else
    {
        const bn::fixed box_bottom = box_screen_top_left_y + box_height;

        if(box_bottom > screen_bottom)
        {
            const bn::fixed shift_up = box_bottom - screen_bottom;
            box_screen_top_left_y -= shift_up;
            tail_screen_top_left_y -= shift_up;
        }
    }

    bn::fixed tail_screen_top_left_x = actor_screen_center_x - (_tail_size / 2);
    const bn::fixed tail_min_x = box_screen_top_left_x + tail_side_margin;
    const bn::fixed tail_max_x = box_screen_top_left_x + box_width - _tail_size - tail_side_margin;

    if(tail_screen_top_left_x < tail_min_x)
    {
        tail_screen_top_left_x = tail_min_x;
    }
    else if(tail_screen_top_left_x > tail_max_x)
    {
        tail_screen_top_left_x = tail_max_x;
    }

    const bn::fixed name_screen_top_left_x = box_screen_top_left_x;
    const bn::fixed name_screen_top_left_y = box_screen_top_left_y - (_tile_size - 4);

    BN_LOG("[dialog] actor=", page.actor_id,
           " speaker=", page.speaker_name,
           " camera=", _camera.x(), ",", _camera.y());
    BN_LOG("[dialog] actor world center=", actor_world_center_x, ",", actor_world_center_y,
           " screen center=", actor_screen_center_x, ",", actor_screen_center_y,
           " screen top_left=", actor_screen_top_left_x, ",", actor_screen_top_left_y,
           " actor_size=", actor_width, "x", actor_height,
           " tail_offset_y=", actor->tail_offset_y,
           " head_anchor_y=", actor_head_anchor_y);
    BN_LOG("[dialog] tail screen top_left=", tail_screen_top_left_x, ",", tail_screen_top_left_y,
           " bottom=", tail_screen_top_left_y + _tail_size,
           " gap_above_actor=", actor_screen_top_left_y - (tail_screen_top_left_y + _tail_size));
    BN_LOG("[dialog] box screen top_left=", box_screen_top_left_x, ",", box_screen_top_left_y,
           " size=", box_width, "x", box_height);
    BN_LOG("[dialog] name screen top_left=", name_screen_top_left_x, ",", name_screen_top_left_y,
           " width=", name_width);

    const bn::fixed tail_top_left_x = tail_screen_top_left_x;
    const bn::fixed tail_top_left_y = tail_screen_top_left_y;
    const bn::fixed box_top_left_x = box_screen_top_left_x;
    const bn::fixed box_top_left_y = box_screen_top_left_y;
    const bn::fixed name_top_left_x = name_screen_top_left_x;
    const bn::fixed name_top_left_y = name_screen_top_left_y;

    _tail_sprite = sprite_item_for_tail(page.type).create_sprite(0, 0);
    _tail_sprite->set_bg_priority(0);
    _tail_sprite->set_z_order(0);
    _tail_sprite->set_top_left_position(tail_top_left_x, tail_top_left_y);
    _tail_sprite->set_visible(true);

    const bn::sprite_item& box_item = sprite_item_for_box(page.type);

    for(int row = 0; row < box_rows && ! _box_sprites.full(); ++row)
    {
        for(int column = 0; column < box_columns && ! _box_sprites.full(); ++column)
        {
            const int box_tile_index = box_tile_index_for_cell(row, box_rows, column, box_columns);

            bn::sprite_ptr sprite = box_item.create_sprite(0, 0);
            sprite.set_bg_priority(0);
            sprite.set_z_order(0);
            sprite.set_tiles(box_item.tiles_item(), box_tile_index);
            sprite.set_top_left_position(box_top_left_x + column * _tile_step,
                                         box_top_left_y + row * _tile_step);
            _box_sprites.push_back(sprite);
        }
    }

    for(int column = 0; column < name_columns && ! _name_box_sprites.full(); ++column)
    {
        bn::sprite_ptr sprite = bn::sprite_items::namebox.create_sprite(0, 0);
        sprite.set_bg_priority(0);
        sprite.set_z_order(0);
        sprite.set_top_left_position(name_top_left_x + column * _tile_step, name_top_left_y);
        _name_box_sprites.push_back(sprite);
    }

    _text_generator.generate_top_left(box_top_left_x + _text_padding, box_top_left_y + _text_padding,
                                      page.line1, _text_sprites);

    if(! page.line2.empty())
    {
        _text_generator.generate_top_left(box_top_left_x + _text_padding,
                                          box_top_left_y + _text_padding + _line_height,
                                          page.line2, _text_sprites);
    }

    if(! page.line3.empty())
    {
        _text_generator.generate_top_left(box_top_left_x + _text_padding,
                                          box_top_left_y + _text_padding + _line_height * 2,
                                          page.line3, _text_sprites);
    }

    _text_generator.generate_top_left(name_top_left_x + _text_padding,
                                      name_top_left_y,
                                      page.speaker_name,
                                      _name_sprites);

    for(bn::sprite_ptr& sprite : _text_sprites)
    {
        sprite.set_bg_priority(0);
        sprite.set_z_order(-30);
    }

    for(bn::sprite_ptr& sprite : _name_sprites)
    {
        sprite.set_bg_priority(0);
        sprite.set_z_order(-30);
    }
}

void dialog_system::_hide_visuals()
{
    _text_sprites.clear();
    _name_sprites.clear();
    _box_sprites.clear();
    _name_box_sprites.clear();

    if(_tail_sprite)
    {
        _tail_sprite->set_visible(false);
    }
}

} // namespace rpg
