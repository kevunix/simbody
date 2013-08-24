/*
 * OgreVisualizer.cpp
 * Taken from Ogre Wiki
*/
#include "OgreVisualizer.h"
#include "DynamicLines.h"
#include <OgreSceneNode.h>
#include <pthread.h>
#include <sstream>

#include "MovableText.h"

using namespace Visualizer;

//-------------------------------------------------------------------------------------
OgreVisualizer::OgreVisualizer() : lineCount(0)
{
}
//-------------------------------------------------------------------------------------
OgreVisualizer::~OgreVisualizer(void)
{
}

//-------------------------------------------------------------------------------------
void OgreVisualizer::createScene(void)
{
	backgroundNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("background");
	//
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);	
	/*
	Ogre::Plane skyPlane;
	skyPlane.d = 100;
	skyPlane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;

	mSceneMgr->setSkyPlane(true, skyPlane, "Examples/CloudySky", 500, 20, true, 0.5f, 150, 150);
	*/
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);

    Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
 
    Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 20, 20, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
 
    Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");

    backgroundNode->attachObject(entGround);
	entGround->setMaterialName("Viz/Brick");
    entGround->setCastShadows(false);

    Ogre::Light* directionalLight = mSceneMgr->createLight("directionalLight");
    directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
    directionalLight->setDiffuseColour(Ogre::ColourValue::White);
    directionalLight->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
 
    directionalLight->setDirection(Ogre::Vector3( 0.55, -0.3, -0.75 )); 
    //directionalLight->setPosition(Ogre::Vector3( 100, 100, 0 )); 

	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));

	mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("main");
}

void OgreVisualizer::drawLine(RenderedLine& line, const std::string& name)
{
	Ogre::SceneNode *sceneNode = NULL;
  	Ogre::ManualObject *obj = NULL;
  	bool attached = false;

	float r,g,b;
	r = line.getColor()[0];
	g = line.getColor()[1];
	b = line.getColor()[2];

  	if (this->mSceneMgr->hasManualObject(name))
  	{
    	sceneNode = this->mSceneMgr->getSceneNode(name);
   		obj = this->mSceneMgr->getManualObject(name);
    	attached = true;
  	}
  	else
  	{
    	sceneNode = this->mainNode->createChildSceneNode(name);
    	obj = this->mSceneMgr->createManualObject(name);
  	}

  	sceneNode->setVisible(true);
  	obj->setVisible(true);

  	obj->begin(name, Ogre::RenderOperation::OT_LINE_LIST);
	
	for (int j = 0; j < line.getLines().size(); j+=3 )
	{
		obj->position(line.getLines().at(j),
						line.getLines().at(j+1),
						line.getLines().at(j+2));
		obj->colour(r, g, b);
	}

  	obj->end();

	// Lines don't cast shadows
	obj->setCastShadows(false);

  	if (!attached)
	{
    	sceneNode->attachObject(obj);
	}
}

void OgreVisualizer::drawText(RenderedText& text, const std::string& name)
{
	Ogre::MovableText* msg = new Ogre::MovableText(name, text.getText());
	msg->setColor(Ogre::ColourValue(text.getColor()[0], text.getColor()[1], text.getColor()[2]));
	msg->setCharacterHeight(0.1);
	msg->setSpaceWidth(0.5);

	Ogre::SceneNode *sceneNode = this->mainNode->createChildSceneNode(name);

	sceneNode->setVisible(true);

	sceneNode->translate(Ogre::Vector3(text.getPosition()[0],
										text.getPosition()[1],
										text.getPosition()[2]));

	fVec4 rot = X_GC.R().convertRotationToAngleAxis();

	sceneNode->rotate(Ogre::Vector3(rot[1], rot[2], rot[3]), Ogre::Radian(rot[0]));

//	msg->setGlobalTranslation(Ogre::Vector3(text.getPosition()[0], text.getPosition()[1], text.getPosition()[2]));
//	msg->setLocalTranslation(Ogre::Vector3(text.getPosition()[0], text.getPosition()[1], text.getPosition()[2]));
/*
	sceneNode->scale(Ogre::Vector3(text.getScale()[0],
						text.getScale()[1],
						text.getScale()[2]));
*/
	sceneNode->attachObject(msg);
}

