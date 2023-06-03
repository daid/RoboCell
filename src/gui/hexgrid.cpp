#include "hexgrid.h"
#include <sp2/graphics/gui/widget/widget.h>


SP_REGISTER_LAYOUT("hexgrid", HexGridLayout);

void HexGridLayout::update(sp::P<sp::gui::Widget> container, sp::Rect2d rect)
{
    sp::Vector2d cell_size;
    int y_max = 0;
    for(sp::P<sp::gui::Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        cell_size.x = std::max(w->layout.size.x, cell_size.x);
        cell_size.y = std::max(w->layout.size.x, cell_size.y);
        y_max = std::max(int(w->layout.position.y), y_max);
    }

    auto x_min = std::numeric_limits<double>::max();
    for(sp::P<sp::gui::Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        sp::Vector2i position{int(w->layout.position.x), y_max - int(w->layout.position.y)};
        
        sp::Vector2d cell_pos = {position.x * cell_size.x, position.y * cell_size.y * 0.8};
        cell_pos.x -= position.y * cell_size.x * 0.5;
        x_min = std::min(x_min, cell_pos.x);
    }

    for(sp::P<sp::gui::Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        sp::Vector2i position{int(w->layout.position.x), y_max - int(w->layout.position.y)};
        
        sp::Vector2d cell_pos = {position.x * cell_size.x, position.y * cell_size.y * 0.8};
        cell_pos.x -= position.y * cell_size.x * 0.5 + x_min;
        auto old_position = w->layout.position;
        w->layout.position.x = w->layout.position.y = 0.0;
        basicLayout(sp::Rect2d(cell_pos, cell_size), *w);
        w->layout.position = old_position;
    }
}
