#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <cmath>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Point touchLocation = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

int a = 0;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Main loop flag
bool quit = false;

//Frees media and shuts down SDL
void close();

bool checkCollision(SDL_Rect a, SDL_Rect b);

enum PADDLEID
{
	PADDLE_1 = 0,
	PADDLE_2 = 1,
	PADDLE_1_2P = 2,
	PADDLE_2_2P = 3
};

class Texture
{
public:
	//Initializes variables
	Texture();

	//Deallocates memory
	~Texture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

#if defined(SDL_TTF_MAJOR_VERSION)
	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

class ball
{
public:

	ball();

	void setPosition(int x, int y);

	void handleEvent(SDL_Event* e);

	void render();

	int GetVelocityX();
	int GetVelocityY();

	void SetVelocityX(int x);
	void SetVelocityY(int y);

	void SetVelocityXY(int x, int y);

	void manageVelocity(SDL_Rect &paddle1, SDL_Rect &paddle2);

	int GetX();
	int GetY();

private:

	SDL_Point mPosition;

	int mVelocityX, mVelocityY;

	SDL_Rect mCollider;
};

class paddle
{
public:

	paddle(PADDLEID ID);

	void setPosition(int x, int y);

	void handleEvent(SDL_Event* e);

	void render();

	int GetX();
	int GetY();

	SDL_Rect mCollider;

private:

	SDL_Point mPosition;

	PADDLEID mID;
};
Texture gBGTexture;


Texture gBallTexture;

Texture gPaddleTexture;

ball gball;

paddle gPaddle1(PADDLE_1);
paddle gPaddle2(PADDLE_2);

Texture::Texture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

Texture::~Texture()
{
	//Deallocate
	free();
}

bool Texture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void Texture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void Texture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void Texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int Texture::getWidth()
{
	return mWidth;
}

int Texture::getHeight()
{
	return mHeight;
}

//**************************************************************************************************************************************************************************************

ball::ball()
{
	mPosition.x = 305;
	mPosition.y = 225;
	mVelocityX = 0;
	mVelocityY = 0;

	mCollider.w = 30;
	mCollider.h = 30;
}

void ball::setPosition(int x, int y)
{
	mPosition.x = x;
	mPosition.y = y;
}

void ball::handleEvent(SDL_Event* e)
{
	
}

void ball::render()
{
	gBallTexture.render(mPosition.x, mPosition.y);
}

int ball::GetX()
{
	return mPosition.x;
}

int ball::GetY()
{
	return mPosition.y;
}

int ball::GetVelocityX()
{
	return mVelocityX;
}
int ball::GetVelocityY()
{
	return mVelocityY;
}

void ball::SetVelocityX(int x)
{
	mVelocityX = x;
}
void ball::SetVelocityY(int y)
{
	mVelocityY = y;
}

void ball::SetVelocityXY(int x, int y)
{
	mVelocityX = x;
	mVelocityY = y;
}

void ball::manageVelocity(SDL_Rect& paddle1, SDL_Rect& paddle2)
{
	if (mPosition.x + 30 >= 640 || mPosition.x <= 0)
	{
		mVelocityX = -mVelocityX;
	}
	else if (checkCollision(mCollider, paddle1))
	{
		mVelocityX = 10;
	}
	else if (checkCollision(mCollider, paddle2))
	{
		mVelocityX = -10;
	}
	
	if (mPosition.y + 30 >= 480 || mPosition.y <= 0)
	{
		mVelocityY = -mVelocityY;
	}

	mPosition.x += mVelocityX;
	mCollider.x = mPosition.x;
	mPosition.y += mVelocityY;
	mCollider.y = mPosition.y;
}
//*********************************************************************************************************************************************************************************************************************

//**************************************************************************************************************************************************************************************

paddle::paddle(PADDLEID ID)
{
	mPosition.x = 0;
	mPosition.y = 0;

	mID = ID;
	if(mID == PADDLE_1)
		mCollider.x = 0;
	else if (mID == PADDLE_2)
		mCollider.x = 610;
	mCollider.y = 0;
	mCollider.w = 30;
	mCollider.h = 103;
}

void paddle::setPosition(int x, int y)
{
	mPosition.x = x;
	mPosition.y = y;
	mCollider.x = x;
	mCollider.y = y;
}

void paddle::handleEvent(SDL_Event* e)
{
	if (mID == PADDLE_1)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);

		if (e->type == SDL_MOUSEMOTION)
		{
			mCollider.y = mPosition.y = y - 51;
		}
		else if (e->type == SDL_FINGERMOTION)
		{
			touchLocation.x = e->tfinger.x * SCREEN_WIDTH;
			touchLocation.y = e->tfinger.y * SCREEN_HEIGHT;
			mCollider.y = mPosition.y = touchLocation.y - 51;
		}
	}
	else if (mID == PADDLE_2)
	{
	}
}

void paddle::render()
{
	gPaddleTexture.render(mPosition.x, mPosition.y);
}

int paddle::GetX()
{
	return mPosition.x;
}
int paddle::GetY()
{
	return mPosition.y;
}

//*********************************************************************************************************************************************************************************************************************

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("PongSDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	if (!gBallTexture.loadFromFile("res/gfx/ball.png"))
	{
		success = false;
		printf("Failed to load ball sprite texture!\n");
	}

	if (!gPaddleTexture.loadFromFile("res/gfx/paddle.png"))
	{
		success = false;
		printf("Failed to load paddle sprite texture!\n");
	}

	if (!gBGTexture.loadFromFile("res/gfx/Background.png"))
	{
		success = false;
		printf("Failed to load background texture!\n");
	}

	return success;
}

void close()
{
	///Free Textures
	gBallTexture.free();
	gPaddleTexture.free();
	gBGTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}

void gameLoop()
{

	//Event handler
	SDL_Event e;
	//Handle events on queue
	while (SDL_PollEvent(&e) != 0)
	{
		//User requests quit
		if (e.type == SDL_QUIT)
		{
			quit = true;
			break;
		}

		gball.handleEvent(&e);
		gPaddle1.handleEvent(&e);
	}

	//Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(gRenderer);

	gPaddle2.setPosition(610, gball.GetY() - 52);
	gball.manageVelocity(gPaddle1.mCollider, gPaddle2.mCollider);

	gBGTexture.render(0, 0);

	gball.render();
	gPaddle1.render();
	gPaddle2.render();

	//Update screen
	SDL_RenderPresent(gRenderer);
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			gball.SetVelocityXY(10, 10);
			#ifdef __EMSCRIPTEN__
				emscripten_set_main_loop(gameLoop, 0, 1);
			#else
				while (!quit)
				{
					gameLoop();
					SDL_Delay(16);
				}
			#endif
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}