#include "ship_space.h"
#include <assert.h>
#include <math.h>
#include <algorithm>

/* create a ship space of x * y * z instantiated chunks */
ship_space::ship_space(unsigned int xd, unsigned int yd, unsigned int zd)
    : min_x(0), min_y(0), min_z(0), topo_dirty(true), num_full_rebuilds(0), num_fast_unifys(0), num_fast_nosplits(0)
{
    unsigned int x = 0,
                 y = 0,
                 z = 0;

    for( x = 0; x < xd; ++x ){
        for( y = 0; y < yd; ++y ){
            for( z = 0; z < zd; ++z ){
                glm::ivec3 v(x, y, z);
                this->chunks[v] = new chunk();
            }
        }
    }

    /* dim is exclusive, max is inclusive
     * so subtract 1 and store
     */
    this->max_x = xd - 1;
    this->max_y = yd - 1;
    this->max_z = zd - 1;
}

/* create an empty ship_space */
ship_space::ship_space(void)
    : min_x(0), min_y(0), min_z(0), max_x(0), max_y(0), max_z(0), topo_dirty(true),
      num_full_rebuilds(0), num_fast_unifys(0), num_fast_nosplits(0)
{
}


static void
split_coord(int p, int *out_block, int *out_chunk)
{
    /* NOTE: There are a number of attractive-looking symmetries which are
     * just plain wrong. */
    int block, chunk;

    if (p < 0) {
        /* negative space is not a mirror of positive:
         * chunk -1 spans blocks -8..-1;
         * chunk -2 spans blocks -16..-9 */
        chunk = (p - CHUNK_SIZE + 1) / CHUNK_SIZE;
    } else {
        /* positive halfspace has no rocket science. */
        chunk = p / CHUNK_SIZE;
    }

    /* the within-chunk offset is just the difference between the minimum block
     * in the chunk and the requested one, regardless of which halfspace we're in. */
    block = p - CHUNK_SIZE * chunk;

    /* write the outputs which were requested */
    if (out_block)
        *out_block = block;
    if (out_chunk)
        *out_chunk = chunk;
}


/* returns a block or null
 * finds the block at the position (x,y,z) within
 * the whole ship_space
 * will move across chunks
 */
block *
ship_space::get_block(int block_x, int block_y, int block_z)
{
    /* Within Block coordinates */
    int wb_x, wb_y, wb_z;
    int chunk_x, chunk_y, chunk_z;

    split_coord(block_x, &wb_x, &chunk_x);
    split_coord(block_y, &wb_y, &chunk_y);
    split_coord(block_z, &wb_z, &chunk_z);

    chunk *c = this->get_chunk(chunk_x, chunk_y, chunk_z);

    if( ! c ){
        return 0;
    }

    return c->blocks.get(wb_x, wb_y, wb_z);
}

/* returns a topo_info or null
 * finds the topo_info at the position (x,y,z) within
 * the whole ship_space
 * will move across chunks
 */
topo_info *
ship_space::get_topo_info(int block_x, int block_y, int block_z)
{
    /* Within Block coordinates */
    int wb_x, wb_y, wb_z;
    int chunk_x, chunk_y, chunk_z;

    split_coord(block_x, &wb_x, &chunk_x);
    split_coord(block_y, &wb_y, &chunk_y);
    split_coord(block_z, &wb_z, &chunk_z);

    chunk *c = this->get_chunk(chunk_x, chunk_y, chunk_z);

    if (!c) {
        return &this->outside_topo_info;
    }

    return c->topo.get(wb_x, wb_y, wb_z);
}

/* returns the chunk containing the block denotated by (x, y, z)
 * or null
 */
chunk *
ship_space::get_chunk_containing(int block_x, int block_y, int block_z)
{
    int chunk_x, chunk_y, chunk_z;

    split_coord(block_x, NULL, &chunk_x);
    split_coord(block_y, NULL, &chunk_y);
    split_coord(block_z, NULL, &chunk_z);

    return this->get_chunk(chunk_x, chunk_y, chunk_z);
}

/* returns the chunk corresponding to the chunk coordinates (x, y, z)
 * note this is NOT using block coordinates
 */
chunk *
ship_space::get_chunk(int chunk_x, int chunk_y, int chunk_z)
{
    glm::ivec3 v(chunk_x, chunk_y, chunk_z);

    auto it = this->chunks.find(v);
    if( it != this->chunks.end() ){
        return it->second;
    }

    return NULL;
}



static float
max_along_axis(float o, float d)
{
    if (d > 0) {
        return fabsf((ceilf(o) - o)/d);
    }
    else {
        return fabsf((floorf(o) - o)/d);
    }
}

/* max reach, counted in edge-crossings. for spherical reach, the results need to be
 * further pruned -- this allows ~2 blocks in the worst case diagonals, and 6 in the
 * best cases, where only one axis is traversed.
 */
