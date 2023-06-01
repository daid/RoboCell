#include "actionbutton.h"
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>
#include <sp2/stringutil/convert.h>


SP_REGISTER_WIDGET("actionbutton", ActionButton);

class Icon : public sp::gui::Widget
{
public:
    Icon(sp::P<sp::gui::Widget> parent);
    
    virtual void setAttribute(const sp::string& key, const sp::string& value) override;
    
    virtual void updateRenderData() override;
private:
    sp::string texture;
    double rotation = 0.0;
};

Icon::Icon(sp::P<sp::gui::Widget> parent)
: sp::gui::Widget(parent)
{
    loadThemeStyle("image");
}

void Icon::setAttribute(const sp::string& key, const sp::string& value)
{
    if (key == "texture" || key == "image")
    {
        texture = value;
        markRenderDataOutdated();
    }
    else if (key == "color")
    {
        render_data.color = sp::stringutil::convert::toColor(value);
    }
    else if (key == "rotation")
    {
        rotation = sp::stringutil::convert::toFloat(value);
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Icon::updateRenderData()
{
    if (texture.length() > 0)
    {
        sp::MeshData::Vertices vertices;
        sp::MeshData::Indices indices{0,1,2, 2,1,3};
        vertices.reserve(4);

        auto c = getRenderSize() * 0.5;
        auto r = std::min(c.x, c.x);
        auto p0 = c + sp::Vector2d(-r, -r).rotate(rotation);
        auto p1 = c + sp::Vector2d( r, -r).rotate(rotation);
        auto p2 = c + sp::Vector2d(-r,  r).rotate(rotation);
        auto p3 = c + sp::Vector2d( r,  r).rotate(rotation);
        vertices.emplace_back(sp::Vector3f(p0.x, p0.y, 0.0f), sp::Vector2f(0, 1));
        vertices.emplace_back(sp::Vector3f(p1.x, p1.y, 0.0f), sp::Vector2f(1, 1));
        vertices.emplace_back(sp::Vector3f(p2.x, p2.y, 0.0f), sp::Vector2f(0, 0));
        vertices.emplace_back(sp::Vector3f(p3.x, p3.y, 0.0f), sp::Vector2f(1, 0));

        render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices), sp::MeshData::Type::Dynamic);
        render_data.shader = sp::Shader::get("internal:basic.shader");
        render_data.texture = sp::texture_manager.get(texture);
    }
    else
    {
        render_data.mesh = nullptr;
    }
}


ActionButton::ActionButton(sp::P<sp::gui::Widget> parent)
: sp::gui::Widget(parent)
{
    loadThemeStyle("actionbutton.background");
    label = new sp::gui::Label(this, "actionbutton.forground");
    label->layout.alignment = sp::Alignment::Bottom;
    label->layout.margin.bottom = 5;
    label->setAttribute("scale_to_text", "true");
    icon = new Icon(this);
    icon->layout.fill_height = true;
    icon->layout.fill_width = true;
    slave_widget = label;
}

void ActionButton::setLabel(const sp::string& value)
{
    label->setLabel(value);
}

void ActionButton::setAttribute(const sp::string& key, const sp::string& value)
{
    if (key == "label" || key == "caption")
    {
        label->setLabel(value);
    }
    else if (key == "text_size" || key == "text.size")
    {
        label->setAttribute(key, value);
    }
    else if (key == "image" || key == "rotation")
    {
        icon->setAttribute(key, value);
    }
    else if (key == "imagesize")
    {
        icon->layout.fill_height = icon->layout.fill_width = false;
        icon->layout.size = sp::stringutil::convert::toVector2d(value);
        icon->layout.alignment = sp::Alignment::Center;
    }
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        label->setAttribute("style", value + ".forground");
    }
    else if (key == "keybinding")
    {
        keybinding = sp::io::Keybinding::getByName(value);
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void ActionButton::onUpdate(float delta)
{
    sp::gui::Widget::onUpdate(delta);

    if (isVisible() && isEnabled() && keybinding)
    {
        if (keybinding->getDown())
        {
            playThemeSound(State::Normal);
            keybinding_down = true;
        }
        if (keybinding->getUp() && keybinding_down)
        {
            keybinding_down = false;
            playThemeSound(State::Hovered);
            runCallback(sp::Variant());
        }
    }
}

void ActionButton::updateRenderData()
{
    updateRenderDataToThemeImage();
}

bool ActionButton::onPointerDown(sp::io::Pointer::Button button, sp::Vector2d position, int id)
{
    if (isEnabled())
        playThemeSound(State::Normal);
    return true;
}

void ActionButton::onPointerUp(sp::Vector2d position, int id)
{
    if (position.x >= 0 && position.x <= getRenderSize().x && position.y >= 0 && position.y <= getRenderSize().y && isEnabled())
    {
        playThemeSound(State::Hovered);
        runCallback(sp::Variant());
    }
}
