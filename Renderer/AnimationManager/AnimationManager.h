//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#pragma once

//forward declaration
class MD5Anim;


class TYWANIMATION_API AnimationManager
{
public:
	AnimationManager();
	~AnimationManager();

	MD5Anim* GetAnimation(std::string fileName, std::string filePath);

	void PurgeAnimations();
private:
	std::unordered_map<std::string, MD5Anim*>	m_mapAnimations;
};
