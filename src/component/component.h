#pragma once

#include <unordered_map>
#include <glm/glm.hpp>
#include "../mesh.h"

template<typename T>
size_t align_size(size_t s)
{
    s += alignof(T)-1;
    s &= ~(alignof(T)-1);
    return s;
}

template<typename T>
T* align_ptr(T* p) {
    return (T*)align_size<T>((size_t)p);
}

static unsigned entities_id_ref = 1;
struct c_entity {
    unsigned id;

    c_entity() : id(entities_id_ref++) {
    }

    bool operator==(const c_entity &other) const {
        return this->id == other.id;
    }
};

namespace std {
    template<>
    struct hash<c_entity> {
        size_t operator()(c_entity const &e) const {
            return hash<unsigned>()(e.id);
        }
    };
}

struct component_manager {
    struct instance {
        unsigned index;
    };

    struct component_buffer {
        unsigned num;
        unsigned allocated;
        void *buffer;
    } buffer;
    
    std::unordered_map<c_entity, unsigned> entity_instance_map;

    virtual void create_component_instance_data(unsigned count) = 0;

    void assign_entity(const c_entity &e) {
        auto i = make_instance(buffer.num);
        entity_instance_map[e] = i.index;
        entity(e);
        ++buffer.num;
    }

    virtual void entity(const c_entity &e) = 0;

    instance lookup(c_entity e) {
        return make_instance(entity_instance_map.find(e)->second);
    }

    instance make_instance(unsigned i) {
        return { i };
    }

    void destroy_entity_instance(c_entity e) {
        auto i = lookup(e);
        destroy_instance(i);
    }

    virtual void destroy_instance(instance i) = 0;

    virtual ~component_manager() {
        // allocated in derived create_component_instance_data() calls
        free(buffer.buffer);
        buffer.buffer = nullptr;
    }
};

// power component
// powered -- connected to power
// bool
// enabled   -- switched on
// bool

struct power_component_manager : component_manager {
    struct power_instance_data {
        c_entity *entity;
        bool *powered;
        bool *enabled;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;

    void destroy_instance(instance i) override;

    void entity(const c_entity &e) override;

    bool & powered(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.powered[inst.index];
    }

    bool & enabled(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.enabled[inst.index];
    }
};

// gas production component
// gas_type -- not set yet
// unsigned
// flow_rate -- rate of flow
// float
// max_pressure -- don't fill past the line
// float

struct gas_production_component_manager : component_manager {
    struct gas_production_instance_data {
        c_entity *entity;
        unsigned *gas_type;
        float *flow_rate;
        float *max_pressure;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;

    void destroy_instance(instance i) override;

    void entity(const c_entity &e) override;

    unsigned & gas_type(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.gas_type[inst.index];
    }

    float & flow_rate(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.flow_rate[inst.index];
    }

    float & max_pressure(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.max_pressure[inst.index];
    }
};

// position relative to ship component
// position -- relative to ship. :)
// glm::vec3
// mat      -- matrix for above
// glm::mat4

struct relative_position_component_manager : component_manager {
    struct relative_position_instance_data {
        c_entity *entity;
        glm::vec3 *position;
        glm::mat4 *mat;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;

    void destroy_instance(instance i) override;

    void entity(const c_entity &e) override;

    glm::vec3 & position(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.position[inst.index];
    }

    glm::mat4 & mat(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.mat[inst.index];
    }
};

// light component
// intensity -- 0-1
// glm::vec3

struct light_component_manager : component_manager {
    struct light_instance_data {
        c_entity *entity;
        float *intensity;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;

    void destroy_instance(instance i) override;

    void entity(const c_entity &e) override;

    float & intensity(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.intensity[inst.index];
    }
};

// renderable component
// no data

struct renderable_component_manager : component_manager {
    struct renderable_instance_data {
        c_entity *entity;
        hw_mesh *mesh;
    } instance_pool;

    void create_component_instance_data(unsigned count) override;

    void destroy_instance(instance i) override;

    void entity(const c_entity &e) override;

    hw_mesh & mesh(c_entity e) {
        auto inst = lookup(e);

        return instance_pool.mesh[inst.index];
    }
};
