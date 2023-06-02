#include "robo.h"
#include "grid.h"
#include "pawn.h"
#include "binder.h"
#include "mainscene.h"

#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/tween.h>
#include <unordered_set>


Robo::Robo(sp::P<sp::Node> parent) : sp::Node(parent) {
    render_data.type = sp::RenderData::Type::Normal;
    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = sp::MeshData::createQuad({2.0f, 2.0f});
    render_data.texture = sp::texture_manager.get("robo.png");
    render_data.color = sp::Color(1, 0.3, 0.3);
    all.add(this);
}

void Robo::onFixedUpdate()
{
    if (step == steps_per_action) {
        step = 0;
        switch(current_action) {
        case Action::MoveForward:
        case Action::PickupDrop:
        case Action::SyncMove:
        case Action::Bind:
            grid_position += gridOffset(direction);
            break;
        case Action::TurnLeft:
            direction = direction - 1;
            grid_position += gridOffset(direction);
            if (carry)
                carry->turn(-1);
            break;
        case Action::TurnRight: 
            direction = direction + 1;
            grid_position += gridOffset(direction);
            if (carry)
                carry->turn(1);
            break;
        case Action::SyncWait:
            break;
        }

        auto previous_action = current_action;
        current_action = Action::MoveForward;
        switch(action_grid.get(grid_position))
        {
        case GridAction::None: break;
        case GridAction::TurnToUpRight:
            if (direction == Direction::Right) current_action = Action::TurnLeft;
            if (direction == Direction::UpLeft) current_action = Action::TurnRight;
            break;
        case GridAction::TurnToRight:
            if (direction == Direction::DownRight) current_action = Action::TurnLeft;
            if (direction == Direction::UpRight) current_action = Action::TurnRight;
            break;
        case GridAction::TurnToDownRight:
            if (direction == Direction::DownLeft) current_action = Action::TurnLeft;
            if (direction == Direction::Right) current_action = Action::TurnRight;
            break;
        case GridAction::TurnToDownLeft:
            if (direction == Direction::Left) current_action = Action::TurnLeft;
            if (direction == Direction::DownRight) current_action = Action::TurnRight;
            break;
        case GridAction::TurnToLeft:
            if (direction == Direction::UpLeft) current_action = Action::TurnLeft;
            if (direction == Direction::DownLeft) current_action = Action::TurnRight;
            break;
        case GridAction::TurnToUpLeft:
            if (direction == Direction::UpRight) current_action = Action::TurnLeft;
            if (direction == Direction::Left) current_action = Action::TurnRight;
            break;
        case GridAction::PickupDrop: current_action = Action::PickupDrop; break;
        case GridAction::Sync: current_action = previous_action == Action::SyncWait ? Action::SyncWait : Action::SyncMove; break;
        case GridAction::Bind: current_action = Action::Bind; break;
        case GridAction::FlipFlopA:
            current_action = Action::TurnLeft;
            setGridAction(grid_position, GridAction::FlipFlopB);
            break;
        case GridAction::FlipFlopB:
            current_action = Action::TurnRight;
            setGridAction(grid_position, GridAction::FlipFlopA);
            break;
        case GridAction::FlopFlipA:
            current_action = Action::TurnLeft;
            setGridAction(grid_position, GridAction::FlopFlipB);
            break;
        case GridAction::FlopFlipB:
            current_action = Action::TurnRight;
            setGridAction(grid_position, GridAction::FlopFlipA);
            break;
        }
    }
    if (step == steps_per_action / 2) {
        if (carry && current_action != Action::TurnLeft && current_action != Action::TurnRight) {
            for(auto c : carry->allBonded()) {
                for(auto b : Binder::all) {
                    if (b->grid_position == c->grid_position) {
                        c->makeBond(b->direction);
                        carry->pickup();
                    }
                    if (b->grid_position + gridOffset(b->direction) == c->grid_position) {
                        c->makeBond(b->direction + 3);
                        carry->pickup();
                    }
                }
            }
        }
        switch(current_action) {
        case Action::MoveForward:
        case Action::TurnLeft:
        case Action::TurnRight:
        case Action::SyncWait:
            break;
        case Action::SyncMove: current_action = Action::SyncWait; break;
        case Action::Bind:
            if (carry) {
                for(auto d : allDirections)
                    carry->makeBond(d);
                carry->pickup();
            } else if (auto p = Pawn::grid.get(grid_position)) {
                for(auto d : allDirections)
                    p->makeBond(d);
            }
            break;
        case Action::PickupDrop:
            if (carry) {
                carry->drop();
                carry = nullptr;
                render_data.texture = sp::texture_manager.get("robo.png");
            } else {
                carry = Pawn::grid.get(grid_position);
                if (carry) {
                    carry->pickup();
                    render_data.texture = sp::texture_manager.get("robo2.png");
                }
            }
            
            break;
        }
    }
    if (step == steps_per_action / 2 + 1 && current_action == Action::SyncWait) {
        bool all_wait = true;
        for(auto r : all) {
            if (r->current_action != Action::SyncWait) all_wait = false;
        }
        if (all_wait) {
            for(auto r : all)
                r->current_action = Action::SyncMove;
        }
    }

    double additional_rotation = 0.0;
    switch(current_action) {
    case Action::MoveForward:
    case Action::PickupDrop: 
    case Action::SyncMove:
    case Action::Bind: {
        sp::Vector2d prev_position = gridToPos(grid_position - gridOffset(direction));
        sp::Vector2d current_position = gridToPos(grid_position);
        sp::Vector2d next_position = gridToPos(grid_position + gridOffset(direction));
        prev_position = (prev_position + current_position) * 0.5;
        next_position = (next_position + current_position) * 0.5;
        setPosition(sp::Tween<sp::Vector2d>::linear(step, 0, steps_per_action, prev_position, next_position));
        }break;
    case Action::TurnLeft: {
        sp::Vector2d center_position = gridToPos(grid_position + gridOffset(direction - 2));
        double start_rotation = toRotation(direction);
        double end_rotation = toRotation(direction - 1);
        end_rotation = start_rotation + sp::angleDifference(start_rotation, end_rotation);
        float r = sp::Tween<double>::linear(step, 0, steps_per_action, start_rotation, end_rotation);
        setPosition(center_position + sp::Vector2d(0, -1.5).rotate(r));
        setRotation(r);
        additional_rotation = r - start_rotation;
        }break;
    case Action::TurnRight: {
        sp::Vector2d center_position = gridToPos(grid_position + gridOffset(direction + 2));
        double start_rotation = toRotation(direction);
        double end_rotation = toRotation(direction + 1);
        end_rotation = start_rotation + sp::angleDifference(start_rotation, end_rotation);
        float r = sp::Tween<double>::linear(step, 0, steps_per_action, start_rotation, end_rotation);
        setPosition(center_position + sp::Vector2d(0, 1.5).rotate(r));
        setRotation(r);
        additional_rotation = r - start_rotation;
        }break;
    case Action::SyncWait:
        setPosition(gridToPos(grid_position));
        break;
    }
    if (carry) {
        carry->moveTo(grid_position, getPosition2D(), additional_rotation);

        for(auto p : carry->allBonded()) {
            if (auto error_pos = Pawn::checkCollision(p->getPosition2D())) {
                sp::P<Scene> scene = sp::Scene::get("MAIN");
                scene->error(error_pos.value());
            }
            for(auto r : all) {
                if (r == this || !r->carry) continue;
                for(auto p2 : r->carry->allBonded()) {
                    if ((p->getPosition2D() - p2->getPosition2D()).length() <= 1.0) {
                        sp::P<Scene> scene = sp::Scene::get("MAIN");
                        scene->error((p->getPosition2D() + p2->getPosition2D()) * 0.5);
                    }
                }
            }
        }
    }

    step += 1;
}

std::optional<sp::Vector2d> Robo::checkCarryCollision(sp::Vector2d world_position)
{
    if (!carry) return {};
    for(auto p : carry->allBonded()) {
        if ((p->getPosition2D() - world_position).length() <= 1.0)
            return (p->getPosition2D() + world_position) * 0.5;
    }
    return {};
}