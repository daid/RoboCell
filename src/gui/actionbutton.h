#pragma once

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace io { class Keybinding; }
namespace gui { class Label; }
}

class Icon;
class ActionButton : public sp::gui::Widget
{
public:
    ActionButton(sp::P<sp::gui::Widget> parent);

    void setLabel(const sp::string& label);
    virtual void setAttribute(const sp::string& key, const sp::string& value) override;

    virtual void onUpdate(float delta) override;
    virtual void updateRenderData() override;
    virtual bool onPointerDown(sp::io::Pointer::Button button, sp::Vector2d position, int id) override;
    virtual void onPointerUp(sp::Vector2d position, int id) override;
private:
    bool keybinding_down = false;
    sp::P<sp::io::Keybinding> keybinding;
    sp::P<Icon> icon;
    sp::P<sp::gui::Label> label;
};
