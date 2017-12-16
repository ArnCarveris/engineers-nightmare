#include <epoxy/gl.h>

#include "../asset_manager.h"
#include "../common.h"
#include "../ship_space.h"
#include "../mesh.h"
#include "tools.h"


extern GLuint overlay_shader;
extern GLuint simple_shader;

extern ship_space *ship;

extern asset_manager asset_man;

struct remove_entity_tool : tool
{
    bool can_use(raycast_info *rc) {
        return rc->block.hit;
    }

    void use(raycast_info *rc) override {
        if (!can_use(rc))
            return;

        int index = normal_to_surface_index(&rc->block);
        remove_ents_from_surface(rc->block.p, index ^ 1);
    }

    void alt_use(raycast_info *rc) override {}

    void long_use(raycast_info *rc) override {}

    void cycle_mode() override {}

    void preview(raycast_info *rc, frame_data *frame) override {
        if (!can_use(rc))
            return;

        int index = normal_to_surface_index(&rc->block);
        block *other_side = ship->get_block(rc->block.p);

        if (!other_side) {
            return;
        }

        auto material = asset_man.get_world_texture_index("white");

        auto mat = frame->alloc_aligned<mesh_instance>(1);
        mat.ptr->world_matrix = mat_position(glm::vec3(rc->block.bl));
        mat.ptr->material = material;
        mat.bind(1, frame);

        auto surf_mesh = asset_man.get_surface_mesh(index, surface_wall);

        glUseProgram(overlay_shader);
        glEnable(GL_POLYGON_OFFSET_FILL);
        draw_mesh(surf_mesh.hw);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glUseProgram(simple_shader);
    }

    void get_description(char *str) override {
        strcpy(str, "Remove entity");
    }
};

tool *tool::create_remove_entity_tool() { return new remove_entity_tool(); }
