#include "mainscene.h"
#include "pawn.h"
#include "robo.h"
#include "goal.h"
#include "binder.h"
#include "level.h"
#include "pathpreview.h"
#include "conf.h"
#include "levelselect.h"

#include <sp2/io/filesystem.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>
#include <sp2/scene/camera.h>
#include <sp2/math/plane.h>

sp::io::Keybinding key_space{"SPACE", " "};
sp::io::Keybinding key_exit{"EXIT", {"Escape", "AC Back"}};
sp::io::Keybinding key_q{"Q", "q"};
sp::io::Keybinding key_a{"A", "a"};
sp::io::Keybinding key_z{"Z", "z"};

sp::io::Keybinding key_w{"W", "w"};
sp::io::Keybinding key_s{"S", "s"};
sp::io::Keybinding key_x{"X", "x"};

sp::io::Keybinding key_e{"E", "e"};
sp::io::Keybinding key_d{"D", "d"};
sp::io::Keybinding key_c{"C", "c"};

sp::io::Keybinding key_r{"R", "r"};
sp::io::Keybinding key_f{"F", "f"};
sp::io::Keybinding key_v{"V", "v"};


Scene::Scene(int save_index) : sp::Scene("MAIN") {
    goal_gui = sp::gui::Loader::load("gui/hud.gui", "GOAL");
    speed_gui = sp::gui::Loader::load("gui/hud.gui", "SPEED_CONTROLS");
    speed_gui->getWidgetWithID("STOP")->setEventCallback([this](sp::Variant v){
        stop();
    });
    speed_gui->getWidgetWithID("PLAY")->setEventCallback([this](sp::Variant v){
        start();
        if (state == State::Run)
            sp::Engine::getInstance()->setGameSpeed(1.0f);
    });
    speed_gui->getWidgetWithID("FASTER")->setEventCallback([this](sp::Variant v){
        start();
        if (state == State::Run)
            sp::Engine::getInstance()->setGameSpeed(std::min(64.0f, sp::Engine::getInstance()->getGameSpeed() * 2.0f));
    });

    action_bar = sp::gui::Loader::load("gui/hud.gui", "ACTION_BAR");
    action_bar->getWidgetWithID("UPLEFT")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::TurnToUpLeft);
    });
    action_bar->getWidgetWithID("LEFT")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::TurnToLeft);
    });
    action_bar->getWidgetWithID("DOWNLEFT")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::TurnToDownLeft);
    });
    action_bar->getWidgetWithID("UPRIGHT")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::TurnToUpRight);
    });
    action_bar->getWidgetWithID("RIGHT")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::TurnToRight);
    });
    action_bar->getWidgetWithID("DOWNRIGHT")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::TurnToDownRight);
    });
    action_bar->getWidgetWithID("BIND")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::Bind);
    });
    action_bar->getWidgetWithID("BIND")->hide();
    action_bar->getWidgetWithID("DELETE")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::None);
    });
    action_bar->getWidgetWithID("PICKUP")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::PickupDrop);
    });
    action_bar->getWidgetWithID("SYNC")->setEventCallback([this](sp::Variant) {
        setClickAction(GridAction::Sync);
    });

    auto camera = new sp::Camera(getRoot());
    camera->setPerspective(30);
    camera->setPosition(sp::Quaterniond::fromAxisAngle({1, 0, 0}, 20) * sp::Vector3d(0, 0, 30));
    camera->setRotation(sp::Quaterniond::fromAxisAngle({1, 0, 0}, 20));
    setDefaultCamera(camera);

    loadGrid(sp::io::preferencePath() + level.key + "." + sp::string(save_index) + ".save");
    buildBackgroundGrid();
    buildLevelNodes();
    buildPathPreview();
    buildRobos();
    sp::Engine::getInstance()->setGameSpeed(0.0);
    this->save_index = save_index;
    onFixedUpdate();
}

Scene::~Scene()
{
    action_bar.destroy();
    speed_gui.destroy();
    goal_gui.destroy();
    finished_screen.destroy();
}

void Scene::onUpdate(float delta)
{
    if (key_space.getDown()) {
        if (state == State::Build) start(); else stop();
    }
    if (key_exit.getDown()) {
        saveGrid(sp::io::preferencePath() + level.key + "." + sp::string(save_index) + ".save");
        delete this;
        openLevelSelect();
    }
}

