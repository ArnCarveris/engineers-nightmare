#pragma once

/* THIS FILE IS AUTOGENERATED BY gen/gen_component_impl.py; DO NOT HAND-MODIFY */

#include <libconfig.h>
#include <memory>

#include "component_manager.h"

struct power_component_manager : component_manager {
    struct instance_data {
        c_entity *entity;
        float *required_power;
        bool *powered;
        float *max_required_power;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;
    void destroy_instance(instance i) override;
    void entity(c_entity e) override;

    instance_data get_instance_data(c_entity e) {
        instance_data d{};
        auto inst = lookup(e);

        d.entity = instance_pool.entity + inst.index;
        d.required_power = instance_pool.required_power + inst.index;
        d.powered = instance_pool.powered + inst.index;
        d.max_required_power = instance_pool.max_required_power + inst.index;

        return d;
    }
};

struct power_component_stub : component_stub {
    power_component_stub() = default;

    float required_power{};

    float max_required_power{};

    void
    assign_component_to_entity(c_entity entity) override;
};
