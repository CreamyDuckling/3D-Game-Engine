#include "SceneGraph.h"
#include "../Graphics/ShaderHandler.h"

int SceneGraph::totalObjectsAdded = 0;

std::map<GLuint, std::vector<Model*>> SceneGraph::sceneModels = std::map<GLuint, std::vector<Model*>>();
std::map<std::string, GameObject*> SceneGraph::sceneObjects = std::map<std::string, GameObject*>();
std::map<std::string, GUIObject*> SceneGraph::sceneGUIObjects = std::map<std::string, GUIObject*>();

std::unique_ptr<SceneGraph> SceneGraph::instance = nullptr;

SceneGraph::SceneGraph() {}
SceneGraph::~SceneGraph() {
	OnDestroy();
}

void SceneGraph::OnDestroy() {
	if (sceneObjects.size() > 0) {
		for (auto o : sceneObjects) {
			delete o.second, o.second = nullptr;
		}
		sceneObjects.clear();
	}
	if (sceneGUIObjects.size() > 0) {
		for (auto go : sceneGUIObjects) {
			delete go.second, go.second = nullptr;
		}
		sceneGUIObjects.clear();
	}
	if (sceneModels.size() > 0) {
		for (auto m : sceneModels) {
			if (m.second.size() > 0) {
				for (auto m_ptr : m.second) {
					delete m_ptr, m_ptr = nullptr;
				}
			}
			m.second.clear();
		}
		sceneModels.clear();
	}
}

void SceneGraph::Update(const float deltaTime_) {
	for (auto o : sceneObjects) {
		o.second->Update(deltaTime_);
	}
}

void SceneGraph::Render(Camera* camera_) {
	for (auto m : sceneModels) {
		glUseProgram(m.first);
		for (auto m_ptr : m.second) {
			m_ptr->Render(camera_);
		}
	}
	glUseProgram(0);
}

void SceneGraph::Draw(Camera* _camera) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(ShaderHandler::GetInstance()->GetShader("SpriteShader"));
	for (auto go : sceneGUIObjects) {
		go.second->Draw(_camera);
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

SceneGraph* SceneGraph::GetInstance() {
	if (instance.get() == nullptr) {
		instance.reset(new SceneGraph);
	}
	return instance.get();
}

void SceneGraph::AddModel(Model* model_) {
	if (sceneModels.find(model_->GetShaderProgram()) == sceneModels.end()) {
		std::vector<Model*> tempVector = std::vector<Model*>();
		tempVector.reserve(10);
		tempVector.push_back(model_);
		sceneModels.insert(std::pair<GLuint, std::vector<Model*>>(model_->GetShaderProgram(), tempVector));
	} else {
		sceneModels[model_->GetShaderProgram()].push_back(model_);
	}
}

/*void SceneGraph::RemoveModel(GLuint program_, Model* model_) {
	// Deallocate the model from memory
	Model* m_ptr = nullptr;
	for (int i = 0; i < sceneModels[program_].size(); i++) {
		m_ptr = sceneModels[program_].at(i);
		if (m_ptr == model_) {
			delete m_ptr, m_ptr = nullptr;
			break; // Exit out of the loop
		}
	}
	// After that, remove the index from the map
	sceneModels[program_].erase(sceneModels[program_].begin());
}*/

void SceneGraph::AddGameObject(GameObject* gameObject_, std::string name_) {
	totalObjectsAdded++;
	if (name_ == "") {
		std::string newName = "GameObject" + std::to_string(totalObjectsAdded);
		gameObject_->SetName(newName);
		sceneObjects[newName] = gameObject_;
	} else if (sceneObjects.find(name_) == sceneObjects.end()) {
		gameObject_->SetName(name_);
		sceneObjects[name_] = gameObject_;
	} else {
		DebugLogger::Warning("Object with the name " + name_ + " already exists in the scene.", "SceneGraph.cpp", __LINE__);
		AddGameObject(gameObject_, ""); // Treat this as adding an object with no name associated with it as an error catch
	}
	// Adding the object to the collision handler
	CollisionHandler::GetInstance()->AddObject(gameObject_);
}

void SceneGraph::RemoveGameObject(std::string name_) {
	CollisionHandler::GetInstance()->RemoveObject(sceneObjects[name_]);
	delete sceneObjects[name_], sceneObjects[name_] = nullptr;
	sceneObjects.erase(name_);
}

GameObject* SceneGraph::GetGameObject(std::string name_) {
	if (sceneObjects.find(name_) != sceneObjects.end()) {
		return sceneObjects.at(name_);
	}
	DebugLogger::Warning("GameObject with that tag name doesn't exist. The value \"nullptr\" was returned.", "SceneGraph.cpp", __LINE__);
	return nullptr;
}

void SceneGraph::AddGUIObject(GUIObject* _guiObject, std::string _name) {
	if (_name == "") {
		std::string newTag = "GUIObject" + std::to_string(sceneGUIObjects.size() + 1);
		_guiObject->SetTag(newTag);
		sceneGUIObjects[newTag] = _guiObject;
	} else if (sceneGUIObjects.find(_name) == sceneGUIObjects.end()) {
		_guiObject->SetTag(_name);
		sceneGUIObjects[_name] = _guiObject;
	} else {
		std::string newTag = "GUIObject" + std::to_string(sceneGUIObjects.size() + 1);
		_guiObject->SetTag(newTag);
		sceneGUIObjects[newTag] = _guiObject;
		DebugLogger::Warning("GUI Object with that name already exists! A new name (" + newTag + ") has been given instead.", "SceneGraph.h", __LINE__);
	}
}

void SceneGraph::RemoveGUIObject(std::string _name) {
	if (sceneGUIObjects.find(_name) != sceneGUIObjects.end()) {
		delete sceneGUIObjects[_name], sceneGUIObjects[_name] = nullptr;
		sceneGUIObjects.erase(_name);
	}
}

GUIObject* SceneGraph::GetGUIObject(std::string _name) {
	if (sceneGUIObjects.find(_name) != sceneGUIObjects.end()) {
		return sceneGUIObjects.at(_name);
	}
	DebugLogger::Warning("GUIObject with that tag name doesn't exist. The value \"nullptr\" was returned.", "SceneGraph.cpp", __LINE__);
	return nullptr;
}