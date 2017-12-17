#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include "soloud.h"
#include "soloud_audiosource.h"

#include "mesh.h"
#include "block.h"
#include "textureset.h"

class asset_manager
{
    std::unordered_map<std::string, mesh_data> meshes;
    std::array<std::array<std::string, face_count>, 256> surf_to_mesh;

    std::unordered_map<std::string, unsigned> world_texture_to_index;

    texture_set *world_textures{ nullptr };
    texture_set *render_textures{nullptr};
    std::unordered_map<std::string, texture_set *> skyboxes {};
    std::unordered_map<std::string, SoLoud::AudioSource *> sounds {};
     
    void load_asset_manifest(char const *filename);

public:
    asset_manager();
    ~asset_manager() = default;

    void load_assets();

    unsigned get_world_texture_index(const std::string &) const;
    mesh_data & get_mesh(const std::string &);

    mesh_data & get_surface_mesh(unsigned surface_index, unsigned surface_type);

    SoLoud::AudioSource * get_sound(const std::string &);

    void bind_world_textures(int i);
    void bind_render_textures(int i);

    void bind_skybox(const std::string &skybox, int i);
};
