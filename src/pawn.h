#pragma once

#include "direction.h"
#include <sp2/scene/node.h>
#include <sp2/container/infinigrid.h>
#include <optional>


class PawnSpawn : public sp::Node
{
public:
    PawnSpawn(sp::P<sp::Node> parent, sp::Vector2i position);

    sp::Vector2i grid_position;
    void onFixedUpdate() override;
};

class Pawn : public sp::Node
{
public:
    Pawn(sp::P<sp::Node> parent, sp::Vector2i position);
    ~Pawn();

    void drop();
    void pickup();
    void makeBond(Direction direction);
    void moveTo(sp::Vector2i grid_position, sp::Vector2d world_position, double additional_rotation);
    void turn(int offset);
    std::vector<Pawn*> allBonded();

    sp::Vector2i grid_position;
    Direction direction = Direction::Right;
    struct Bond {
        Direction direction;
        sp::P<Pawn> other;
        sp::P<sp::Node> visual;
    };
    std::vector<Bond> bonds;

    static std::optional<sp::Vector2d> checkCollision(sp::Vector2d world_position);
    static sp::InfiniGrid<sp::P<Pawn>> grid;
};
