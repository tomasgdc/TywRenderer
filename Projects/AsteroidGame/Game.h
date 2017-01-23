#pragma once

#include <vector>
#include <unordered_map>
#include "Actor.h"

//Forward declaration
class Font;
class AsteroidNode;

class Game
{
public:
	enum GameMode
	{
		ATTRACT_MODE,
		DEAD_MODE,
		PLAY_MODE
	};

public:
	Game();
	~Game();

	void Initialize();
	void Input();
	void Update(double dt);
	void Render();
	bool IsQuit() { return m_bQuit; }

	double m_dCurrentTime;
	double m_dPreviousTime;
	int m_iGameMode;
	int m_iFrameNumber;
	int m_iFramesPerSecond;
protected:

	void CheckCollision();

	//Yess, can't think of better name
	//What it will. It will split big asteroid into smaller asteroids (up to you how many you want to have)
	void SplitAsteroid(AsteroidNode & pAsteroidNode, Actor& asteroid);

private:
	Font* m_pFont;
	std::unordered_map<std::string, void*> mesh;
	bool m_bQuit;
};


extern bool IsKeyDown(int key); // use windows VK_ codes for special keys, eg VK_LEFT; use capital chars for letter keys eg 'A', '0'. use VK_LBUTTON and VK_RBUTTON for mouse buttons. 
extern void ResetKeys();