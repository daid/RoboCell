#pragma once

#include "direction.h"
#include <sp2/scene/node.h>
#include <optional>

class Pawn;
class Robo : public sp::Node
{
public:
    enum class Action {
        MoveForward,
        TurnLeft,
        TurnRight,
        PickupDrop,
        SyncMove,
        SyncWait,
        Bind,
    };

    Robo(sp::P<sp::Node> parent);
    void onFixedUpdate() override;

    std::optional<sp::Vector2d> checkCarryCollision(sp::Vector2d world_position);

    static constexpr int steps_per_action = 30;
    int step = steps_per_action / 2;
    sp::Vector2i grid_position;
    Direction direction = Direction::Right;
    Action current_action = Action::MoveForward;
    sp::P<Pawn> carry;

    static inline sp::PList<Robo> all;
};
