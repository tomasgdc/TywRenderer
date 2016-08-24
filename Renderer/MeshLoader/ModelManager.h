/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_

//forward declared
class RenderModel;

class TYWRENDERER_API RenderModelManager 
{
public:
	virtual					~RenderModelManager() {}

	//removes model from hash map
	virtual void			RemoveModel(RenderModel *model) = 0;

	// returns NULL if modelName is NULL, also returns NULL
	//if model could not be founded
	virtual	RenderModel *	FindModel(const char *modelName) = 0;

	//adds model to the hash map
	virtual void			AddModel(RenderModel *model) = 0;

	//clears all models from hash map
	virtual void			Clear() = 0;
};


class TYWRENDERER_API RenderModelManagerLocal: public RenderModelManager 
{
public:
							RenderModelManagerLocal() {}
	virtual					~RenderModelManagerLocal() {}

	virtual RenderModel *	FindModel(const char *modelName);
	virtual void			AddModel(RenderModel *model);
	virtual void			RemoveModel(RenderModel *model);
	virtual void			Clear();
private:
	std::map<std::string, RenderModel*> models;
	std::map<std::string, RenderModel*>::iterator it;
};



extern TYWRENDERER_API RenderModelManagerLocal localModelManager;
extern TYWRENDERER_API RenderModelManager* modelManager;
#endif
