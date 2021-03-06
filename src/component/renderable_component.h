#pragma once

/* THIS FILE IS AUTOGENERATED BY gen/gen_component_impl.py; DO NOT HAND-MODIFY */

#include <libconfig.h>
#include <memory>

#include "component_manager.h"
#include "../enums/enums.h"

struct renderable_component_manager : component_manager {
    struct instance_data {
        c_entity *entity;
        const char* *mesh;
        unsigned *material;
        bool *draw;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;
    void destroy_instance(instance i) override;
    void entity(c_entity e) override;

    instance_data get_instance_data(c_entity e) {
        instance_data d{};
        auto inst = lookup(e);

        d.entity = instance_pool.entity + inst.index;
        d.mesh = instance_pool.mesh + inst.index;

        d.material = instance_pool.material + inst.index;

        d.draw = instance_pool.draw + inst.index;

        return d;
    }
};

struct renderable_component_stub : component_stub {
    renderable_component_stub() = default;

    std::string mesh{};

    std::string material{};

    void
    assign_component_to_entity(c_entity entity) override;

    static std::unique_ptr<component_stub> from_config(config_setting_t const *config);
};
