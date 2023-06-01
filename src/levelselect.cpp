#include "levelselect.h"
#include "mainscene.h"
#include "level.h"
#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/io/filesystem.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/stringutil/convert.h>

static sp::P<sp::gui::Widget> gui;

void openLevelSelect()
{
    gui = sp::gui::Loader::load("gui/mainmenu.gui", "LEVEL_SELECT");
    auto container = gui->getWidgetWithID("LEVELS");
    for(auto name : sp::io::ResourceProvider::find("level/*.txt")) {
        auto key = name.substr(6, -4);
        auto [main, sub] = key.partition("-");
        if (sub == "") continue;
        auto x = sp::stringutil::convert::toInt(sub) - 1;
        auto y = sp::stringutil::convert::toInt(main) - 1;
        x += y / 2;
        auto ab = new sp::gui::Button(container);
        ab->setSize({60, 60});
        ab->setAttribute("style", "actionbutton");
        ab->layout.position.x = x;
        ab->layout.position.y = y;
        ab->setAttribute("caption", key);
        ab->setEventCallback([key](sp::Variant) {
            level.load(key);
            gui->getWidgetWithID("LEVEL_KEY")->setAttribute("caption", key);
            gui->getWidgetWithID("LEVEL_NAME")->setAttribute("caption", level.name);
            for(int save_index=1; save_index<=3; save_index++) {
                auto button = gui->getWidgetWithID("SAVE" + sp::string(save_index));
                button->enable();

                auto tree = sp::io::KeyValueTreeLoader::loadFile(sp::io::preferencePath() + level.key + "." + sp::string(save_index) + ".save");
                if (tree) {
                    button->getWidgetWithID("SUBLABEL")->setAttribute("caption", sp::string(int(tree->root_nodes.size())) + " actions");
                } else {
                    button->getWidgetWithID("SUBLABEL")->setAttribute("caption", "-");
                }
            }
        });
    }
    for(int save_index=1; save_index<=3; save_index++) {
        gui->getWidgetWithID("SAVE" + sp::string(save_index))->setEventCallback([save_index](sp::Variant) {
            gui.destroy();
            new Scene(save_index);
        });
    }
}
