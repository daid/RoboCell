#include "grid.h"
#include "conf.h"

#include <sp2/scene/scene.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/io/keyValueTreeSaver.h>
#include <sp2/stringutil/convert.h>


sp::InfiniGrid<GridAction> action_grid{GridAction::None};
sp::InfiniGrid<sp::P<sp::Node>> action_node_grid{nullptr};
sp::InfiniGrid<bool> footprint_grid{false};

sp::Vector2d gridToPos(sp::Vector2i v) {
    return {std::sqrt(3.0) * 0.5f * (v.x * 2 + v.y), v.y * 3.0f / 2.0f};
}

sp::Vector2i posToGrid(sp::Vector2d v) {
    double f_q = std::sqrt(3.0) / 3.0 * v.x  -  1.0/3.0 * v.y;
    double f_r = 2.0/3.0 * v.y;
    double f_s = -f_q - f_r;

    int q = std::round(f_q);
    int r = std::round(f_r);
    int s = std::round(f_s);

    auto q_diff = std::abs(q - f_q);
    auto r_diff = std::abs(r - f_r);
    auto s_diff = std::abs(s - f_s);

    if (q_diff > r_diff && q_diff > s_diff)
        q = -r-s;
    else if (r_diff > s_diff)
        r = -q-s;
    else
        s = -q-r;

    return {q, r};
}

sp::Vector2i gridOffset(Direction d) {
    switch(d) {
    case Direction::UpRight: return {0, 1};
    case Direction::Right: return {1, 0};
    case Direction::DownRight: return {1, -1};
    case Direction::DownLeft: return {0, -1};
    case Direction::Left: return {-1, 0};
    case Direction::UpLeft: return {-1, 1};
    }
    return {0, 0};
}

void setGridAction(sp::Vector2i p, GridAction action) {
    if (action == GridAction::None)
        action_grid.clear(p);
    else
        action_grid.set(p, action);
    if (sp::P<sp::Node> n = action_node_grid.get(p)) {
        n.destroy();
        action_node_grid.clear(p);
    }
    if (action == GridAction::None)
        return;

    auto n = new sp::Node(sp::Scene::get("MAIN")->getRoot());
    n->render_data.type = sp::RenderData::Type::Normal;
    n->render_data.shader = sp::Shader::get("internal:basic.shader");
    n->render_data.mesh = action_mesh;
    switch(action) {
    case GridAction::None: break;
    case GridAction::TurnToUpRight: n->render_data.texture = sp::texture_manager.get("action/arrow.png"); n->setRotation(60); break;
    case GridAction::TurnToRight: n->render_data.texture = sp::texture_manager.get("action/arrow.png"); n->setRotation(0); break;
    case GridAction::TurnToDownRight: n->render_data.texture = sp::texture_manager.get("action/arrow.png"); n->setRotation(-60); break;
    case GridAction::TurnToDownLeft: n->render_data.texture = sp::texture_manager.get("action/arrow.png"); n->setRotation(-120); break;
    case GridAction::TurnToLeft: n->render_data.texture = sp::texture_manager.get("action/arrow.png"); n->setRotation(180); break;
    case GridAction::TurnToUpLeft: n->render_data.texture = sp::texture_manager.get("action/arrow.png"); n->setRotation(120); break;
    case GridAction::PickupDrop: n->render_data.texture = sp::texture_manager.get("action/pickup.png"); break;
    case GridAction::Sync: n->render_data.texture = sp::texture_manager.get("action/sync.png"); break;
    case GridAction::Bind: n->render_data.texture = sp::texture_manager.get("action/bind.png"); break;
    case GridAction::FlipFlopA: n->render_data.texture = sp::texture_manager.get("action/flipflopA.png"); break;
    case GridAction::FlipFlopB: n->render_data.texture = sp::texture_manager.get("action/flipflopB.png"); break;
    case GridAction::FlopFlipA: n->render_data.texture = sp::texture_manager.get("action/flipflopA.png"); break;
    case GridAction::FlopFlipB: n->render_data.texture = sp::texture_manager.get("action/flipflopB.png"); break;
    }
    n->setPosition(gridToPos(p));
    action_node_grid.set(p, n);
}

void saveGrid(const sp::string& filename)
{
    sp::KeyValueTree tree;
    for(auto [p, v] : action_grid) {
        if (v == GridAction::None) continue;
        tree.root_nodes.emplace_back();
        tree.root_nodes.back().items["x"] = sp::string(p.x);
        tree.root_nodes.back().items["y"] = sp::string(p.y);
        tree.root_nodes.back().items["action"] = sp::string(static_cast<int>(v));
    }
    sp::io::KeyValueTreeSaver::save(filename, tree);
}

void clearGrid()
{
    action_grid.clear();
    for(auto it : action_node_grid) {
        sp::P<sp::Node>(it.data).destroy();
    }
    action_node_grid.clear();
}

bool loadGrid(const sp::string& filename)
{
    clearGrid();
    auto tree = sp::io::KeyValueTreeLoader::loadFile(filename);
    if (!tree) return false;
    for(auto n : tree->root_nodes) {
        auto x = sp::stringutil::convert::toInt(n.items["x"]);
        auto y = sp::stringutil::convert::toInt(n.items["y"]);
        auto action = sp::stringutil::convert::toInt(n.items["action"]);
        setGridAction({x, y}, static_cast<GridAction>(action));
    }
    return true;
}

sp::string gridToString()
{
    sp::string result;
    for(auto [p, v] : action_grid) {
        if (v == GridAction::None) continue;
        result += sp::string(p.x) + "," + sp::string(p.y) + "," + sp::string(static_cast<int>(v)) + ",";
    }
    return result;
}
