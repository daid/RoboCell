#include "binder.h"
#include "grid.h"
#include "conf.h"
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>


Binder::Binder(sp::P<sp::Node> parent, sp::Vector2i position, Direction direction)
: sp::Node(parent)
{
    render_data.type = sp::RenderData::Type::Transparent;
    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = binder_mesh;
    render_data.texture = sp::texture_manager.get("binder.png");
    render_data.color = sp::Color(1.0, 1.0, 1.0, 0.5);
    auto p = (gridToPos(position) + gridToPos(position + gridOffset(direction))) * 0.5;
    setPosition(p);
    setRotation(toRotation(direction));
    grid_position = position;
    this->direction = direction;

    all.add(this);
}