#pragma once

#include "bn_camera_ptr.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_string.h"
#include "bn_string_view.h"
#include "bn_vector.h"

namespace rpg {

class dialog_system
{
public:
    explicit dialog_system(const bn::camera_ptr& camera);

    void register_actor(const bn::string_view& actor_id, bn::sprite_ptr& sprite, int tail_offset_y = 8);

    bool queue_script_line(const bn::string_view& script_line);
    void clear();
    void update();

    [[nodiscard]] bool active() const;

public:
    enum class box_type : uint8_t
    {
        think,
        speech,
        shout,
    };

private:
    struct actor_anchor
    {
        bn::string<16> id;
        bn::sprite_ptr* sprite = nullptr;
        int tail_offset_y = 8;
    };

    struct dialog_page
    {
        bn::string<24> speaker_name;
        bn::string<16> actor_id;
        box_type type = box_type::speech;
        bn::string<96> line1;
        bn::string<96> line2;
        bn::string<96> line3;
    };

    static constexpr int _max_actors = 8;
    static constexpr int _max_pages = 16;
    static constexpr int _max_text_sprites = 128;
    static constexpr int _max_box_sprites = 48;
    static constexpr int _max_name_box_sprites = 8;
    static constexpr int _line_height = 16;
    static constexpr int _text_padding = 8;
    static constexpr int _content_max_width = 112;
    static constexpr int _box_height = _text_padding * 2 + _line_height * 3;
    static constexpr int _tile_size = 16;
    static constexpr int _tile_overlap = 2;
    static constexpr int _tile_step = _tile_size - _tile_overlap;
    static constexpr int _tail_size = 8;

    bool _parse_header_and_text(const bn::string_view& header_and_text, bn::string<24>& speaker_name,
                                bn::string<192>& dialog_text) const;
    [[nodiscard]] int _tiles_for_length(int length) const;
    [[nodiscard]] int _length_for_tiles(int tiles) const;
    bool _parse_box_type(const bn::string_view& value, box_type& output_type) const;
    bool _find_actor(const bn::string_view& actor_id, actor_anchor*& output_actor);

    void _wrap_and_append_pages(const bn::string_view& text, const bn::string<24>& speaker_name,
                                const bn::string<16>& actor_id, box_type type);
    void _append_page(const bn::string<24>& speaker_name, const bn::string<16>& actor_id, box_type type,
                      const bn::string<96>& line1, const bn::string<96>& line2, const bn::string<96>& line3);
    void _show_current_page();
    void _hide_visuals();

    bn::camera_ptr _camera;
    bn::sprite_text_generator _text_generator;
    bn::vector<actor_anchor, _max_actors> _actors;
    bn::vector<dialog_page, _max_pages> _pages;
    int _current_page_index = -1;

    bn::optional<bn::sprite_ptr> _tail_sprite;
    bn::vector<bn::sprite_ptr, _max_box_sprites> _box_sprites;
    bn::vector<bn::sprite_ptr, _max_name_box_sprites> _name_box_sprites;
    bn::vector<bn::sprite_ptr, _max_text_sprites> _text_sprites;
    bn::vector<bn::sprite_ptr, 24> _name_sprites;
};

} // namespace rpg
