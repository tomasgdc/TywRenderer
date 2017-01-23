#include "Game.h"

//Engine includes
#include "Engine.h"
#include "Renderer.h"

//Include Physics
#include "Physics.h"

//Lib includes
//#include "lib.h"

//Include Font
//#include "Font.h"

//include SceneManager
#include "SceneManager.h"

//Event Manager
#include "EventManager.h"

//Sound Manager
//#include "SoundManager.h"

//Node Objects
#include "AsteroidNode.h"
#include "BulletNode.h"
#include "ShipNode.h"

//Other includes
#include <sstream>

//Audio
//#include "AudioComponent.h"


#define NUM_INTIAL_ASTEROIDS	10
#define MAX_ASTEROIDS	 		(NUM_INTIAL_ASTEROIDS*4)
#define MAX_BULLETS				60
#define MAX_LIVES				3
#define RAPID_FIRE_DELAY_FRAMES 7



float	g_asteroidRadii[] =
{
	20.0f,
	30.0f,
	40.0f,
	50.0f
};


enum enumObjectsId : uint32_t
{
	Ship = 0,
	Asteroid = 1,
	Bullet = 2
};

enum enumScores : int32_t
{
	AsteroidSize50 = 10,
	AsteroidSize40 = 20,
	AsteroidSize30 = 50,
	AsteroidSize20 = 100
};


//Global
int     g_score = 0;
double  g_dSlowMotionCountDown = 5000.0; // for 5 seconds slow motion is active
double  g_dSlowMotionTimerStart = 0.0;
double  g_dSlowMotionTimerElapsed = 0.0;
bool    g_bSlowMotion = false;

double  g_dGunCooldownCountDown = 1000.0; // 1 second cooldown
double  g_dGunCooldownTimerStart = 0.0;
double  g_dGunCooldownTimerElapsed = 0.0;
bool	g_bGunCooldown = false;

int g_nextFiredBulletIndex = 0;
int g_rapidFireTimer = 0;

int		g_lives = 500;
Actor	g_actorLives;
void	* g_ShipTexture = nullptr;
//FSOUND_SAMPLE	* g_ExplosionSound = nullptr;
//FSOUND_SAMPLE   * g_LazerSound = nullptr;

//Keys
bool g_keydown[256];
int g_keyhit[256];


//Functions
void RandomiseAsteroid(Actor& pActor, int size);
Actor CreateNewAsteroid(const Actor& pCopy, int size);
void ResetPlayerPos();
void RandomiseAsteroids(int numAsteroids);
void ResetGame();


/*
We are using fixed timestep and we are locking it at 60 fps. Plus we can manipulate it
int order to get slow motion effect.
http://gameprogrammingpatterns.com/game-loop.html
*/
Game::Game() :
	m_iGameMode(ATTRACT_MODE),
	m_iFrameNumber(0),
	m_iFramesPerSecond(60),
	m_dCurrentTime(0.0),
	m_dPreviousTime(0.0),
	m_bQuit(false)
{
	//m_pFont = new Font;
}


Game::~Game()
{
	//delete m_pFont;
	//m_pFont = nullptr;
}



void Game::Input()
{
	switch (m_iGameMode)
	{
	case Game::ATTRACT_MODE:
		//m_bQuit = IsKeyDown(VK_ESCAPE);
		if (IsKeyDown(VK_SPACE))
		{
			//ResetKeys();
			RandomiseAsteroids(NUM_INTIAL_ASTEROIDS);
			ResetGame();
			m_iGameMode = Game::PLAY_MODE;
			m_iFramesPerSecond = 60;
			ResetPlayerPos();
		}
		break;
	case Game::DEAD_MODE:
		if (IsKeyDown(VK_ESCAPE))
		{
			//ResetKeys();
			m_iGameMode = Game::ATTRACT_MODE;

		}
		break;
	case Game::PLAY_MODE:
		if (IsKeyDown(VK_ESCAPE))
		{
			//ResetKeys();
			m_iGameMode = Game::ATTRACT_MODE;
		}


		if (IsKeyDown(VK_SPACE))
		{
			if (g_rapidFireTimer)
			{
				g_rapidFireTimer--;
			}

			//Going around the sound problem by introducing cooldown for each fire.. Hell yes....
			if (!g_bGunCooldown && g_rapidFireTimer == 0)
			{
				Engine::getInstance().GetEventManager()->TriggerEvent(Event::Get_EvtData_OnFire());
				//Engine::getInstance().GetSoundManager()->PlaySnd(g_LazerSound);

				g_nextFiredBulletIndex = (g_nextFiredBulletIndex + 1) % MAX_BULLETS;
				g_rapidFireTimer = RAPID_FIRE_DELAY_FRAMES;
				g_bGunCooldown = true;
			}

			//float speedPercentage = 5.0f;//change this as needed
			//AudioComponent::SetChanelGroup(44100.0f * (speedPercentage / 100.0f));
			//Engine::getInstance().GetEventManager()->QueueEvent(Event::Get_EvtData_PhysicsCollided());
		}
		else
		{
			g_rapidFireTimer = 0;
		}

		if (IsKeyDown(VK_RIGHT) || IsKeyDown(VK_LEFT) || IsKeyDown(VK_UP))
		{
			Engine::getInstance().GetEventManager()->TriggerEvent(Event::Get_EvtData_ActorMove());
		}

		break;
	}
}


