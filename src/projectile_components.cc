#include "projectile_components.h"
#include "physics.h"
#include <algorithm>
#include "mesh.h"
#include "shader_params.h"

extern physics *phy;
extern hw_mesh *projectile_hw;

extern glm::mat4
mat_position(glm::vec3);

#define MAX_PROJECTILES 2000

void
projectile_manager::create_component_instance_data(unsigned count) {
    if (count <= buffer.allocated)
        return;

    component_buffer new_buffer;
    projectile_instance_data new_pool;

    size_t size = sizeof(c_entity) * count;
    size = sizeof(float) * count + align_size<float>(size);
    size = sizeof(float) * count + align_size<float>(size);
    size = sizeof(glm::vec3) * count + align_size<glm::vec3>(size);
    size = sizeof(glm::vec3) * count + align_size<glm::vec3>(size);

    new_buffer.buffer = malloc(size);
    new_buffer.num = buffer.num;
    new_buffer.allocated = count;
    memset(new_buffer.buffer, 0, size);

    new_pool.entity = (c_entity *)new_buffer.buffer;
    new_pool.mass = align_ptr((float *)(new_pool.entity + count));
    new_pool.lifetime = align_ptr((float *)(new_pool.mass + count));
    new_pool.position = align_ptr((glm::vec3 *)(new_pool.lifetime + count));
    new_pool.velocity = align_ptr((glm::vec3 *)(new_pool.position + count));

    memcpy(new_pool.entity, projectile_pool.entity, buffer.num * sizeof(c_entity));
    memcpy(new_pool.mass, projectile_pool.mass, buffer.num * sizeof(float));
    memcpy(new_pool.lifetime, projectile_pool.lifetime, buffer.num * sizeof(float));
    memcpy(new_pool.position, projectile_pool.position, buffer.num * sizeof(glm::vec3));
    memcpy(new_pool.velocity, projectile_pool.velocity, buffer.num * sizeof(glm::vec3));

    free(buffer.buffer);
    buffer = new_buffer;

    projectile_pool = new_pool;
}

void
projectile_manager::spawn(glm::vec3 pos, glm::vec3 vel) {
    if (buffer.num >= buffer.allocated) {
        // lazy allocate more than we need with room to spare
        auto count = std::max(buffer.num, (unsigned)MAX_PROJECTILES);
        create_component_instance_data(count * 2);
    }

    auto i = make_instance(buffer.num);

    set_position(i, pos);
    set_velocity(i, vel);
    set_lifetime(i, initial_lifetime);

    ++buffer.num;
}

void
projectile_manager::draw() {
    for (auto i = 0u; i < buffer.num; ++i) {
        auto inst = make_instance(i);
        /* TODO: instancing etc to get rid of this upload/draw loop */
        per_object->val.world_matrix = mat_position(position(inst));
        per_object->upload();
        draw_mesh(projectile_hw);
    }
}

void
projectile_manager::destroy_instance(instance i) {
    auto last_id = buffer.num - 1;
    auto last = projectile_pool.entity[last_id];

    projectile_pool.entity[i.index] = projectile_pool.entity[last_id];
    projectile_pool.mass[i.index] = projectile_pool.mass[last_id];
    projectile_pool.lifetime[i.index] = projectile_pool.lifetime[last_id];
    projectile_pool.position[i.index] = projectile_pool.position[last_id];
    projectile_pool.velocity[i.index] = projectile_pool.velocity[last_id];

    entity_instance_map[last] = i.index;
    entity_instance_map.erase(last);

    --buffer.num;
}

void projectile_linear_manager::simulate(float dt) {
    for (auto i = 0u; i < buffer.num; ) {
        auto inst = make_instance(i);
        auto new_pos = position(inst) + velocity(inst) * dt;

        auto hit = phys_raycast_generic(position(inst), new_pos,
            phy->ghostObj, phy->dynamicsWorld);

        if (hit.hit) {
            new_pos = hit.hitCoord;
            set_velocity(inst, glm::vec3(0));
            set_lifetime(inst, after_collision_lifetime);
        }

        set_position(inst, new_pos);

        set_lifetime(inst, lifetime(inst) - dt);

        if (lifetime(inst) <= 0.f) {
            destroy_instance(inst);
            --i;
        }

        ++i;
    }
}

void projectile_sine_manager::simulate(float dt) {
    static auto time = 0.f;
    time += dt;
    for (auto i = 0u; i < buffer.num; ) {
        auto inst = make_instance(i);
        if (velocity(inst) != glm::vec3(0)) {
            auto new_pos = position(inst) + velocity(inst) * dt;
            new_pos.z += sin(lifetime(inst) * 20)* 0.01f;

            auto hit = phys_raycast_generic(position(inst), new_pos,
                phy->ghostObj, phy->dynamicsWorld);

            if (hit.hit) {
                new_pos = hit.hitCoord;
                set_velocity(inst, glm::vec3(0));
                set_lifetime(inst, after_collision_lifetime);
            }

            set_position(inst, new_pos);
        }

        set_lifetime(inst, lifetime(inst) - dt);

        if (lifetime(inst) <= 0.f) {
            destroy_instance(inst);
            --i;
        }

        ++i;
    }
}
