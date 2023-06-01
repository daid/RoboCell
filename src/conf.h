#pragma once

#include <sp2/graphics/meshdata.h>


static constexpr float grid_z = -0.1;
static constexpr float spawn_z = -0.07;
static constexpr float path_z = -0.05;
static constexpr float error_z = 0.1;

extern std::shared_ptr<sp::MeshData> binder_mesh;
extern std::shared_ptr<sp::MeshData> cell_mesh;
extern std::shared_ptr<sp::MeshData> cell_spawn_mesh;
extern std::shared_ptr<sp::MeshData> cell_bond_mesh;
extern std::shared_ptr<sp::MeshData> goal_mesh;
extern std::shared_ptr<sp::MeshData> goal_bond_mesh;
extern std::shared_ptr<sp::MeshData> action_mesh;