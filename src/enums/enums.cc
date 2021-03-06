/* THIS FILE IS AUTOGENERATED BY gen/gen_enums.py; DO NOT HAND-MODIFY */

#include "enums.h"

#include <cassert>
#include <cstring>


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const char* get_enum_description(window_mode value) {
    switch(value)
    {
    case window_mode::windowed:
        return "Windowed";
    case window_mode::fullscreen:
        return "Fullscreen";
    default:
        assert(false);
        return nullptr;
    }
}

const char* get_enum_string(window_mode value) {
    switch(value)
    {
    case window_mode::windowed:
        return "windowed";
    case window_mode::fullscreen:
        return "fullscreen";
    default:
        assert(false);
        return nullptr;
    }
}

template<> window_mode get_enum<window_mode>(const char *e) {
    auto val{window_mode::invalid};
    if (!strcmp(e, "windowed")) {
        val = window_mode::windowed;
    }
    if (!strcmp(e, "fullscreen")) {
        val = window_mode::fullscreen;
    }
    assert(val != window_mode::invalid);
    return val;
}

window_mode config_setting_get_window_mode(const config_setting_t *setting) {
    const char *str = config_setting_get_string(setting);
    return get_enum<window_mode>(str);
}

int config_setting_set_window_mode(config_setting_t *setting, window_mode value) {
    auto str = get_enum_string(value);
    return (config_setting_set_string(setting, str));
}

int config_setting_lookup_window_mode(const config_setting_t *setting, const char *name, window_mode *value) {
    auto *member = config_setting_get_member(setting, name);
    if(!member) {
        return CONFIG_FALSE;
    }

    *value = (window_mode)config_setting_get_window_mode(member);
    return CONFIG_TRUE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const char* get_enum_description(placement value) {
    switch(value)
    {
    case placement::full_block_snapped:
        return "placement";
    case placement::half_block_snapped:
        return "placement";
    case placement::quarter_block_snapped:
        return "placement";
    case placement::eighth_block_snapped:
        return "placement";
    default:
        assert(false);
        return nullptr;
    }
}

const char* get_enum_string(placement value) {
    switch(value)
    {
    case placement::full_block_snapped:
        return "full_block_snapped";
    case placement::half_block_snapped:
        return "half_block_snapped";
    case placement::quarter_block_snapped:
        return "quarter_block_snapped";
    case placement::eighth_block_snapped:
        return "eighth_block_snapped";
    default:
        assert(false);
        return nullptr;
    }
}

template<> placement get_enum<placement>(const char *e) {
    auto val{placement::invalid};
    if (!strcmp(e, "full_block_snapped")) {
        val = placement::full_block_snapped;
    }
    if (!strcmp(e, "half_block_snapped")) {
        val = placement::half_block_snapped;
    }
    if (!strcmp(e, "quarter_block_snapped")) {
        val = placement::quarter_block_snapped;
    }
    if (!strcmp(e, "eighth_block_snapped")) {
        val = placement::eighth_block_snapped;
    }
    assert(val != placement::invalid);
    return val;
}

placement config_setting_get_placement(const config_setting_t *setting) {
    const char *str = config_setting_get_string(setting);
    return get_enum<placement>(str);
}

int config_setting_set_placement(config_setting_t *setting, placement value) {
    auto str = get_enum_string(value);
    return (config_setting_set_string(setting, str));
}

int config_setting_lookup_placement(const config_setting_t *setting, const char *name, placement *value) {
    auto *member = config_setting_get_member(setting, name);
    if(!member) {
        return CONFIG_FALSE;
    }

    *value = (placement)config_setting_get_placement(member);
    return CONFIG_TRUE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const char* get_enum_description(rotation value) {
    switch(value)
    {
    case rotation::axis_aligned:
        return "rotation";
    case rotation::rot_45:
        return "rotation";
    case rotation::rot_15:
        return "rotation";
    case rotation::no_rotation:
        return "rotation";
    default:
        assert(false);
        return nullptr;
    }
}

const char* get_enum_string(rotation value) {
    switch(value)
    {
    case rotation::axis_aligned:
        return "axis_aligned";
    case rotation::rot_45:
        return "rot_45";
    case rotation::rot_15:
        return "rot_15";
    case rotation::no_rotation:
        return "no_rotation";
    default:
        assert(false);
        return nullptr;
    }
}

template<> rotation get_enum<rotation>(const char *e) {
    auto val{rotation::invalid};
    if (!strcmp(e, "axis_aligned")) {
        val = rotation::axis_aligned;
    }
    if (!strcmp(e, "rot_45")) {
        val = rotation::rot_45;
    }
    if (!strcmp(e, "rot_15")) {
        val = rotation::rot_15;
    }
    if (!strcmp(e, "no_rotation")) {
        val = rotation::no_rotation;
    }
    assert(val != rotation::invalid);
    return val;
}

rotation config_setting_get_rotation(const config_setting_t *setting) {
    const char *str = config_setting_get_string(setting);
    return get_enum<rotation>(str);
}

int config_setting_set_rotation(config_setting_t *setting, rotation value) {
    auto str = get_enum_string(value);
    return (config_setting_set_string(setting, str));
}

int config_setting_lookup_rotation(const config_setting_t *setting, const char *name, rotation *value) {
    auto *member = config_setting_get_member(setting, name);
    if(!member) {
        return CONFIG_FALSE;
    }

    *value = (rotation)config_setting_get_rotation(member);
    return CONFIG_TRUE;
}
