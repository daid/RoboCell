#include "goal.h"
#include "grid.h"
#include "pawn.h"
#include "conf.h"
#include <sp2/engine.h>
#include <sp2/graphics/mesh/obj.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/audio/sound.h>


GoalNode::GoalNode(sp::P<sp::Node> parent, sp::Vector2i position)
: sp::Node(parent)
{
    render_data.type = sp::RenderData::Type::Normal;
    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = goal_mesh;
    render_data.texture = sp::texture_manager.get("cell/blood.png");
    render_data.color = sp::Color(0.5, 0.5, 0.5);

    grid_position = position;
    setPosition(gridToPos(position));
}

sp::P<GoalNode> GoalNode::addBond(Direction direction)
{
    auto visual = new sp::Node(this);
    visual->render_data.type = sp::RenderData::Type::Normal;
    visual->render_data.shader = sp::Shader::get("internal:basic.shader");
    visual->render_data.mesh = goal_bond_mesh;
    visual->render_data.texture = sp::texture_manager.get("binder.png");
    visual->render_data.color = sp::Color(1.0, 1.0, 1.0, 0.5);
    visual->setPosition(sp::Vector2d(std::sqrt(3) * 0.5, 0).rotate(toRotation(direction)));
    visual->setRotation(toRotation(direction));

    auto target = posToGrid(getPosition2D()) + gridOffset(direction);
    for(auto gn : base->all) {
        if (target == posToGrid(gn->getPosition2D())) {
            gn->bonds.push_back({this, direction + 3});
            bonds.push_back({gn, direction});
            return gn;
        }
    }
    auto gn = new GoalNode(getParent(), target);
    gn->base = base;
    base->all.add(gn);
    gn->bonds.push_back({this, direction + 3});
    bonds.push_back({gn, direction});
    return gn;
}

bool GoalNode::checkGoal()
{
    auto p = Pawn::grid.get(grid_position);
    if (!p) return false;
    if (p->render_data.texture != render_data.texture)
        return false;
    if (p->bonds.size() != bonds.size()) return false;
    int my_mask = 0;
    for(auto b : bonds)
        my_mask |= 1 << static_cast<int>(b.d);
    int other_mask = 0;
    for(auto b : p->bonds)
        other_mask |= 1 << static_cast<int>(b.direction);
    return my_mask == other_mask;
}

Goal::Goal(sp::P<sp::Node> parent, sp::Vector2i position)
: GoalNode(parent, position)
{
    base = this;
    all.add(this);
}

void Goal::onFixedUpdate()
{
    auto p = Pawn::grid.get(grid_position);
    if (!p) return;
    for(auto g : all)
        if (!g->checkGoal())
            return;
    for(auto b : p->allBonded())
        delete b;
    done = std::min(done + 1, required);

    if (sp::Engine::getInstance()->getGameSpeed() < 16.0f)
        sp::Engine::getInstance()->setGameSpeed(sp::Engine::getInstance()->getGameSpeed() * 2.0f);

    sp::audio::Sound::play("sfx/goal.wav");
}