void Game::Update(double dt)
{
	if (m_iGameMode == Game::ATTRACT_MODE || m_iGameMode == Game::DEAD_MODE)
	{
		return;
	}

	//if no more lives
	if (g_lives <= 0)
	{
		m_iGameMode = Game::DEAD_MODE;
		g_bSlowMotion = false;
		return;
	}

	// allow event queue to process for up to 20 ms
	//Engine::getInstance().GetEventManager()->Update(20);

	//Check for collision
	Game::CheckCollision();

	//Give deltatime for same game speed across all rendering speed
	Engine::getInstance().GetSceneManager()->Update(dt);
}


void Game::Render()
{
	switch (m_iGameMode)
	{
	case Game::ATTRACT_MODE:
	{
		//Main title could be incorporated somewhere else
		std::string strMainTitle("asteroids");
		int const len = strMainTitle.length();
		int i = 0;

		for (auto& c : strMainTitle)
		{
			float x = Engine::getInstance().GetWindowsWidth() / 2 + (i - len / 2) * 100.0f;
			float y = Engine::getInstance().GetWindowsHeight() / 2 + 5 * sin(m_iFrameNumber * 0.1f + i);
			float w = 80 + 40 * sin(m_iFrameNumber * 0.01f * (2 + i + 1));
			float h = 80 + 40 * cos(m_iFrameNumber * 0.02f * (2 + i + 1));
			float r = 0;

			//void *pTexture = m_pFont->GetGlyphTexture(c);
			//if (pTexture != nullptr)
			//{
				//Engine::getInstance().GetRenderer()->VRenderGFX(pTexture, x, y, w, h, r);
			//}

			//Increment
			i++;
		}
		break;
	}
	case Game::DEAD_MODE:
	{
		//Main title could be incorporated somewhere else
		std::string strMainTitle("you died");
		int const len = strMainTitle.length();
		int i = 0;

		for (auto& c : strMainTitle)
		{
			float x = Engine::getInstance().GetWindowsWidth() / 2 + (i - len / 2) * 100.0f;
			float y = Engine::getInstance().GetWindowsHeight() / 2 + 5 * sin(m_iFrameNumber * 0.1f + i);
			float w = 80 + 40 * sin(m_iFrameNumber * 0.01f * (2 + i + 1));
			float h = 80 + 40 * cos(m_iFrameNumber * 0.02f * (2 + i + 1));
			float r = 0;

			//void *pTexture = m_pFont->GetGlyphTexture(c);
			//if (pTexture != nullptr)
			//{
			//	Engine::getInstance().GetRenderer()->VRenderGFX(pTexture, x, y, w, h, r);
			//}

			//Increment
			i++;
		}
		break;
	}
	case Game::PLAY_MODE:
	{
		//Draw lives
		{

			float x = 40.0f;
			float y = 60.0f;
			float radius2 = g_actorLives.m_radius + g_actorLives.m_radius;
			for (int i = 0; i < g_lives; i++)
			{
				x = x + radius2;
				//Engine::getInstance().GetRenderer()->VRenderGFX(g_ShipTexture, x, y, g_actorLives.m_radius, g_actorLives.m_radius, deg2rad(0));
			}

		}

		//Draw score
		{
			float w = 20.0f;
			float h = 20.0f;
			float r = 0.0f;

			float x = 40.0f;
			float y = h;

			//Convert to str
			std::stringstream ss;
			ss << g_score;

			//Get string
			std::string strNumber(ss.str());
			int const len = strNumber.length();

			//m_pFont->RenderText(strNumber, x, y, w, 0.0f, w, h, r);
		}

		//draw countdown
		{
			//if true
			if (g_bSlowMotion)
			{
				g_dSlowMotionTimerElapsed = g_dSlowMotionCountDown - (Engine::getInstance().GetTimeInMS() - g_dSlowMotionTimerStart);

				float w = 20.0f;
				float h = 20.0f;
				float r = 0.0f;

				float x = Engine::getInstance().GetWindowsWidth() / 2;
				float y = h;

				//Convert to str
				std::stringstream ss;
				ss << g_dSlowMotionTimerElapsed;

				//Get string
				std::string strNumber(ss.str());
				int const len = strNumber.length();

				//m_pFont->RenderText(strNumber, x, y, w, 0.0f, w, h, r);

				//Turn off slow motion after 5 seconds
				if (g_dSlowMotionTimerElapsed <= 0.0)
				{
					g_bSlowMotion = false;
					m_iFramesPerSecond = 60;
				}
			}
			else
			{
				//Not best thing
				g_dSlowMotionTimerStart = Engine::getInstance().GetTimeInMS();
			}
		}

		//draw gun cooldown
		{
			//if true
			if (g_bGunCooldown)
			{
				g_dGunCooldownTimerElapsed = g_dGunCooldownCountDown - (Engine::getInstance().GetTimeInMS() - g_dGunCooldownTimerStart);

				//Tunr off cooldown
				if (g_dGunCooldownTimerElapsed <= 0.0)
				{
					g_bGunCooldown = false;
				}
				else
				{
					float w = 20.0f;
					float h = 20.0f;
					float r = 0.0f;

					//Convert to str
					std::stringstream ss;
					ss << g_dGunCooldownTimerElapsed;

					//Get string
					std::string strNumber(ss.str());
					int const len = strNumber.length();

					float x = Engine::getInstance().GetWindowsWidth() - w * len * 2;
					float y = h;
					//m_pFont->RenderText(strNumber, x, y, w, 0.0f, w, h, r);
				}
			}
			else
			{
				//Not best thing
				g_dGunCooldownTimerStart = Engine::getInstance().GetTimeInMS();
			}
		}
		//Render all objects
		Engine::getInstance().GetSceneManager()->Render();

		break;
	}
	}
}


