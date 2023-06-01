#include "pathpreview.h"
#include "direction.h"
#include "grid.h"
#include "level.h"

#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <unordered_set>


struct VisitData {
    sp::Vector2i p;
    Direction d;
    bool operator==(const VisitData& vd) const { return p == vd.p && d == vd.d; }
};
namespace std { template <> struct hash<VisitData> { size_t operator()(const VisitData &vd) const { return vd.p.x ^ vd.p.y ^ static_cast<size_t>(vd.d); } }; }


sp::P<sp::Node> buildPreviewPath(sp::P<sp::Node> root)
{
    auto preview_base = new sp::Node(root);

    auto quad = sp::MeshData::createQuad({2.0f, 2.0f});
    for(auto [p, d, c] : level.start_points) {
        p += gridOffset(d);
        std::unordered_set<VisitData> visited;
        for(int n=0; n<50; n++) {
            if (visited.find({p, d}) != visited.end()) {
                break;
            }
            visited.insert({p, d});

            auto node = new sp::Node(preview_base);
            node->render_data.type = sp::RenderData::Type::Additive;
            node->render_data.shader = sp::Shader::get("internal:basic.shader");
            node->render_data.mesh = quad;
            node->render_data.texture = sp::texture_manager.get("path-straight.png");
            node->render_data.color = c;
            node->render_data.color.a *= 0.5;
            node->setPosition(gridToPos(p));
            node->setRotation(static_cast<int>(d) * -60 + 60);
            auto action = action_grid.get(p);
            if (action >= GridAction::TurnToUpRight && action <= GridAction::TurnToUpLeft) {
                auto target_direction = static_cast<Direction>(static_cast<int>(action) - 1);
                if (d == target_direction + 1) {
                    d = target_direction;
                    node->render_data.texture = sp::texture_manager.get("path-turn.png");
                    node->setRotation(static_cast<int>(d) * -60 - 60);
                }
                if (d == target_direction - 1) {
                    d = target_direction;
                    node->render_data.texture = sp::texture_manager.get("path-turn.png");
                    node->setRotation(static_cast<int>(d) * -60 + 180);
                }
                n -= 1;
            }

            p += gridOffset(d);
        }
    }
    return preview_base;
}