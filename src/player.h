#pragma once

#include <glm/glm.hpp>

#define EYE_OFFSET_Z    0.4

struct player {
    float angle;
    float elev;
    glm::vec3 pos;
    glm::vec3 dir;  /* computed */
    glm::vec3 eye;  /* computed */
    glm::vec2 move;

    int selected_slot;

    bool jump;

    bool use;

    bool reset;

    bool last_left_button;
    bool left_button;

    bool crouch;
    bool crouch_end;

    bool gravity;

    bool menu_requested = false;

    bool ui_dirty;

    /* used within physics to toggle gravity */
    bool disable_gravity;
};

