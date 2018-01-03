#include <epoxy/gl.h>
#include <glm/ext.hpp>
#include <array>
#include <limits>


#include "../asset_manager.h"
#include "../common.h"
#include "../ship_space.h"
#include "../mesh.h"
#include "../block.h"
#include "../player.h"
#include "tools.h"
#include "../utils/debugdraw.h"


extern GLuint overlay_shader;
extern GLuint simple_shader;

extern ship_space *ship;
extern player pl;

extern asset_manager asset_man;

const static std::array<glm::ivec3, 4> room_sizes{glm::ivec3(4, 4, 4),
                                                  glm::ivec3(4, 6, 4),
                                                  glm::ivec3(5, 5, 5),
                                                  glm::ivec3(7, 7, 7),
};

constexpr glm::ivec2 door_size = {2, 2};

struct add_room_tool : tool {
    unsigned room_size = 0;

    raycast_info_block rc;

    void pre_use(player *pl) override {
        ship->raycast_block(pl->eye, pl->dir, MAX_REACH_DISTANCE, cross_surface, &rc);
    }

    bool can_use() {
        return rc.hit;
    }

    void use() override {
        if (!can_use()) return;

        auto size = room_sizes[room_size];
        auto f = -rc.n;
        auto u = surface_index_to_normal(surface_zp);
        glm::ivec3 ln = glm::ivec3(-f.y, f.x, f.z);

        auto next_block = rc.bl + f;
        auto front_left = next_block + (ln * (size.y / 2));
        auto back_right = next_block + (f * (size.x - 1)) - ln * (size.y / 2);

        auto mins = glm::min(front_left, back_right);
        auto maxs = glm::max(front_left, back_right);

        // cut out room
        ship->cut_out_cuboid(mins, maxs, surface_wall);

        // cut out door
//        auto door = glm::vec3(2, 2, 1);
//        ship->cut_out_cuboid(door_base, door_base + u, surface_wall);

        ship->validate();
    }

    void cycle_mode() override {
        room_size++;
        if (room_size >= room_sizes.size()) {
            room_size = 0;
        }
    }

    glm::vec3 get_closest_alignment(std::vector<glm::vec3> cand, glm::vec3 v) {

        glm::ivec3 best;
        float bestdot = std::numeric_limits<float>::lowest();
        for (auto c : cand) {
            auto val = glm::dot(c, v);
            if (val > bestdot) {
                best = c;
                bestdot = val;
            }
        }

        return best;
    }

    void preview(frame_data *frame) override {
        if (!rc.hit)
            return;

        if (can_use()) {
            auto door_center = glm::round(rc.hitCoord);
            glm::vec3 block = rc.bl;
            glm::vec3 forward = -rc.n;
            auto v = door_center;

            auto m = glm::mat4_cast(glm::normalize(pl.rot));
            auto right = glm::vec3(m[0]);
            auto up = glm::vec3(m[1]);

            std::vector<glm::vec3> dirs = {surface_index_to_normal(surface_xp), surface_index_to_normal(surface_xm),
                         surface_index_to_normal(surface_yp), surface_index_to_normal(surface_ym),
                         surface_index_to_normal(surface_zp), surface_index_to_normal(surface_zm)};
            auto r = get_closest_alignment(dirs, right);
            auto u = get_closest_alignment(dirs, up);

            dd::sphere(glm::value_ptr(door_center), dd::colors::Yellow, 0.05f);

            auto fix = rc.hitCoord - (glm::vec3)(glm::ivec3)rc.hitCoord;
            if (r == dirs[0] && fix.x < 0.5) {
                block.x--;
            }
            else if (r == dirs[1] && fix.x > 0.5) {
                block.x++;
            }
            else if (r == dirs[2] && fix.y < 0.5) {
                block.y--;
            }
            else if (r == dirs[3] && fix.y > 0.5) {
                block.y++;
            }
            else if (r == dirs[4] && fix.z < 0.5) {
                block.z--;
            }
            else if (r == dirs[5] && fix.z > 0.5) {
                block.z++;
            }

            if (u == dirs[0] && fix.x < 0.5) {
                block.x--;
            }
            else if (u == dirs[1] && fix.x > 0.5) {
                block.x++;
            }
            else if (u == dirs[2] && fix.y < 0.5) {
                block.y--;
            }
            else if (u == dirs[3] && fix.y > 0.5) {
                block.y++;
            }
            else if (u == dirs[4] && fix.z < 0.5) {
                block.z--;
            }
            else if (u == dirs[5] && fix.z > 0.5) {
                block.z++;
            }

            std::array<glm::vec3, 4> doors;
            doors[0] = block;
            doors[1] = block + r;
            doors[2] = block + u;
            doors[3] = block + u + r;

            for (auto & door : doors) {
                auto mesh = asset_man.get_mesh("frame");

                auto mat = frame->alloc_aligned<mesh_instance>(1);
                mat.ptr->world_matrix = mat_position(glm::vec3(door));
                mat.ptr->color = glm::vec4(1.f, 0.f, 0.f, 1.f);
                mat.bind(1, frame);

                glUseProgram(overlay_shader);
                glEnable(GL_POLYGON_OFFSET_FILL);
                draw_mesh(mesh.hw);
                glDisable(GL_POLYGON_OFFSET_FILL);
                glUseProgram(simple_shader);
            }
        }
    }

    void get_description(char *str) override {
        sprintf(str, "Add room: %s\n", glm::to_string(room_sizes[room_size]).c_str());
    }
};


tool *tool::create_add_room_tool() { return new add_room_tool; }