/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_

//forward declared
class RenderModel;

class  RenderModelManager 
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
	virtual void			Clear(VkDevice device) = 0;
};


class  RenderModelManagerLocal: public RenderModelManager 
{
public:
							RenderModelManagerLocal() {}
	virtual					~RenderModelManagerLocal() {}

	virtual RenderModel *	FindModel(const char *modelName);
	virtual void			AddModel(RenderModel *model);
	virtual void			RemoveModel(RenderModel *model);
	virtual void			Clear(VkDevice device);
private:
	std::map<std::string, RenderModel*> models;
	std::map<std::string, RenderModel*>::iterator it;
};



extern  RenderModelManagerLocal localModelManager;
extern  RenderModelManager* modelManager;
#endif