#define MAX_PLAYER_REACH 6


void
ship_space::raycast(float ox, float oy, float oz, float dx, float dy, float dz, raycast_info *rc)
{
    /* implementation of the algorithm described in
     * http://www.cse.yorku.ca/~amana/research/grid.pdf
     */

    assert(rc);
    rc->hit = false;

    /* if less than 0 we need to subtract one
     * as float truncation will bias
     * towards 0
     */
    int x = (int)(ox < 0 ? ox - 1: ox);
    int y = (int)(oy < 0 ? oy - 1: oy);
    int z = (int)(oz < 0 ? oz - 1: oz);

    int nx = 0;
    int ny = 0;
    int nz = 0;

    block *bl = 0;

    bl = this->get_block(x,y,z);
    rc->inside = bl ? bl->type != block_empty : 0;

    int stepX = dx > 0 ? 1 : -1;
    int stepY = dy > 0 ? 1 : -1;
    int stepZ = dz > 0 ? 1 : -1;

    float tDeltaX = fabsf(1/dx);
    float tDeltaY = fabsf(1/dy);
    float tDeltaZ = fabsf(1/dz);

    float tMaxX = max_along_axis(ox, dx);
    float tMaxY = max_along_axis(oy, dy);
    float tMaxZ = max_along_axis(oz, dz);

    for (int i = 0; i < MAX_PLAYER_REACH; ++i) {
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                x += stepX;
                tMaxX += tDeltaX;
                nx = -stepX;
                ny = 0;
                nz = 0;
            }
            else {
                z += stepZ;
                tMaxZ += tDeltaZ;
                nx = 0;
                ny = 0;
                nz = -stepZ;
            }
        }
        else {
            if (tMaxY < tMaxZ) {
                y += stepY;
                tMaxY += tDeltaY;
                nx = 0;
                ny = -stepY;
                nz = 0;
            }
            else {
                z += stepZ;
                tMaxZ += tDeltaZ;
                nx = 0;
                ny = 0;
                nz = -stepZ;
            }
        }

        bl = this->get_block(x, y, z);
        if (!bl && !rc->inside){
            /* if there is no block then we are outside the grid
             * we still want to keep stepping until we either
             * hit a block within the grid or exceed our maximum
             * reach
             */
            continue;
        }

        if (rc->inside ^ (bl && bl->type != block_empty)) {
            rc->hit = true;
            rc->x = x;
            rc->y = y;
            rc->z = z;
            rc->block = bl;
            rc->nx = nx;
            rc->ny = ny;
            rc->nz = nz;
            rc->px = x + nx;
            rc->py = y + ny;
            rc->pz = z + nz;
            return;
        }
    }
}

/* ensure that the specified block_{x,y,z} can be fetched with a get_block
 *
 * this will instantiate a new containing chunk if necessary
 *
 * this will not instantiate or modify any other chunks
 */
void
ship_space::ensure_block(int block_x, int block_y, int block_z)
{
    int chunk_x, chunk_y, chunk_z;

    split_coord(block_x, NULL, &chunk_x);
    split_coord(block_y, NULL, &chunk_y);
    split_coord(block_z, NULL, &chunk_z);

    /* guarantee we have the size we need */
    this->ensure_chunk(chunk_x, chunk_y, chunk_z);
}

/* ensure that the specified chunk exists
 *
 * this will instantiate a new chunk if necessary
 *
 * this will not instantiate or modify any other chunks
 */
void
ship_space::ensure_chunk(int chunk_x, int chunk_y, int chunk_z)
{
    glm::ivec3 v(chunk_x, chunk_y, chunk_z);
    /* automatically creates the entry if not present */
    auto &ch = this->chunks[v];
    if (!ch) {
        ch = new chunk();
        this->_maintain_bounds(chunk_x, chunk_y, chunk_z);
    }
}

/* internal method which updated {min,max}_{x,y,z}
 * if the {x,y,z}_seen values are lower/higher
 */
void
ship_space::_maintain_bounds(int x_seen, int y_seen, int z_seen)
{
    this->min_x = std::min(min_x, x_seen);
    this->min_y = std::min(min_y, y_seen);
    this->min_z = std::min(min_z, z_seen);

    this->max_x = std::max(max_x, x_seen);
    this->max_y = std::max(max_y, y_seen);
    this->max_z = std::max(max_z, z_seen);
}

topo_info *
topo_find(topo_info *p)
{
    /* compress path */
    if (p->p != p) {
        p->p = topo_find(p->p);
    }

    return p->p;
}