void Game::Initialize()
{
	//Load font
	//m_pFont->Initialize("data/texture_glyp_data.txt", "abcdefghijklmnopqrstuvwxyz0123456789");

	/*
	//Read somewhere from files.
	"gfx/asteroid.png",
	"gfx/ship.png",
	"gfx/laser.png"

	Yes. Duplicate code now....
	TODO: Cleanup
	*/


	BulletNode* pBulletNode(nullptr);
	AsteroidNode* pAsteroidNode(nullptr);

	//Asteroid id = 0
	{
		//void * pTexture = Engine::getInstance().GetRenderer()->VLoadGFX("gfx/asteroid.png");
		//g_ExplosionSound = Engine::getInstance().GetSoundManager()->LoadSnd("audio/explosion.wav");
		//if (pTexture != nullptr && g_ExplosionSound != nullptr)
		{
			///pAsteroidNode = new AsteroidNode(pTexture);
			//Engine::getInstance().GetSceneManager()->AddChild(enumObjectsId::Asteroid, pAsteroidNode);
		}
		//else
		{
			//PrintMessageBox could not get texture
		}
	}

	//Laser id = 1
	{
		//void * pTexture = Engine::getInstance().GetRenderer()->VLoadGFX("gfx/laser.png");
		//g_LazerSound = Engine::getInstance().GetSoundManager()->LoadSnd("audio/lazer.wav");
		//if (pTexture != nullptr && g_LazerSound != nullptr)
		//{
			//pBulletNode = new BulletNode(pTexture);
			//Engine::getInstance().GetSceneManager()->AddChild(enumObjectsId::Bullet, pBulletNode);
		//}
		{
			//PrintMessageBox could not get texture
		}
	}

	//Ship id = 2
	{
		//g_ShipTexture = Engine::getInstance().GetRenderer()->VLoadGFX("gfx/ship.png");
		if (g_ShipTexture != nullptr)
		{
			ShipNode* pShip = new ShipNode(g_ShipTexture);
			Engine::getInstance().GetSceneManager()->AddChild(enumObjectsId::Ship, pShip);

			if (pBulletNode != nullptr)
			{
				pBulletNode->AddBulletLocationOrigin(pShip->GetActor());
			}
		}
		{
			//PrintMessageBox could not get texture
		}
	}

	//Reserve size for bullets
	if (pBulletNode != nullptr)
		pBulletNode->ReserveBullets(MAX_BULLETS);

	if (pAsteroidNode != nullptr)
		pAsteroidNode->ReserveMemory(MAX_ASTEROIDS);


	//Register event
	Engine::getInstance().GetEventManager()->AddListener(std::bind(&Game::CheckCollision, this), Event::Get_EvtData_PhysicsCollided());

	// initial game mode
	m_iGameMode = Game::ATTRACT_MODE;
}

