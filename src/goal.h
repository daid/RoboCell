#pragma once

#include "direction.h"
#include <sp2/scene/node.h>

class Goal;
class GoalNode : public sp::Node
{
public:
    GoalNode(sp::P<sp::Node> parent, sp::Vector2i position);

    sp::P<GoalNode> addBond(Direction direction);
    bool checkGoal();

    sp::Vector2i grid_position;
    sp::P<Goal> base;
    struct Bond {
        sp::P<GoalNode> node;
        Direction d;
    };
    std::vector<Bond> bonds;
};

class Goal : public GoalNode
{
public:
    Goal(sp::P<sp::Node> parent, sp::Vector2i position);

    void onFixedUpdate() override;

    int done = 0;
    int required = 30;
    sp::PList<GoalNode> all;
};