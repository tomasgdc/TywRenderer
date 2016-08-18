/* ============================================================================
* Tywyl Engine
* Platform:      Windows
* WWW:
* ----------------------------------------------------------------------------
* Copyright 2015 Tomas Mikalauskas. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY TOMAS MIKALAUSKAS ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL TOMAS MIKALAUSKAS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Tomas Mikalauskas.

DISCLAIMER
The engine design is based on Doom3 BFG engine
https://github.com/id-Software/DOOM-3-BFG.
A lot of tweaks maded to suit my needs.
Tywyll game engine design and functionality will be changed with time.
============================================================================
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
