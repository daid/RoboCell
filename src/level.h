#pragma once

#include "direction.h"
#include "grid.h"
#include <sp2/graphics/color.h>
#include <sp2/math/vector2.h>

class LevelData {
public:
    sp::string key;
    sp::string name;
    sp::string info;
    
    struct StartPoint {
        sp::Vector2i grid_point;
        Direction direction;
        sp::Color color;
    };
    std::vector<StartPoint> start_points;
    struct Spawn {
        sp::Vector2i grid_point;
        sp::string texture;
    };
    std::vector<Spawn> spawn_points;
    struct GoalBond {
        Direction direction;
        sp::string texture;
        std::vector<GoalBond> bonds;
    };
    struct Goal {
        sp::Vector2i grid_point;
        sp::string texture;
        int amount = 10;
        std::vector<GoalBond> bonds;
    };
    std::vector<Goal> goal_points;
    struct Binder {
        sp::Vector2i grid_point;
        Direction direction;
    };
    std::vector<Binder> binders;
    struct Action {
        sp::Vector2i grid_point;
        GridAction action;
    };
    std::vector<Action> preplaced_actions;

    void load(sp::string key);
};
extern LevelData level;

struct LevelFinishedInfo
{
    int cycles = 0;
    int actions = 0;
    int footprint = 0;
};
extern std::unordered_map<sp::string, LevelFinishedInfo> level_finished_info;
void saveLevelFinishedInfo();
void loadLevelFinishedInfo();
