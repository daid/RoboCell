#include "pawn.h"
#include "grid.h"
#include "robo.h"
#include "conf.h"
#include <sp2/graphics/mesh/obj.h>
#include <sp2/graphics/textureManager.h>
#include <unordered_set>


sp::InfiniGrid<sp::P<Pawn>> Pawn::grid{nullptr};

PawnSpawn::PawnSpawn(sp::P<sp::Node> parent, sp::Vector2i position)
: sp::Node(parent)
{
    render_data.type = sp::RenderData::Type::Transparent;
    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = cell_spawn_mesh;
    render_data.texture = sp::texture_manager.get("cell/blood.png");
    render_data.color = sp::Color(1.0, 1.0, 1.0, 0.5);
    setPosition(gridToPos(position));
    grid_position = position;
}

void PawnSpawn::onFixedUpdate()
{
    if (Pawn::grid.get(grid_position) != nullptr) return;
    for(auto r : Robo::all) {
        if (r->checkCarryCollision(gridToPos(grid_position)))
            return;
    }
    auto p = new Pawn(getParent(), grid_position);
    p->render_data.texture = render_data.texture;
}

Pawn::Pawn(sp::P<sp::Node> parent, sp::Vector2i position)
: sp::Node(parent)
{
    render_data.type = sp::RenderData::Type::Normal;
    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = cell_mesh;
    render_data.texture = sp::texture_manager.get("cell/blood.png");

    grid_position = position;
    setPosition(gridToPos(position));

    grid.set(position, this);  
}

Pawn::~Pawn()
{
    grid.clear(grid_position);
}

static void recursiveDrop(std::unordered_set<Pawn*>& done, Pawn* p)
{
    done.insert(p);
    Pawn::grid.set(p->grid_position, p);
    for(const auto& b : p->bonds) {
        if (done.find(*b.other) != done.end())
            continue;
        recursiveDrop(done, *b.other);
    }
}

static void recursivePickup(std::unordered_set<Pawn*>& done, Pawn* p)
{
    done.insert(p);
    Pawn::grid.clear(p->grid_position);
    for(const auto& b : p->bonds) {
        if (done.find(*b.other) != done.end())
            continue;
        recursivePickup(done, *b.other);
    }
}

void Pawn::drop()
{
    std::unordered_set<Pawn*> done;
    recursiveDrop(done, this);
}

void Pawn::pickup()
{
    std::unordered_set<Pawn*> done;
    recursivePickup(done, this);
}

std::optional<sp::Vector2d> Pawn::checkCollision(sp::Vector2d world_position) {
    auto grid_position = posToGrid(world_position);
    if (Pawn::grid.get(grid_position) && (gridToPos(grid_position) - world_position).length() <= 1.0) {
        return (gridToPos(grid_position) + world_position) * 0.5;
    }
    for(auto d : allDirections) {
        if (Pawn::grid.get(grid_position + gridOffset(d)) && (gridToPos(grid_position + gridOffset(d)) - world_position).length() <= 1.0)
            return (gridToPos(grid_position + gridOffset(d)) + world_position) * 0.5;
    }
    return {};
}

static void recursiveUpdate(std::unordered_set<Pawn*>& done, Pawn* p, sp::Vector2i g, sp::Vector2d w, double additional_rotation)
{
    done.insert(p);
    p->setPosition(w);
    p->setRotation(toRotation(p->direction) + additional_rotation);
    p->grid_position = g;
    for(const auto& b : p->bonds) {
        if (done.find(*b.other) != done.end())
            continue;
        auto other_w = w + sp::Vector2d(std::sqrt(3), 0).rotate(toRotation(b.direction) + additional_rotation);
        auto other_g = g + gridOffset(b.direction);
        recursiveUpdate(done, *b.other, other_g, other_w, additional_rotation);
    }
}

void Pawn::moveTo(sp::Vector2i grid_position, sp::Vector2d world_position, double additional_rotation)
{
    std::unordered_set<Pawn*> done;
    recursiveUpdate(done, this, grid_position, world_position, additional_rotation);
}

static void recursiveTurn(std::unordered_set<Pawn*>& done, Pawn* p, int offset)
{
    done.insert(p);
    p->direction = p->direction + offset;
    for(auto& b : p->bonds) {
        b.direction = b.direction + offset;
        if (b.visual->getParent() == p) {
            float r = toRotation(b.direction) - toRotation(p->direction);
            b.visual->setPosition(sp::Vector2d(std::sqrt(3) * 0.5, 0).rotate(r));
            b.visual->setRotation(r);
        }
        if (done.find(*b.other) != done.end())
            continue;
        recursiveTurn(done, *b.other, offset);
    }
}

void Pawn::turn(int offset)
{
    std::unordered_set<Pawn*> done;
    recursiveTurn(done, this, offset);
}

std::vector<Pawn*> Pawn::allBonded()
{
    std::vector<Pawn*> result{{this}};
    std::unordered_set<Pawn*> done;
    std::vector<Pawn*> todo{{this}};
    done.insert(this);

    while(!todo.empty()) {
        auto p = todo.back();
        todo.pop_back();

        for(const auto& b : p->bonds) {
            if (done.find(*b.other) == done.end()) {
                done.insert(*b.other);
                result.push_back(*b.other);
                todo.push_back(*b.other);
            }
        }
    }
    return result;
}

void Pawn::makeBond(Direction direction) {
    for(auto b : bonds)
        if (b.direction == direction)
            return;
    auto other = grid.get(grid_position + gridOffset(direction));
    if (!other) return;
    auto visual = new sp::Node(this);
    visual->render_data.type = sp::RenderData::Type::Transparent;
    visual->render_data.shader = sp::Shader::get("internal:basic.shader");
    visual->render_data.mesh = cell_bond_mesh;
    visual->render_data.texture = sp::texture_manager.get("cell/bond.png");
    float r = toRotation(direction) - toRotation(this->direction);
    visual->setPosition(sp::Vector2d(std::sqrt(3) * 0.5, 0).rotate(r));
    visual->setRotation(r);
    bonds.push_back({direction, other, visual});
    other->bonds.push_back({direction + 3, this, visual});
}
