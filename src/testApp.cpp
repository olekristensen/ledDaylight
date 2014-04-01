#include "testApp.h"

std::string exec(char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

//--------------------------------------------------------------
void testApp::setup()
{
    hideGUI = false;

    scenes.push_back(new darknessFollowsScene());

    for(std::vector<ledScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it)
    {
        ledScene* s = *(it);
        s->setup();
    }

    setGUI();

//    sourceCamera.connect();
//	  cameraController.load("cameraSettings.xml");

    ofSetFrameRate(30);

}

void testApp::setGUI()
{
    vector<string> sceneNames;

    for(std::vector<ledScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it)
    {
        ledScene* s = *(it);
        sceneNames.push_back(s->name);
    }

    gui = new ofxUISuperCanvas("LEDlys Control 1.5b");

//    gui->addLabel("Press 'h' to Hide GUIs", OFX_UI_FONT_SMALL);
    gui->setWidth(ofGetWidth()/3.);
    gui->setFont("GUI/DroidSans.ttf");
    gui->setFontSize(OFX_UI_FONT_LARGE, 10);
    gui->setFontSize(OFX_UI_FONT_MEDIUM, 8);
    gui->setFontSize(OFX_UI_FONT_SMALL, 6);
    gui->addLabel("");
    gui->addLabel("Scene", OFX_UI_FONT_LARGE);
    gui->addSpacer();
    gui->addLabel("");
    gui->addRadio("sceneName", sceneNames)->activateToggle(sceneNames.back());
    gui->addSpacer();
    gui->addLabel("");

    for(std::vector<ledScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it)
    {
        ledScene* s = *(it);
        s->setGUI(gui);
    }

    gui->addLabel("Presets", OFX_UI_FONT_LARGE);
    gui->addSpacer();
    gui->addLabel("Keep settings as xml files", OFX_UI_FONT_SMALL);
    gui->addSpacer();
    gui->addLabelButton("Load", bLoadSettings, gui->getRect()->getWidth()-8, 20)->setColorBack(ofColor(48,48,48));
    gui->addLabelButton("Save", bSaveSettings, gui->getRect()->getWidth()-8, 20)->setColorBack(ofColor(48,48,48));
    gui->addSpacer();
    gui->addFPS();

    gui->getRect()->setHeight(ofGetHeight());
    gui->autoSizeToFitWidgets();

    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
    string name = e.getName();
    int kind = e.getKind();
    //cout << "got event from: " << name << endl;
    if(name == "Save")
    {
        ofxUIButton *button = (ofxUIButton *) e.getButton();
        if(button->getValue())
        {
            ofFileDialogResult dr = fullScreenSaveDialog( ofToDataPath("settings")+"/settings.xml", "Save settings");
            if(dr.bSuccess)
            {
                ofFile file(dr.filePath);
                if(file.getExtension() != "xml")
                {
                    file = ofFile(dr.filePath + ".xml");
                }
                gui->saveSettings(file.getAbsolutePath());
            }
        }
    }
    else if(name == "Load")
    {
        ofxUIButton *button = (ofxUIButton *) e.getButton();
        if(button->getValue())
        {

            ofFileDialogResult dr = fullScreenLoadDialog("Load settings", false, ofToDataPath("settings")+"/*.xml");

            if(dr.bSuccess)
            {
                ofFile file(dr.filePath);
                if(file.getExtension() == "xml")
                {
                    gui->loadSettings(file.getAbsolutePath());
                }
            }
        }
    }
}


//--------------------------------------------------------------
void testApp::update()
{

//    buffer.Blackout();

    ofxUIRadio * rSceneNames = (ofxUIRadio*) gui->getWidget("sceneName");
    string activeSceneName = rSceneNames->getActive()->getName();

    for(std::vector<ledScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it)
    {
        ledScene* s = *(it);
        if(s->name == activeSceneName)
        {
            if(activeScene != s)
            {
                if(activeScene)
                {
                    activeScene->hideGUI();
                }
                activeScene = s;
                activeScene->showGUI();
            }
        }
    }

    activeScene->update();

/*    if (!ola_client.SendDmx(0, buffer))
    {
        cout << "Send DMX failed" << endl;
    }

    //  cameraController.update();
*/
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofBackgroundGradient(ofColor(40), ofColor(10), OF_GRADIENT_CIRCULAR);
    ofEnableDepthTest();
    ofEnableSmoothing();
    float viewportWidth = (ofGetWidth()-gui->getRect()->getWidth());
    ofPushMatrix(); {
    ofViewport(gui->getRect()->getWidth(),0,viewportWidth,ofGetHeight());

        activeScene->draw();

    ofViewport();
    } ofPopMatrix();

//    cameraController.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{
    if(key == 'f')
    {
        ofToggleFullscreen();
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
    if(activeScene){
        activeScene->mouseMoved( x,  y);
    }
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
if(activeScene){
        activeScene->mouseDragged( x,  y,  button);
    }
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{
if(activeScene){
        activeScene->mousePressed( x,  y,  button);
    }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{
    if(activeScene){
        activeScene->mouseReleased( x,  y,  button);
    }
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{
    gui->getRect()->setHeight(ofGetHeight());
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{

}
