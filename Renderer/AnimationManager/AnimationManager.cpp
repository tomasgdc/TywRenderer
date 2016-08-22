//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.

#include <RendererPch\stdafx.h>
#include "Geometry\JointTransform.h"

#include "MD5Anim\MD5Anim.h"
#include "AnimationManager.h"



//=====================================
AnimationManager::AnimationManager()
//=====================================
{

}

//=====================================
AnimationManager::~AnimationManager()
//=====================================
{
	PurgeAnimations();
}

//==============================================================================================
MD5Anim* AnimationManager::GetAnimation(std::string fileName, std::string filePath)
//==============================================================================================
{
	MD5Anim* anim(nullptr);
	

	//check if already exsits
	auto iter = m_mapAnimations.find(fileName);
	if (iter != m_mapAnimations.end())
	{
		anim = iter->second;
	}
	else
	{
		anim = TYW_NEW MD5Anim;
		if (!anim->LoadAnim(fileName, filePath))
		{
			SAFE_DELETE(anim);
			anim = nullptr;
		}
		else
		{
			m_mapAnimations.insert(std::make_pair(fileName, anim));
		}
	}
	return anim;
}

//===========================================
void AnimationManager::PurgeAnimations()
//===========================================
{

	for (auto iter = m_mapAnimations.begin(); iter != m_mapAnimations.end(); ++iter)
	{
		SAFE_DELETE(iter->second);
	}
	m_mapAnimations.clear();
}