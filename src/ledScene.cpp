#include "ledScene.h"

ledScene::ledScene()
{

}

ledScene::~ledScene()
{

}

void ledScene::hideGUI()
{
    for(std::vector<ofxUIWidget*>::iterator it = guiWidgets.begin(); it != guiWidgets.end(); ++it)
    {
        ofxUIWidget* w = *(it);
        w->setVisible(false);
    }
//    gui->centerWidgetsOnCanvas(false, false);
}

void ledScene::showGUI()
{
    for(std::vector<ofxUIWidget*>::iterator it = guiWidgets.begin(); it != guiWidgets.end(); ++it)
    {
        ofxUIWidget* w = *(it);
        w->setVisible(true);
    }
//    gui->centerWidgetsOnCanvas(false, false);
}

