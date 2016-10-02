/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>

//Renderer Includes
#include "Model_local.h"
#include "ModelManager.h"


RenderModelManagerLocal localModelManager;
RenderModelManager* modelManager = &localModelManager;

/*
========================
AddModel
========================
*/
void RenderModelManagerLocal::AddModel(RenderModel *model) {
	if (!model)return;
	models.insert(std::map<std::string, RenderModel*>
		::value_type(model->getName(),(model)));
}

/*
========================
FindModel
========================
*/
RenderModel *	RenderModelManagerLocal::FindModel(const char *modelName) {
	it = models.find(modelName);
	if (it == models.end()) {
		//Engine::getInstance().Sys_Error("ERROR: RenderModelManagerLocal::FindModel -> could not find (%s) model", modelName);
		return nullptr;
	}
	return it->second;
}

/*
========================
RemoveModel
========================
*/
void RenderModelManagerLocal::RemoveModel(RenderModel *model) {
	if (!model)return;
	it = models.find(model->getName());
	if (it == models.end()){
		//Engine::getInstance().Sys_Error("ERROR: RenderModelManagerLocal::RemoveModel -> could not find (%s) model", model->getName());
	}
	models.erase(it);
}

/*
========================
Clear
========================
*/
void RenderModelManagerLocal::Clear(VkDevice device) 
{
	for (it = models.begin(); it != models.end(); ++it) 
	{
		it->second->Clear(device);
	}
	models.clear();
}