void Scene::onFixedUpdate()
{
    int done = 0;
    int required = 0;
    for(auto n : getRoot()->getChildren()) {
        sp::P<Goal> g = n;
        if (g) {
            done += g->done;
            required += g->required;
        }
    }

    goal_gui->getWidgetWithID("LABEL")->setAttribute("caption", "Goal: " + sp::string(done) + "/" + sp::string(required));
    if (state == State::Run) {
        if (done == required) {
            state = State::Finished;
            sp::Engine::getInstance()->setGameSpeed(0.0);
            finished_screen = sp::gui::Loader::load("gui/hud.gui", "FINISHED");
            finished_screen->getWidgetWithID("CONTINUE")->setEventCallback([this](sp::Variant) {
                stop();
            });
            finished_screen->getWidgetWithID("EXIT")->setEventCallback([this](sp::Variant) {
                delete this;
                openLevelSelect();
            });
            int actions = 0;
            for(auto [p, a] : action_grid) if (a != GridAction::None) actions++;
            int footprint = 0;
            for(auto [p, used] : footprint_grid) if (used) footprint++;
            finished_screen->getWidgetWithID("CYCLES")->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(cycles));
            finished_screen->getWidgetWithID("ACTIONS")->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(actions));
            finished_screen->getWidgetWithID("FOOTPRINT")->getWidgetWithID("AMOUNT")->setAttribute("caption", sp::string(footprint));

            level_finished_info[level.key].cycles = std::min(level_finished_info[level.key].cycles, cycles);
            level_finished_info[level.key].actions = std::min(level_finished_info[level.key].actions, actions);
            level_finished_info[level.key].footprint = std::min(level_finished_info[level.key].footprint, footprint);
            saveLevelFinishedInfo();
        } else {
            cycles += 1;
        }
    }
}

void Scene::start()
{
    switch(state)
    {
    case State::Build:
        state = State::Run;
        buildRobos();
        sp::Engine::getInstance()->setGameSpeed(1.0);
        action_bar->hide();
        saveGrid(sp::io::preferencePath() + level.key + "." + sp::string(save_index) + ".save");
        cycles = 0;
        break;
    case State::Run:
    case State::Error:
    case State::Finished:
        break;
    }
}

void Scene::stop()
{
    switch(state)
    {
    case State::Build:
        break;
    case State::Run:
    case State::Error:
    case State::Finished:
        state = State::Build;
        sp::Engine::getInstance()->setGameSpeed(0.0);
        buildRobos();
        action_bar->show();
        onFixedUpdate();
        break;
    }
}

bool dragging = false;
sp::Vector3d pointer_down_pos;
bool Scene::onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) {
    auto p3 = sp::Plane3d({0, 0, 0}, {0, 0, 1}).intersect(ray);
    pointer_down_pos = p3;
    dragging = false;
    return true;
}

void Scene::onPointerDrag(sp::Ray3d ray, int id) {
    auto p3 = sp::Plane3d({0, 0, 0}, {0, 0, 1}).intersect(ray);
    auto diff = p3 - pointer_down_pos;
    auto camera_pos = getCamera()->getPosition3D() - diff;
    getCamera()->setPosition(camera_pos);
    background_grid->setPosition(gridToPos(posToGrid({camera_pos.x, camera_pos.y + 10})));
    dragging = true;
}

void Scene::onPointerUp(sp::Ray3d ray, int id) {
    if (dragging) return;
    if (state == State::Build) {
        auto p3 = sp::Plane3d({0, 0, 0}, {0, 0, 1}).intersect(ray);
        auto p = posToGrid({p3.x, p3.y});
        setGridAction(p, click_place_action);
        buildPathPreview();
    }
}

