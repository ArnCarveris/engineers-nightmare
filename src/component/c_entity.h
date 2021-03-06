#pragma once

static unsigned entities_id_ref = 1;

struct c_entity {
    unsigned id{0};

    bool operator==(c_entity const &other) const {
        return this->id == other.id;
    }

    bool operator!=(c_entity const &other) const {
        return this->id != other.id;
    }

    bool operator<(c_entity const &other) const {
        return this->id < other.id;
    }

    static c_entity spawn() {
        c_entity e = { entities_id_ref++ };
        return e;
    }

    static bool is_valid(c_entity const &check) {
        return check.id != 0;
    }
};
