#include "level.h"
#include <sp2/io/resourceProvider.h>
#include <sp2/stringutil/convert.h>

LevelData level;

static Direction toDirection(const sp::string& s) {
    if (s == "UR") return Direction::UpRight;
    if (s == "R") return Direction::Right;
    if (s == "DR") return Direction::DownRight;
    if (s == "DL") return Direction::DownLeft;
    if (s == "L") return Direction::Left;
    if (s == "UL") return Direction::UpLeft;
    return Direction::Right;
}

void LevelData::load(sp::string key)
{
    start_points.clear();
    spawn_points.clear();
    goal_points.clear();
    binders.clear();

    auto stream = sp::io::ResourceProvider::get("level/" + key + ".txt");
    if (!stream) return;
    this->key = key;
    name = stream->readLine().strip();
    while(stream->tell() < stream->getSize()) {
        auto parts = stream->readLine().split();
        if (parts[0] == "R") { //Robo spawn
            level.start_points.push_back({
                { sp::stringutil::convert::toInt(parts[1]), sp::stringutil::convert::toInt(parts[2])},
                toDirection(parts[3]),
                sp::stringutil::convert::toColor(parts[4])});
        }
        else if (parts[0] == "S") { //Cell spawn
            level.spawn_points.push_back({
                { sp::stringutil::convert::toInt(parts[1]), sp::stringutil::convert::toInt(parts[2])},
                parts[3]});
        }
        else if (parts[0] == "G") { //Goal
            level.goal_points.push_back({
                { sp::stringutil::convert::toInt(parts[1]), sp::stringutil::convert::toInt(parts[2])},
                parts[4]});
            level.goal_points.back().amount = sp::stringutil::convert::toInt(parts[3]);
            std::vector<GoalBond>* bv = &level.goal_points.back().bonds;
            int idx = 5;
            while(idx < int(parts.size())) {
                bv->push_back({toDirection(parts[idx]), parts[idx+1]});
                idx += 2;
                if (idx < int(parts.size()) && parts[idx] == ">") {
                    bv = &bv->back().bonds;
                    idx++;
                }
            }
        }
        else if (parts[0] == "B") { //Binder
            level.binders.push_back({
                { sp::stringutil::convert::toInt(parts[1]), sp::stringutil::convert::toInt(parts[2])},
                toDirection(parts[3])});
        }
    }
}