void Scene::setClickAction(GridAction action) {
    click_place_action = action;
    action_bar->getWidgetWithID("UPLEFT")->setAttribute("style", action == GridAction::TurnToUpLeft ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("LEFT")->setAttribute("style", action == GridAction::TurnToLeft ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("DOWNLEFT")->setAttribute("style", action == GridAction::TurnToDownLeft ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("UPRIGHT")->setAttribute("style", action == GridAction::TurnToUpRight ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("RIGHT")->setAttribute("style", action == GridAction::TurnToRight ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("DOWNRIGHT")->setAttribute("style", action == GridAction::TurnToDownRight ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("BIND")->setAttribute("style", action == GridAction::Bind ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("DELETE")->setAttribute("style", action == GridAction::None ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("PICKUP")->setAttribute("style", action == GridAction::PickupDrop ? "actionbutton.selected" : "actionbutton");
    action_bar->getWidgetWithID("SYNC")->setAttribute("style", action == GridAction::Sync ? "actionbutton.selected" : "actionbutton");
}

void Scene::buildPathPreview() {
    preview_base.destroy();
    preview_base = buildPreviewPath(getRoot());
    preview_base->setPosition(sp::Vector3d{0, 0, path_z});
}

void Scene::buildRobos() {
    for(auto n : getRoot()->getChildren()) {
        sp::P<Robo> r = n;
        r.destroy();
        sp::P<Pawn> p = n;
        p.destroy();
        sp::P<Goal> g = n;
        if (g)
            g->done = 0;
    }
    finished_screen.destroy();
    error_node.destroy();
    for(auto [p, d, c] : level.start_points) {
        auto r = new Robo(getRoot());
        r->render_data.color = c;
        r->grid_position = p;
        r->setPosition(gridToPos(p));
        r->direction = d;
    }
    footprint_grid.clear();
}

static void buildGoalRecursive(sp::P<GoalNode> gn, const std::vector<LevelData::GoalBond>& bonds) {
    for(auto b : bonds) {
        auto n = gn->addBond(b.direction);
        n->render_data.texture = sp::texture_manager.get("cell/" + b.texture + ".png");
        buildGoalRecursive(n, b.bonds);
    }
}

void Scene::buildLevelNodes() {
    for(auto s : level.spawn_points) {
        auto ps = new PawnSpawn(getRoot(), s.grid_point);
        ps->render_data.texture = sp::texture_manager.get("cell/" + s.texture + ".png");
    }

    for(auto g : level.goal_points) {
        auto goal = new Goal(getRoot(), g.grid_point);
        goal->render_data.texture = sp::texture_manager.get("cell/" + g.texture + ".png");
        goal->done = 0;
        goal->required = g.amount;
        buildGoalRecursive(goal, g.bonds);
    }
    for(auto b : level.binders)
        new Binder(getRoot(), b.grid_point, b.direction);
}

void Scene::buildBackgroundGrid() {
    background_grid = new sp::Node(getRoot());
    background_grid->render_data.type = sp::RenderData::Type::Normal;
    background_grid->render_data.shader = sp::Shader::get("internal:color.shader");
    
    sp::MeshData::Indices indices;
    sp::MeshData::Vertices vertices;

    float w = std::sqrt(3.0f) * 1.0f;
    float h = 2.0f;
    for(int x=-14; x<14; x++) {
        for(int y=-10; y<10; y++) {
            auto idx = vertices.size();
            float cx = w * x + (y & 1) * w * 0.5;
            float cy = h * 3 / 4 * y;
            vertices.emplace_back(sp::Vector3f{cx, cy - 0.8f, grid_z});
            vertices.emplace_back(sp::Vector3f{cx + w * 0.5f * 0.8f, cy - 0.5f * 0.8f, grid_z});
            vertices.emplace_back(sp::Vector3f{cx + w * 0.5f * 0.8f, cy + 0.5f * 0.8f, grid_z});
            vertices.emplace_back(sp::Vector3f{cx, cy + 0.8f, grid_z});
            vertices.emplace_back(sp::Vector3f{cx - w * 0.5f * 0.8f, cy + 0.5f * 0.8f, grid_z});
            vertices.emplace_back(sp::Vector3f{cx - w * 0.5f * 0.8f, cy - 0.5f * 0.8f, grid_z});
            indices.push_back(idx);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);
            indices.push_back(idx);
            indices.push_back(idx + 2);
            indices.push_back(idx + 3);
            indices.push_back(idx);
            indices.push_back(idx + 3);
            indices.push_back(idx + 4);
            indices.push_back(idx);
            indices.push_back(idx + 4);
            indices.push_back(idx + 5);
        }
    }
    background_grid->render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices));
    background_grid->render_data.color = sp::Color(0.3, 0.3, 0.3);
}

void Scene::error(sp::Vector2d error_position)
{
    if (state == State::Run) {
        state = State::Error;
        sp::Engine::getInstance()->setGameSpeed(0.0);

        error_node = new sp::Node(getRoot());
        error_node->setPosition(sp::Vector3d(error_position.x, error_position.y, error_z));
        error_node->render_data.type = sp::RenderData::Type::Transparent;
        error_node->render_data.shader = sp::Shader::get("internal:basic.shader");
        error_node->render_data.mesh = sp::MeshData::createQuad({1.5, 1.5});
        error_node->render_data.texture = sp::texture_manager.get("action/delete.png");
        error_node->render_data.color = sp::Color(1.0, 0.0, 0.0, 1.0);
    }
}
