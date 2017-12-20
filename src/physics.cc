#include <btBulletDynamicsCommon.h>
#include <stdio.h>

#include "player.h"
#include "physics.h"
#include "utils/debugdraw.h"

#define MOVE_SPEED  0.07f
#define CROUCH_FACTOR 0.4f
#define AIR_CONTROL_FACTOR 0.25f
#include <algorithm>
#include <glm/ext.hpp>

/* a simple constructor hacked together based on
 * http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World
 */
physics::physics(player *p)
    :
    broadphase(new btDbvtBroadphase()),
    collisionConfiguration(new btDefaultCollisionConfiguration()),
    dispatcher(new btCollisionDispatcher(collisionConfiguration.get())),
    solver(new btSequentialImpulseConstraintSolver),
    dynamicsWorld(new btDiscreteDynamicsWorld(
        dispatcher.get(),
        broadphase.get(),
        solver.get(),
        collisionConfiguration.get()))
{
    /* some default gravity
     * z is up and down
     */
    this->dynamicsWorld->setGravity(btVector3(0, 0, 0));

    /* store a pointer to our player so physics can drive his position */
    this->pl = p;

    rb_controller = std::make_unique<en_rb_controller>(mat_position(pl->pos));
    dynamicsWorld->addRigidBody(rb_controller.get());
}

void
physics::tick_controller(float dt)
{
    /* messy input -> char controller binding
     * TODO: untangle.
     */

    float c = cosf(this->pl->angle);
    float s = sinf(this->pl->angle);

    btVector3 fwd(c, s, 0);
    btVector3 right(s, -c, 0);

    if (pl->gravity) {
        pl->disable_gravity ^= true;

        pl->ui_dirty = true;
    }

    if (pl->jump) {
        rb_controller->applyCentralImpulse(vec3_to_bt(glm::normalize(pl->dir) * 125.0f));
    }
    if (pl->crouch) {
        rb_controller->setLinearVelocity({0, 0, 0});
    }
}

void
physics::tick(float dt)
{
    dynamicsWorld->stepSimulation(dt, 10);

    btTransform trans = this->rb_controller->getWorldTransform();

    this->pl->pos.x = trans.getOrigin().getX();
    this->pl->pos.y = trans.getOrigin().getY();
    this->pl->pos.z = trans.getOrigin().getZ();
}
