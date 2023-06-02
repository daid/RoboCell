#include "conf.h"


static std::shared_ptr<sp::MeshData> createQuad(sp::Vector2f size, float z)
{
    size *= 0.5f;
    
    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices{0,1,2,2,1,3};
    vertices.reserve(4);
    
    vertices.emplace_back(sp::Vector3f(-size.x, -size.y, z), sp::Vector2f(0, 1));
    vertices.emplace_back(sp::Vector3f( size.x, -size.y, z), sp::Vector2f(1, 1));
    vertices.emplace_back(sp::Vector3f(-size.x,  size.y, z), sp::Vector2f(0, 0));
    vertices.emplace_back(sp::Vector3f( size.x,  size.y, z), sp::Vector2f(1, 0));
    
    return sp::MeshData::create(std::move(vertices), std::move(indices));
}

std::shared_ptr<sp::MeshData> binder_mesh = createQuad({1.5,0.8}, -0.06f);
std::shared_ptr<sp::MeshData> cell_mesh = createQuad({1,1}, 0.05f);
std::shared_ptr<sp::MeshData> cell_spawn_mesh = createQuad({1,1}, -0.06);
std::shared_ptr<sp::MeshData> cell_bond_mesh = createQuad({1.5,0.8}, 0.025f);
std::shared_ptr<sp::MeshData> goal_mesh = createQuad({1,1}, -0.08f);
std::shared_ptr<sp::MeshData> goal_bond_mesh = createQuad({1.5, 0.8}, -0.09f);
std::shared_ptr<sp::MeshData> action_mesh = createQuad({1,1}, -0.07f);
std::shared_ptr<sp::MeshData> path_mesh = createQuad({2,2}, 0.0f);
