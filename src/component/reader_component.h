#pragma once

/* THIS FILE IS AUTOGENERATED BY gen/gen_component_impl.py; DO NOT HAND-MODIFY */

#include <libconfig.h>
#include <memory>

#include "component_manager.h"

struct reader_component_manager : component_manager {
    struct instance_data {
        c_entity *entity;
        char const * *name;
        c_entity *source;
        char const * *desc;
        float *data;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;
    void destroy_instance(instance i) override;
    void entity(c_entity e) override;

    instance_data get_instance_data(c_entity e) {
        instance_data d{};
        auto inst = lookup(e);

        d.entity = instance_pool.entity + inst.index;
        d.name = instance_pool.name + inst.index;

        d.source = instance_pool.source + inst.index;

        d.desc = instance_pool.desc + inst.index;

        d.data = instance_pool.data + inst.index;

        return d;
    }
};

struct reader_component_stub : component_stub {
    reader_component_stub() = default;

    std::string name{};

    std::string desc{};

    void
    assign_component_to_entity(c_entity entity) override;

    static std::unique_ptr<component_stub> from_config(config_setting_t const *config);
};
