#pragma once

#include "component_manager.h"

// type component
// type -- entity type index
// unsigned

struct type_component_manager : component_manager {
    struct switchable_instance_data {
        c_entity *entity;
        unsigned *type;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;

    void destroy_instance(instance i) override;

    void entity(c_entity const &e) override;

    unsigned & type(c_entity const &e) {
        auto inst = lookup(e);

        return instance_pool.type[inst.index];
    }
};