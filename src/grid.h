#pragma once

#include "direction.h"

#include <sp2/container/infinigrid.h>
#include <sp2/scene/node.h>

enum class GridAction
{
    None,
    TurnToUpRight,
    TurnToRight,
    TurnToDownRight,
    TurnToDownLeft,
    TurnToLeft,
    TurnToUpLeft,
    PickupDrop,
    Sync,
    Bind,
    FlipFlopA,
    FlipFlopB,
    FlopFlipA,
    FlopFlipB,
};

extern sp::InfiniGrid<GridAction> action_grid;
extern sp::InfiniGrid<sp::P<sp::Node>> action_node_grid;
extern sp::InfiniGrid<bool> footprint_grid;

sp::Vector2d gridToPos(sp::Vector2i v);
sp::Vector2i posToGrid(sp::Vector2d v);
sp::Vector2i gridOffset(Direction d);

void setGridAction(sp::Vector2i p, GridAction action);
void saveGrid(const sp::string& filename);
void clearGrid();
bool loadGrid(const sp::string& filename);
