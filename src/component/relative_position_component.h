#pragma once

/* THIS FILE IS AUTOGENERATED BY gen/gen_component_impl.py; DO NOT HAND-MODIFY */

#include "component_manager.h"

struct relative_position_component_manager : component_manager {
    struct instance_data {
        c_entity *entity;
        glm::vec3 *position;
        glm::mat4 *mat;
        float *rotation;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;
    void destroy_instance(instance i) override;
    void entity(c_entity e) override;

    instance_data get_instance_data(c_entity e) {
        instance_data d;
        auto inst = lookup(e);

        d.entity = instance_pool.entity + inst.index;
        d.position = instance_pool.position + inst.index;
        d.mat = instance_pool.mat + inst.index;
        d.rotation = instance_pool.rotation + inst.index;

        return d;
    }
};