Actor CreateNewAsteroid(const Actor& pCopy, int size)
{
	Actor temp;

	temp.m_x = pCopy.m_x;
	temp.m_y = pCopy.m_y;
	temp.m_rz = rand() / (1.0f*RAND_MAX) * PI * 2 * RAD2DEG;
	float asteroidVelocity = 2 + rand() / (1.0f * RAND_MAX) * 2;
	temp.m_vx = -sin(temp.m_rz * DEG2RAD) * asteroidVelocity;
	temp.m_vy = -cos(temp.m_rz * DEG2RAD) * asteroidVelocity;
	temp.m_avz = 2 + rand() / (1.0f*RAND_MAX) * 8.0f;	// angular velocity in degrees per sec
	temp.m_size = size;
	temp.m_radius = g_asteroidRadii[size];
	temp.m_alive = true;

	return temp;
}


void RandomiseAsteroid(Actor& pActor, int size)
{
	pActor.m_x = rand() / (1.0f*RAND_MAX) * Engine::getInstance().GetWindowsWidth();
	pActor.m_y = rand() / (1.0f*RAND_MAX) * Engine::getInstance().GetWindowsHeight();
	pActor.m_rz = rand() / (1.0f*RAND_MAX) * PI * 2 * RAD2DEG;
	float asteroidVelocity = 2 + rand() / (1.0f * RAND_MAX) * 2;
	pActor.m_vx = -sin(pActor.m_rz * DEG2RAD) * asteroidVelocity;
	pActor.m_vy = -cos(pActor.m_rz * DEG2RAD) * asteroidVelocity;
	pActor.m_avz = 2 + rand() / (1.0f*RAND_MAX) * 8.0f;	// angular velocity in degrees per sec
	pActor.m_size = size;
	pActor.m_radius = g_asteroidRadii[size];
	pActor.m_alive = true;
}

void RandomiseAsteroids(int numAsteroids)
{
	ISceneNode* pSceneNode = Engine::getInstance().GetSceneManager()->GetChild(enumObjectsId::Asteroid);
	if (pSceneNode != nullptr)
	{
		//ewwwww
		AsteroidNode * pAsteroid = dynamic_cast<AsteroidNode*>(pSceneNode);

		//Clear all data
		pAsteroid->DeletActors();
		Actor temp;

		for (int i = 0; i < numAsteroids; i++)
		{
			int const size = rand() % (sizeof(g_asteroidRadii) / sizeof(g_asteroidRadii[0]));
			RandomiseAsteroid(temp, size);
			pAsteroid->AddActorToTheList(temp);
		}

	}
}

void ResetPlayerPos()
{
	//Very dangerious what I'm doing here.
	//Better return object type and check if it is correct object type instead
	//of doing dynamic_cast because it will allow to use and other derived class function
	//even if that class will be totally different. When it will come down to touching members of that class
	//the program will blow off.

	ISceneNode* pSceneNodeShip = Engine::getInstance().GetSceneManager()->GetChild(enumObjectsId::Ship);
	if (pSceneNodeShip != nullptr)
	{
		ShipNode * pShip = dynamic_cast<ShipNode*>(pSceneNodeShip);

		Actor actor;
		actor.m_x = Engine::getInstance().GetWindowsWidth() / 2;
		actor.m_y = Engine::getInstance().GetWindowsHeight() / 2;
		actor.m_vx = 0;
		actor.m_vy = 0;
		actor.m_rz = 0;
		actor.m_radius = 20;
		actor.m_alive = true;
		pShip->ResetPlayer(actor);
	}

	//Lives render
	{
		g_actorLives.m_x = Engine::getInstance().GetWindowsWidth() / 2;
		g_actorLives.m_y = Engine::getInstance().GetWindowsHeight() / 2;
		g_actorLives.m_vx = 0;
		g_actorLives.m_vy = 0;
		g_actorLives.m_rz = 0;
		g_actorLives.m_radius = 20;
		g_actorLives.m_alive = true;
	}
}

