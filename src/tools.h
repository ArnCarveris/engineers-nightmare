#pragma once

#include "block.h"

struct player;
struct raycast_info;


struct tool
{
    virtual ~tool() {}

    virtual void use(raycast_info *rc) = 0;
    virtual void alt_use(raycast_info *rc) = 0;
    virtual void long_use(raycast_info *rc) = 0;

    virtual void cycle_mode() = 0;

    virtual void preview(raycast_info *rc) = 0;
    virtual void get_description(char *str) = 0;

    static tool *create_add_block_tool();
    static tool *create_remove_block_tool();
    static tool *create_add_surface_tool(surface_type s);
    static tool *create_remove_surface_tool();
    static tool *create_fire_projectile_tool(player *pl);
};