bool OgreVisualizer::drawManualObject(RenderedMesh& renderedMesh, Ogre::ManualObject * obj, const std::string& materialName)
{
	Mesh * mesh = meshes[renderedMesh.getMeshIndex()][renderedMesh.getResolution()];

	float r,g,b;
	r = renderedMesh.getColor()[0];
	g = renderedMesh.getColor()[1];
	b = renderedMesh.getColor()[2];

//	Let's draw this
	unsigned short representation = renderedMesh.getRepresentation();
	if(representation == DecorativeGeometry::DrawSurface)
	{
	  	obj->begin(materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		vector<unsigned short> faces = mesh->getFaces();
		vector<float> vertices = mesh->getVertices();
		vector<float> normals = mesh->getNormals();

		for(int i = 0; i < vertices.size(); i+=3)
		{
			obj->position(vertices.at(i), vertices.at(i+1), vertices.at(i+2));
			obj->colour(r,g,b);
		}

		for(int i = 0; i < normals.size(); i+=3)
		{
			obj->normal(normals.at(i), normals.at(i+1), normals.at(i+2));
		}
		for (int j = 0; j < faces.size(); j+=3 )
		{
			obj->triangle(faces.at(j), faces.at(j+1), faces.at(j+2));
		}

  		obj->end();

	}
	else if (representation == DecorativeGeometry::DrawPoints)
	{
	  	obj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
		vector<float> vertices = mesh->getVertices();
		for (int j = 0; j < vertices.size(); j+=3 )
		{
			obj->position(vertices.at(j), vertices.at(j+1), vertices.at(j+2));
			obj->colour(r,g,b);
		}
  		obj->end();

	}
	else if (representation == DecorativeGeometry::DrawWireframe)
	{
	  	obj->begin(materialName, Ogre::RenderOperation::OT_LINE_LIST);
		vector<unsigned short> edges = mesh->getEdges();
		vector<float> vertices = mesh->getVertices();
		vector<float> normals = mesh->getNormals();

		for(int i = 0; i < vertices.size(); i+=3)
		{
			obj->position(vertices.at(i), vertices.at(i+1), vertices.at(i+2));
			obj->colour(r,g,b);
		}

		for(int i = 0; i < normals.size(); i+=3)
		{
			obj->normal(normals.at(i), normals.at(i+1), normals.at(i+2));
		}

		for (int j = 0; j < edges.size(); j+=3 )
		{
//			obj->position(edges.at(j), edges.at(j+1), edges.at(j+2));
		}

  		obj->end();
	}
	else
	{
		std::cout << "YOU SHOULDN'T BE HERE\n";
		return false;
	}

	return true;

}

void OgreVisualizer::drawMesh(RenderedMesh& renderedMesh, const std::string& name)
{
	Ogre::SceneNode * sceneNode = NULL;
  	bool attached = false;
	Ogre::ManualObject * obj = NULL;

	if (this->mSceneMgr->hasManualObject(name))
	{
		sceneNode = this->mSceneMgr->getSceneNode(name);
		obj = this->mSceneMgr->getManualObject(name);
		attached = true;
	}
	else
	{
		sceneNode = this->mainNode->createChildSceneNode(name);
		obj = this->mSceneMgr->createManualObject(name);
	}

	sceneNode->setVisible(true);
  	obj->setVisible(true);

	sceneNode->translate(Ogre::Vector3(renderedMesh.getTransform().p()[0],
										renderedMesh.getTransform().p()[1],
										renderedMesh.getTransform().p()[2]));

	fVec4 rot = renderedMesh.getTransform().R().convertRotationToAngleAxis();

	sceneNode->rotate(Ogre::Vector3(rot[1], rot[2], rot[3]), Ogre::Radian(rot[0]));
	sceneNode->scale(Ogre::Vector3(renderedMesh.getScale()[0],
						renderedMesh.getScale()[1],
						renderedMesh.getScale()[2]));

  	if (!attached && drawManualObject(renderedMesh, obj, "" ))
	{
    	sceneNode->attachObject(obj);
	}
}

void OgreVisualizer::drawTransparentMesh(RenderedMesh& renderedMesh, const std::string& name)
{
	Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().create(name, "General");
	matPtr->setReceiveShadows(false);
	matPtr->getTechnique(0)->setLightingEnabled(true);

	matPtr->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	matPtr->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
//	matPtr->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1);

	float r,g,b;
	r = renderedMesh.getColor()[0];
	g = renderedMesh.getColor()[1];
	b = renderedMesh.getColor()[2];

	matPtr->getTechnique(0)->getPass(0)->setAmbient(r, g, b);
	matPtr->getTechnique(0)->getPass(0)->setDiffuse(r, g, b, 0.2);

	Ogre::SceneNode * sceneNode = NULL;
  	bool attached = false;
	Ogre::ManualObject * obj = NULL;

	if (this->mSceneMgr->hasManualObject(name))
	{
		sceneNode = this->mSceneMgr->getSceneNode(name);
		obj = this->mSceneMgr->getManualObject(name);
		attached = true;
	}
	else
	{
		sceneNode = this->mainNode->createChildSceneNode(name);
		obj = this->mSceneMgr->createManualObject(name);
	}

	sceneNode->setVisible(true);
  	obj->setVisible(true);

	sceneNode->translate(Ogre::Vector3(renderedMesh.getTransform().p()[0],
										renderedMesh.getTransform().p()[1],
										renderedMesh.getTransform().p()[2]));

	fVec4 rot = renderedMesh.getTransform().R().convertRotationToAngleAxis();

	sceneNode->rotate(Ogre::Vector3(rot[1], rot[2], rot[3]), Ogre::Radian(rot[0]));
	sceneNode->scale(Ogre::Vector3(renderedMesh.getScale()[0],
						renderedMesh.getScale()[1],
						renderedMesh.getScale()[2]));

  	if (!attached && drawManualObject(renderedMesh, obj, name ))
	{
    	sceneNode->attachObject(obj);
	}

	

}

void OgreVisualizer::destroyAllAttachedMovableObjects( Ogre::SceneNode * node )
{
	if(!node) return;

 	// Destroy all the attached objects
	Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

	while ( itObject.hasMoreElements() )
	  node->getCreator()->destroyMovableObject(itObject.getNext());

	// Recurse to child SceneNodes
	Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

	while ( itChild.hasMoreElements() )
	{
		Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      	destroyAllAttachedMovableObjects( pChildNode );
	}

}

void OgreVisualizer::renderScene()
{

	pthread_mutex_lock(&sceneLock);

	mSceneMgr->destroyAllManualObjects();
	mainNode->removeAndDestroyAllChildren();
	destroyAllAttachedMovableObjects(mainNode);
	mainNode->getCreator()->destroySceneNode(mainNode);
	mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("main");

	lineCount = 0;

    if (scene != NULL) {


		for (int i = 0; i < (int) pendingCommands.size(); i++)
		{
			pendingCommands[i]->execute(meshes);
			delete pendingCommands[i];
		}
		pendingCommands.clear();

        for (int i = 0; i < (int) scene->lines.size(); i++)
		{
		std::string str;
		ostringstream convert;
		convert << "line" << lineCount;
		str = convert.str();
			drawLine(scene->lines[i], str);
			lineCount++;
        }
        for (int i = 0; i < (int) scene->sceneText.size(); i++)
		{
		std::string str;
		ostringstream convert;
		convert << "text" << lineCount;
		str = convert.str();

          	drawText(scene->sceneText[i], str);
			lineCount++;
		}
        for (int i = 0; i < (int) scene->drawnMeshes.size(); i++)
		{
		std::string str;
		ostringstream convert;
		convert << "mesh" << lineCount;
		str = convert.str();

            drawMesh(scene->drawnMeshes[i], str);
			lineCount++;
		}

        for (int i = 0; i < (int) scene->solidMeshes.size(); i++)
		{
		std::string str;
		ostringstream convert;
		convert << "line" << lineCount;
		str = convert.str();

			drawMesh(scene->solidMeshes[i], str);
			lineCount++;
		}
		
        vector<pair<float, int> > order(scene->transparentMeshes.size());
        for (int i = 0; i < (int) order.size(); i++)
		{
            order[i] = make_pair((float)(~X_GC.R()*scene->transparentMeshes[i].getTransform().p())[2], i);
		}
        sort(order.begin(), order.end());
        for (int i = 0; i < (int) order.size(); i++)
		{
		std::string str;
		ostringstream convert;
		convert << "line" << lineCount;
		str = convert.str();

            drawTransparentMesh(scene->transparentMeshes[order[i].second], str);

			lineCount++;
		}

        scene->sceneHasBeenDrawn = true;
    }

	pthread_cond_signal(&sceneHasBeenDrawn);  
	pthread_mutex_unlock(&sceneLock);

}

void OgreVisualizer::go()
{

	if(!setup())
	{
		return;
	}

	createScene();


	while(true)
	{

//		std::cout << "FPS: " << mWindow->getLastFPS() << std::endl;

		renderScene();
		// Do stuff
		mRoot->renderOneFrame();
	}
}

int main(int argc, char *argv[])
{

	bool talkingToSimulator = false;
	int inPipe, outPipe;

	if (argc >= 3) {
		std::stringstream(argv[1]) >> inPipe;
		std::stringstream(argv[2]) >> outPipe;
        talkingToSimulator = true; // presumably those were the pipes
  	} else {
        printf("\n**** VISUALIZER HAS NO SIMULATOR TO TALK TO ****\n");
        printf("The Visualizer  was invoked directly with no simulator\n");
        printf("process to talk to. Will attempt to bring up the display anyway\n");
        printf("in case you want to look at the About message.\n");
        printf("The Visualizer is intended to be invoked programmatically.\n");
    }

	// Create application object
	OgreVisualizer app;

	std::cout << "-=-=-=-[Initializing] inPipe: " << inPipe << std::endl;
	if(talkingToSimulator) 
	{
		app.shakeHandsWithSimulator(inPipe, outPipe);
		pthread_t thread;

		VisualizerBase::VisualizerCom vcom;
		vcom.arg = &app;
		vcom.inPipe = inPipe;
		vcom.outPipe = outPipe;
		int rc = pthread_create(&thread, NULL, VisualizerBase::listenForInputHelper, &vcom);
	}
//	app.dumpMessage();

	try {
		app.go();

	} catch( Ogre::Exception& e ) {
		std::cerr << "An exception has occured: " <<
			e.getFullDescription().c_str() << std::endl;
	}

	return 0;

}