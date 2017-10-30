#include <btBulletDynamicsCommon.h>
#include <stdio.h>

#include "player.h"
#include "physics.h"

#define MOVE_SPEED  0.07f
#define CROUCH_FACTOR 0.4f
#define AIR_CONTROL_FACTOR 0.25f
#include <algorithm>

/* a simple constructor hacked together based on
 * http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World
 */
physics::physics(player *p)
{
    /* a broadspace filters out obvious non-colliding pairs
     * before the more expensive collision detection algorithm sees them
     *
     * FIXME currently using default broadphase
     * see http://bulletphysics.org/mediawiki-1.5.8/index.php/Broadphase
     * for more information
     */
    this->broadphase = new btDbvtBroadphase();

    this->collisionConfiguration = new btDefaultCollisionConfiguration();
    this->dispatcher = new btCollisionDispatcher(this->collisionConfiguration);

    /* the magic sauce that makes everything else work */
    this->solver = new btSequentialImpulseConstraintSolver;

    /* our actual world */
    this->dynamicsWorld =
        new btDiscreteDynamicsWorld(this->dispatcher,
                                    this->broadphase,
                                    this->solver,
                                    this->collisionConfiguration);

    /* some default gravity
     * z is up and down
     */
    this->dynamicsWorld->setGravity(btVector3(0, 0, -10));

    /* store a pointer to our player so physics can drive his position */
    this->pl = p;

    /* set player height to physics height */
//    pl->pos.x = PLAYER_START_X;
//    pl->pos.y = PLAYER_START_Y;
//    pl->pos.z = PLAYER_START_Z;


    /* setup player rigid body */
    /* player height is 2 * radius + height
     * therefore, height for capsule is
     * total height - 2 * radius
     */
    auto standHeight = std::max(0.f, PLAYER_STAND_HEIGHT - 2 * PLAYER_RADIUS);
    this->standShape = new btCapsuleShapeZ(PLAYER_RADIUS, standHeight);
    auto crouchHeight = std::max(0.f, PLAYER_CROUCH_HEIGHT - 2 * PLAYER_RADIUS);
    this->crouchShape = new btCapsuleShapeZ(PLAYER_RADIUS, crouchHeight);
    float maxStepHeight = 0.15f;

    /* setup the character controller. this gets a bit fiddly. */
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(pl->pos.x, pl->pos.y, pl->pos.z));
    this->ghostObj = new btPairCachingGhostObject();
    this->ghostObj->setWorldTransform(startTransform);
    this->broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    this->ghostObj->setCollisionShape(this->standShape);
    this->ghostObj->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    this->controller = new en_char_controller(this->ghostObj, this->standShape, this->crouchShape, btScalar(maxStepHeight));

    this->dynamicsWorld->addCollisionObject(this->ghostObj, btBroadphaseProxy::CharacterFilter,
            btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
    this->dynamicsWorld->addAction(this->controller);
    this->controller->setUpAxis(2);
}

physics::~physics()
{
    delete(this->broadphase);

    delete(this->collisionConfiguration);
    delete(this->dispatcher);

    delete(this->solver);

    delete(this->dynamicsWorld);

    delete(this->standShape);
    delete(this->crouchShape);
    delete(this->ghostObj);
    delete(this->controller);
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

        if( this->pl->disable_gravity ){
            this->controller->setGravity(0);
        } else {
            /* http://bulletphysics.org/Bullet/BulletFull/btKinematicCharacterController_8cpp_source.html : 144
             * 3G acceleration.
             */
            this->controller->setGravity(9.8f * 3);
        }

        pl->ui_dirty = true;
    }

    float speed = MOVE_SPEED;
    if (!this->controller->onGround())
        speed *= AIR_CONTROL_FACTOR;
    else if (this->controller->isCrouching())
        speed *= CROUCH_FACTOR;

    pl->height = this->controller->isCrouching() ? PLAYER_CROUCH_HEIGHT : PLAYER_STAND_HEIGHT;

    fwd *= this->pl->move.y * speed;
    right *= this->pl->move.x * speed;

    this->controller->setWalkDirection(fwd + right);

    if (pl->jump && this->controller->onGround())
        this->controller->jump();

    if (pl->reset) {
        /* reset position (for debug) */
        this->controller->warp(btVector3(0, 0, 0));
    }

    if (pl->crouch) {
        this->controller->crouch(this->dynamicsWorld);
    }
    else if (pl->crouch_end) {
        this->controller->crouchEnd();
    }
}

void
physics::tick(float dt)
{
    dynamicsWorld->stepSimulation(dt, 10);

    btTransform trans = this->ghostObj->getWorldTransform();

    this->pl->pos.x = trans.getOrigin().getX();
    this->pl->pos.y = trans.getOrigin().getY();
    this->pl->pos.z = trans.getOrigin().getZ();
}