/* helper to unify subtrees */
static topo_info *
topo_unite(topo_info *from, topo_info *to)
{
    from = topo_find(from);
    to = topo_find(to);

    /* already in same subtee? */
    if (from == to) return from;

    if (from->rank < to->rank) {
        from->p = to;
        return to;
    } else if (from->rank > to->rank) {
        to->p = from;
        return from;
    } else {
        /* merging two rank-r subtrees produces a rank-r+1 subtree. */
        to->p = from;
        from->rank++;
        return from;
    }
}

void
ship_space::update_topology_for_remove_surface(int x, int y, int z, int px, int py, int pz, int face)
{
    if (topo_dirty) {
        /* if we already dirtied it, we cant assume anything. just take the rebuild */
        return;
    }

    topo_info *t = topo_find(get_topo_info(x, y, z));
    topo_info *u = topo_find(get_topo_info(px, py, pz));

    num_fast_unifys++;

    if (t == u) {
        /* we're not really unifying */
        return;
    }

    topo_info *v = topo_unite(t, u);
    /* track sizing */
    v->size = t->size + u->size;
}

static bool
exists_alt_path(int x, int y, int z, block *a, block *b, ship_space *ship, int face)
{
    block *c;

    if (face != surface_xp) {
        c = ship->get_block(x+1, y, z);
        if (a->surfs[surface_xp] != surface_wall && b->surfs[surface_xp] != surface_wall &&
                (!c || c->surfs[face] != surface_wall))
            return true;
        c = ship->get_block(x-1, y, z);
        if (a->surfs[surface_xm] != surface_wall && b->surfs[surface_xm] != surface_wall &&
                (!c || c->surfs[face] != surface_wall))
            return true;
    }

    if (face != surface_yp) {
        c = ship->get_block(x, y+1, z);
        if (a->surfs[surface_yp] != surface_wall && b->surfs[surface_yp] != surface_wall &&
                (!c || c->surfs[face] != surface_wall))
            return true;
        c = ship->get_block(x, y-1, z);
        if (a->surfs[surface_ym] != surface_wall && b->surfs[surface_ym] != surface_wall &&
                (!c || c->surfs[face] != surface_wall))
            return true;
    }

    if (face != surface_zp) {
        c = ship->get_block(x, y, z+1);
        if (a->surfs[surface_zp] != surface_wall && b->surfs[surface_zp] != surface_wall &&
                (!c || c->surfs[face] != surface_wall))
            return true;
        c = ship->get_block(x, y, z-1);
        if (a->surfs[surface_zm] != surface_wall && b->surfs[surface_zm] != surface_wall &&
                (!c || c->surfs[face] != surface_wall))
            return true;
    }

    return false;
}

void
ship_space::update_topology_for_add_surface(int x, int y, int z, int px, int py, int pz, int face)
{
    if (topo_dirty) {
        /* if we already dirtied it, we cant assume anything. just take the rebuild */
        return;
    }

    /* can this surface even split (does it block atmo?) */
    if (get_block(x, y, z)->surfs[face] != surface_wall)
        return;

    /* collapse an obvious symmetry */
    if (face & 1) {
        /* symmetry */
        std::swap(x, px);
        std::swap(y, py);
        std::swap(z, pz);
        face ^= 1;
    }

    /* try to quickly prove that we don't divide space */
    if (exists_alt_path(x, y, z, get_block(x, y, z), get_block(px, py, pz), this, face)) {
        num_fast_nosplits++;
    }
    else {
        topo_dirty = true;
    }
}

static glm::ivec3 dirs[] = {
    glm::ivec3(1, 0, 0),
    glm::ivec3(-1, 0, 0),
    glm::ivec3(0, 1, 0),
    glm::ivec3(0, -1, 0),
    glm::ivec3(0, 0, 1),
    glm::ivec3(0, 0, -1),
};

/* rebuild the ship topology. this is generally not the optimal thing -
 * we can dynamically rebuild parts of the topology cheaper based on
 * knowing the change that was made.
 */
