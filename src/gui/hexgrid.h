#pragma once

#include <sp2/graphics/gui/layout/layout.h>

class HexGridLayout : public sp::gui::Layout
{
public:
    virtual void update(sp::P<sp::gui::Widget> container, sp::Rect2d rect) override;
};
