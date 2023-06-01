#pragma once

#include "grid.h"
#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>

class Scene : public sp::Scene
{
public:
    Scene(int save_index);

    void onUpdate(float delta) override;
    bool onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) override;
    void onPointerDrag(sp::Ray3d ray, int id) override;
    void onPointerUp(sp::Ray3d ray, int id) override;

    void setClickAction(GridAction action);
    void buildPathPreview();
    void buildRobos();
    void buildLevelNodes();
    void buildBackgroundGrid();

    void start();
    void stop();
    void error(sp::Vector2d error_position);

    sp::P<sp::Node> preview_base;
    sp::P<sp::Node> background_grid;
    sp::P<sp::Node> error_node;
    sp::P<sp::gui::Widget> action_bar;
    sp::P<sp::gui::Widget> goal_gui;

    GridAction click_place_action = GridAction::None;
    enum class State {
        Build,
        Run,
        Error,
    } state = State::Build;

    int save_index = 0;
};