void
ship_space::rebuild_topology()
{
    if (!topo_dirty)
        return;
    topo_dirty = false;

    num_full_rebuilds++;

    /* 1/ initially, every block is its own subtree */
    for (auto it = chunks.begin(); it != chunks.end(); it++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int x = 0; x < CHUNK_SIZE; x++) {
                    topo_info *t = it->second->topo.get(x, y, z);
                    t->p = t;
                    t->rank = 0;
                    t->size = 0;
                }
            }
        }
    }

    this->outside_topo_info.p = &this->outside_topo_info;
    this->outside_topo_info.rank = 0;
    this->outside_topo_info.size = 0;

    /* 2/ combine across air-permeable interfaces */
    for (auto it = chunks.begin(); it != chunks.end(); it++) {
        for (int z = 1; z < CHUNK_SIZE - 1; z++) {
            for (int y = 1; y < CHUNK_SIZE - 1; y++) {
                for (int x = 1; x < CHUNK_SIZE - 1; x++) {
                    block *bl = it->second->blocks.get(x, y, z);

                    /* TODO: proper air-permeability query -- soon it will be not just walls! */
                    for (int i = 0; i < 6; i++) {
                        if (bl->surfs[i] != surface_wall) {
                            glm::ivec3 offset = dirs[i];
                            topo_unite(it->second->topo.get(x, y, z),
                                       it->second->topo.get(x + offset.x, y + offset.y, z + offset.z));
                        }
                    }
                }
            }
        }

        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                block *bl = it->second->blocks.get(0, y, z);
                topo_info *to = it->second->topo.get(0, y, z);

                /* TODO: proper air-permeability query -- soon it will be not just walls! */
                for (int i = 0; i < 6; i++) {
                    if (bl->surfs[i] != surface_wall) {
                        glm::ivec3 offset = dirs[i];
                        topo_unite(to,
                              get_topo_info(CHUNK_SIZE * it->first.x + 0 + offset.x,
                                            CHUNK_SIZE * it->first.y + y + offset.y,
                                            CHUNK_SIZE * it->first.z + z + offset.z));
                    }
                }

                bl = it->second->blocks.get(CHUNK_SIZE - 1, y, z);
                to = it->second->topo.get(CHUNK_SIZE - 1, y, z);

                /* TODO: proper air-permeability query -- soon it will be not just walls! */
                for (int i = 0; i < 6; i++) {
                    if (bl->surfs[i] != surface_wall) {
                        glm::ivec3 offset = dirs[i];
                        topo_unite(to,
                              get_topo_info(CHUNK_SIZE * it->first.x + CHUNK_SIZE - 1 + offset.x,
                                            CHUNK_SIZE * it->first.y + y + offset.y,
                                            CHUNK_SIZE * it->first.z + z + offset.z));
                    }
                }

                bl = it->second->blocks.get(y, 0, z);
                to = it->second->topo.get(y, 0, z);

                /* TODO: proper air-permeability query -- soon it will be not just walls! */
                for (int i = 0; i < 6; i++) {
                    if (bl->surfs[i] != surface_wall) {
                        glm::ivec3 offset = dirs[i];
                        topo_unite(to,
                              get_topo_info(CHUNK_SIZE * it->first.x + y + offset.x,
                                            CHUNK_SIZE * it->first.y + 0 + offset.y,
                                            CHUNK_SIZE * it->first.z + z + offset.z));
                    }
                }

                bl = it->second->blocks.get(y, CHUNK_SIZE - 1, z);
                to = it->second->topo.get(y, CHUNK_SIZE - 1, z);

                /* TODO: proper air-permeability query -- soon it will be not just walls! */
                for (int i = 0; i < 6; i++) {
                    if (bl->surfs[i] != surface_wall) {
                        glm::ivec3 offset = dirs[i];
                        topo_unite(to,
                              get_topo_info(CHUNK_SIZE * it->first.x + y + offset.x,
                                            CHUNK_SIZE * it->first.y + CHUNK_SIZE - 1 + offset.y,
                                            CHUNK_SIZE * it->first.z + z + offset.z));
                    }
                }

                bl = it->second->blocks.get(y, z, 0);
                to = it->second->topo.get(y, z, 0);

                /* TODO: proper air-permeability query -- soon it will be not just walls! */
                for (int i = 0; i < 6; i++) {
                    if (bl->surfs[i] != surface_wall) {
                        glm::ivec3 offset = dirs[i];
                        topo_unite(to,
                              get_topo_info(CHUNK_SIZE * it->first.x + y + offset.x,
                                            CHUNK_SIZE * it->first.y + z + offset.y,
                                            CHUNK_SIZE * it->first.z + 0 + offset.z));
                    }
                }

                bl = it->second->blocks.get(y, z, CHUNK_SIZE - 1);
                to = it->second->topo.get(y, z, CHUNK_SIZE - 1);

                /* TODO: proper air-permeability query -- soon it will be not just walls! */
                for (int i = 0; i < 6; i++) {
                    if (bl->surfs[i] != surface_wall) {
                        glm::ivec3 offset = dirs[i];
                        topo_unite(to,
                              get_topo_info(CHUNK_SIZE * it->first.x + y + offset.x,
                                            CHUNK_SIZE * it->first.y + z + offset.y,
                                            CHUNK_SIZE * it->first.z + CHUNK_SIZE - 1 + offset.z));
                    }
                }
            }
        }
    }

    /* 3/ finalize, and accumulate sizes */
    for (auto it = chunks.begin(); it != chunks.end(); it++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int x = 0; x < CHUNK_SIZE; x++) {
                    topo_info *t = topo_find(it->second->topo.get(x, y, z));
                    t->size++;
                }
            }
        }
    }
}