/*
TODO:
Specialize function more properly.
Do not do add score in here.
Do it somewhere else.
Create events what to do when collision occured... or smth...
*/
void Game::CheckCollision()
{
	ISceneNode * pSceneNodeAsteroid = Engine::getInstance().GetSceneManager()->GetChild(enumObjectsId::Asteroid);
	ISceneNode * pSceneNodeBullet = Engine::getInstance().GetSceneManager()->GetChild(enumObjectsId::Bullet);
	ISceneNode * pSceneNodeShip = Engine::getInstance().GetSceneManager()->GetChild(enumObjectsId::Ship);

	//check for nullptr
	if (pSceneNodeAsteroid == nullptr || pSceneNodeBullet == nullptr || pSceneNodeShip == nullptr)return;

	AsteroidNode * pAsteroidNode = dynamic_cast<AsteroidNode*>(pSceneNodeAsteroid);
	BulletNode   * pBulletNode = dynamic_cast<BulletNode*>(pSceneNodeBullet);
	ShipNode     * pShipNode = dynamic_cast<ShipNode*>(pSceneNodeShip);

	//Get all actors
	auto& refActors = pAsteroidNode->GetActorsRef();

	//Check collision for each asteroid
	for (auto& asteroid : refActors)
	{
		if (asteroid.m_alive)
		{
			//Check collision against bullet
			if (pBulletNode->CheckCollision(asteroid))
			{
				//Check if within the limits
				if (refActors.size() + 1 < MAX_ASTEROIDS)
				{
					SplitAsteroid(*pAsteroidNode, asteroid);
					if (asteroid.m_radius == g_asteroidRadii[0])g_score += enumScores::AsteroidSize20;
					else if (asteroid.m_radius == g_asteroidRadii[1])g_score += enumScores::AsteroidSize30;
					else if (asteroid.m_radius == g_asteroidRadii[2])g_score += enumScores::AsteroidSize40;
					else if (asteroid.m_radius == g_asteroidRadii[3])g_score + enumScores::AsteroidSize50;
				}

				asteroid.m_alive = false;

				//Play sound
				//float speedPercentage = 5.0f;//change this as needed
				//AudioComponent::SetChanelGroup(44100.0f * (speedPercentage / 100.0f));
			//	Engine::getInstance().GetSoundManager()->PlaySnd(g_ExplosionSound);
				return;
			}

			//Check collision against ship
			if (pShipNode->CheckCollision(asteroid))
			{
				//if ship dies then asteroid have to die tooo.....
				asteroid.m_alive = false;
				SplitAsteroid(*pAsteroidNode, asteroid);

				//slow down the game to 30 fps
				//25 fps does not give smooth slow motion
				m_iFramesPerSecond = 30;
				g_bSlowMotion = true;

				//Play sound
				//float speedPercentage = 5.0f;//change this as needed
				//AudioComponent::SetChanelGroup(44100.0f * (speedPercentage / 100.0f));
			//	Engine::getInstance().GetSoundManager()->PlaySnd(g_ExplosionSound);

				if (g_lives > 0)
				{
					g_lives -= 1;
					ResetPlayerPos();
				}
				break;
			}
		}
	}
}

void Game::SplitAsteroid(AsteroidNode & pAsteroidNode, Actor& asteroid)
{
	if (asteroid.m_radius == g_asteroidRadii[1])
	{
		g_score += enumScores::AsteroidSize30;

		pAsteroidNode.AddNewAsteroid(CreateNewAsteroid(asteroid, 0));
		pAsteroidNode.AddNewAsteroid(CreateNewAsteroid(asteroid, 0));
	}
	else if (asteroid.m_radius == g_asteroidRadii[2])
	{
		g_score += enumScores::AsteroidSize40;

		pAsteroidNode.AddNewAsteroid(CreateNewAsteroid(asteroid, 1));
		pAsteroidNode.AddNewAsteroid(CreateNewAsteroid(asteroid, 1));
	}
	else if (asteroid.m_radius == g_asteroidRadii[3])
	{
		g_score += enumScores::AsteroidSize50;

		pAsteroidNode.AddNewAsteroid(CreateNewAsteroid(asteroid, 2));
		pAsteroidNode.AddNewAsteroid(CreateNewAsteroid(asteroid, 2));
	}
}


void ResetGame()
{
	g_score = 0;
	g_lives = MAX_LIVES;
}



bool IsKeyDown(int key) // use windows VK_ codes for special keys, eg VK_LEFT; use capital chars for letter keys eg 'A', '0'
{
	return g_keydown[key & 255];
}

void ResetKeys()
{
	memset(g_keydown, 0, sizeof(g_keydown));
	memset(g_keyhit, 0, sizeof(g_keyhit));
}
