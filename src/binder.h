#pragma once

#include <sp2/scene/node.h>
#include "direction.h"


class Binder : public sp::Node
{
public:
    Binder(sp::P<sp::Node> parent, sp::Vector2i position, Direction direction);

    sp::Vector2i grid_position;
    Direction direction;

    static inline sp::PList<Binder> all;
};
