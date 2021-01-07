#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using std::fstream;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

#define NUM_OF_OBSTACLES 2
#define NUM_OF_MENU 3

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Creates image from font string
        bool loadFromRenderedText( std::string textureText, SDL_Color textColor );

		//Deallocates texture
		void free();

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

		//Gets image coordinates
		int getX();
		int getY();

		//Sets image coordinates
		void setX(int xPos);
		void setY(int yPos);

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
		int x;
		int y;
};

class Roach
{
    public:
		//The dimensions of the roach
		static const int ROACH_WIDTH = 92;
		static const int ROACH_HEIGHT = 59;

		//Maximum axis velocity of the roach
		static const int ROACH_VEL = 10;

		static const float GRAVITY = 10.0f;

		//Initializes the variables
		Roach();

		//Takes key presses and adjusts the roach's position
		void handleEvent( SDL_Event& e );

		//Moves the roach and checks collision
		void move(std::vector<SDL_Rect> &shelf, std::vector<SDL_Rect> &lights);

		//Shows the roach on the screen
		void render();

		void gravitate();

		//Gets the collision boxes
        std::vector<SDL_Rect>& getColliders();

    private:
		//The X and Y offsets of the roach
		int mPosX, mPosY;

		//The velocity of the roach
		int mVelX, mVelY;

		float mRVel;

		//roach's collision boxes
        std::vector<SDL_Rect> mColliders;

        //Moves the collision boxes relative to the roach's offset
        void shiftColliders();
};

class Shelf
{
    public:
		//The dimensions of the shelf
		static const int SHELF_WIDTH = 141;
		static const int SHELF_HEIGHT = 480;

		static const float SHELF_SPEED = 1.0f;

		//The X and Y offsets of the shelf
		int mPosX, mPosY;

		Shelf();

		//Moves the roach and checks collision
		void move(std::vector<SDL_Rect> &roach);

		//Shows the shelf on the screen
		void render(bool isUpward = true);

		void accelerate();

		void randomise();

		//Gets the collision boxes
        std::vector<SDL_Rect>& getColliders();

        //Moves the collision boxes relative to the shelf's offset
        void shiftColliders();

    private:
		//The velocity of the shelf
		int mVelX, mVelY;

		float mRVel;

		//shelf's collision boxes
        std::vector<SDL_Rect> mColliders;
};

class Lights
{
    public:
		//The dimensions of the lights
		static const int LIGHTS_WIDTH = 103;
		static const int LIGHTS_HEIGHT = 480;

		static const float LIGHTS_SPEED = 1.0f;

		//The X and Y offsets of the lights
		int mPosX, mPosY;

		Lights();

		void move(int shelf_x_position, int shelf_y_position, std::vector<SDL_Rect> &roach);

		//Shows the lights on the screen
		void render(bool isUpward = true);

		void accelerate();

		void randomise(int shelf_y_position);

		//Gets the collision boxes
        std::vector<SDL_Rect>& getColliders();

        //Moves the collision boxes relative to the shelf's offset
        void shiftColliders();

    private:
		//The velocity of the lights
		int mVelX, mVelY;

		float mRVel;

		//shelf's collision boxes
        std::vector<SDL_Rect> mColliders;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

void randomise_shelf(Shelf shelf[]);

void randomise_lights(Lights lights[]);

//Box set collision detector
bool checkCollision(std::vector<SDL_Rect> &roach, std::vector<SDL_Rect> &obstacle);//std::vector<SDL_Rect> &shelf, std::vector<SDL_Rect> &lights);

//Menu
void showMenu();

//Core Game
void startGame();

//View Score
int showScore(bool isHighScore = false);

//Evaluate Score
void evaluateScore();

//Calculate Score
void calculateScore();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Globally used font
TTF_Font *gFont = NULL;

//Scene textures
LTexture gRoachTexture;
LTexture gBGTexture;
LTexture gShelfTexture;
LTexture gLightsTexture;
LTexture gScoreTexture;
LTexture gMenuTexture[3];
LTexture gGenericTexture;
LTexture gCockyTexture;


Uint32 currentScore;
Uint32 startTime;

bool endGame;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }

    //Return success
    return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

int LTexture::getX()
{
	return x;
}

int LTexture::getY()
{
	return y;
}

void LTexture::setX(int xPos)
{
    x = xPos;
}

void LTexture::setY(int yPos)
{
    y = yPos;
}

Roach::Roach()
{
    //Initialize the offsets
    mPosX = (SCREEN_WIDTH / 2) - (ROACH_WIDTH / 2);
    mPosY = SCREEN_HEIGHT / 2 - (ROACH_HEIGHT / 2);

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

    mRVel = 0.0f;

    mColliders.resize(2);

    //Initialize the collision boxes' width and height
    mColliders[ 0 ].w = 66;
    mColliders[ 0 ].h = 33;

    mColliders[ 1 ].w = 91;
    mColliders[ 1 ].h = 17;

    //Initialize colliders' relative to position
    shiftColliders();
}

void Roach::handleEvent( SDL_Event& e )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 && mRVel >= GRAVITY / 8.0f)
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_SPACE:
                mRVel = -GRAVITY / 2.2f;
            break;
        }
    }
    //If a key was released
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_SPACE:
                mRVel += GRAVITY / 8.0f;
            break;
        }
    }
}

void Roach::move(std::vector<SDL_Rect> &shelf, std::vector<SDL_Rect> &lights)
{
    if (mVelY >= GRAVITY)
    {
        mVelY = GRAVITY;
    }

    mPosY += mVelY;
    shiftColliders();

    //If the roach went too far up or down or collides to shelf or lights
    if(( mPosY < 0 ) || ( mPosY + ROACH_HEIGHT > SCREEN_HEIGHT ))
    {
        //Move back
        mPosY -= mVelY;
        shiftColliders();
        endGame = true;
    }
}

void Roach::gravitate()
{
    mRVel += 0.008f;

    mVelY = (int)mRVel;
}

void Roach::shiftColliders()
{
    mColliders[0].x = mPosX + 28; //magic #
    mColliders[0].y = mPosY + 5; //magic #

    mColliders[1].x = mPosX + 2; //magic #
    mColliders[1].y = mPosY + mColliders[0].h;
}

std::vector<SDL_Rect>& Roach::getColliders()
{
	return mColliders;
}

void Roach::render()
{
    //Show the roach
	gRoachTexture.render( mPosX, mPosY );
}

Shelf::Shelf()
{
    //Initialize the offsets
    mPosX = (SCREEN_WIDTH) - (rand() % 20);
    mPosY = rand() % SCREEN_HEIGHT + ((SCREEN_HEIGHT / 2) + 100);

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

    mRVel = 0.0f;

    mColliders.resize(1);

    //Initialize the collision boxes' width and height
    mColliders[ 0 ].w = 141;
    mColliders[ 0 ].h = 480;

    //Initialize colliders relative to position
    shiftColliders();
}

void Shelf::accelerate()
{
    mRVel += 0.008f;

    mVelX = (int)mRVel;
}

void Shelf::shiftColliders()
{
    //Row offset
    int r = 0;

    //Go through the shelf's collision boxes
    for( int set = 0; set < mColliders.size(); ++set )
    {
        mColliders[set].x = mPosX + (SHELF_WIDTH - mColliders[set].w);
        mColliders[set].y = mPosY + r;
        r += mColliders[set].h;
    }
}

std::vector<SDL_Rect>& Shelf::getColliders()
{
	return mColliders;
}

void Shelf::move(std::vector<SDL_Rect> &roach)//, std::vector<SDL_Rect> &lights)
{
    if (mVelX >= SHELF_SPEED)
    {
        mVelX = SHELF_SPEED;
    }

    mPosX -= mVelX;
    shiftColliders();

    if (mPosX + SHELF_WIDTH < 0)
    {
        mPosX = SCREEN_WIDTH;
        randomise();
        shiftColliders();
    }

    //If the shelf collides to roach
    if(checkCollision(roach, mColliders))
    {
        //Move back
        mPosX += mVelX;
        shiftColliders();
        endGame = true;
    }
}

void Shelf::render(bool isUpward)
{
    //Show the roach
	gShelfTexture.render( mPosX, mPosY );
}

void Shelf::randomise()
{
    int randomHeight;

    randomHeight = 50 + rand() % (SCREEN_HEIGHT + 1) - 50;

    if (randomHeight < (SCREEN_HEIGHT / 3))
    {
        randomHeight += 100;
    }
    mPosY = randomHeight;
}

Lights::Lights()
{
    //Initialize the offsets
    mPosX = (SCREEN_WIDTH) - (rand() % 20);
    mPosY = 0;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

    mRVel = 0.0f;

    mColliders.resize(3);

    //Initialize the collision boxes' width and height

    //pole
    mColliders[ 0 ].w = 11;
    mColliders[ 0 ].h = 420;

    //lamp
    mColliders[ 1 ].w = 103;
    mColliders[ 1 ].h = 45;

    //bulb
    mColliders[ 2 ].w = 20;
    mColliders[ 2 ].h = 17;

    //Initialize colliders relative to position
    shiftColliders();
}

void Lights::accelerate()
{
    mRVel += 0.008f;

    mVelX = (int)mRVel;
}

void Lights::shiftColliders()
{
    //Manual setting of colliders
    //pole
    mColliders[0].x = mPosX + 46; //magic #
    mColliders[0].y = mPosY;

    //lamp
    mColliders[1].x = mPosX;
    mColliders[1].y = mPosY + mColliders[0].h;

    //bulb
    mColliders[2].x = mPosX + 40; //magic #
    mColliders[2].y = mPosY + mColliders[0].h + mColliders[1].h;

}

std::vector<SDL_Rect>& Lights::getColliders()
{
	return mColliders;
}

void Lights::move(int shelf_x_position, int shelf_y_position, std::vector<SDL_Rect> &roach)//, std::vector<SDL_Rect> &shelf)
{
    if (mVelX >= LIGHTS_SPEED)
    {
        mVelX = LIGHTS_SPEED;
    }

    mPosX -= mVelX;
    shiftColliders();

    if (mPosX + LIGHTS_WIDTH < 0 && shelf_x_position > SCREEN_WIDTH / 2)
    {
        mPosX = shelf_x_position + 20;
        randomise(shelf_y_position);
        shiftColliders();
    }

    //If the lights collides to roach
    if(checkCollision(roach, mColliders))
    {
        //Move back
        mPosX += mVelX;
        shiftColliders();
        endGame = true;
    }
}

void Lights::render(bool isUpward)
{
    //Show the roach
    if (isUpward)
    {
        gLightsTexture.render( mPosX, mPosY );
    }
    else
    {
        gLightsTexture.render( mPosX, mPosY, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
    }
}

void Lights::randomise(int shelf_y_position)
{
    int randomHeight;

    randomHeight = rand() % (LIGHTS_HEIGHT - 100);

    mPosY = randomHeight * (-1);

    int lightsPosition = mPosY + LIGHTS_HEIGHT;

    if (lightsPosition >= shelf_y_position - 200)
    {
        mPosY = shelf_y_position - LIGHTS_HEIGHT - 100;
    }

    if (mPosY + LIGHTS_HEIGHT >= SCREEN_HEIGHT - 100)
    {
        mPosY -= 150;
    }
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "Cocky Roach", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

				//Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
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

	//Load roach texture
	if( !gRoachTexture.loadFromFile( "00_cocky_roach/roach.png" ) )
	{
		printf( "Failed to load roach texture!\n" );
		success = false;
	}

	//Load background texture
	if( !gBGTexture.loadFromFile( "00_cocky_roach/bg.png" ) )
	{
		printf( "Failed to load background texture!\n" );
		success = false;
	}

	//Load shelf texture
	if( !gShelfTexture.loadFromFile( "00_cocky_roach/obstacle.png" ) )
	{
		printf( "Failed to load shelf texture!\n" );
		success = false;
	}

	//Load lights texture
	if( !gLightsTexture.loadFromFile( "00_cocky_roach/lights.png" ) )
	{
		printf( "Failed to load lights texture!\n" );
		success = false;
	}

	//Load menu logo texture
	if( !gCockyTexture.loadFromFile( "00_cocky_roach/cocky_roach.png" ) )
	{
		printf( "Failed to load cocky texture!\n" );
		success = false;
	}

	//Open the font
    gFont = TTF_OpenFont( "00_cocky_roach/lazy.ttf", 28 );
    if( gFont == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }

	return success;
}

void close()
{
	//Free loaded images
	gRoachTexture.free();
	gBGTexture.free();
	gShelfTexture.free();
	gLightsTexture.free();
	gScoreTexture.free();
	gGenericTexture.free();
	gCockyTexture.free();

	for(int i = 0; i < NUM_OF_MENU; ++i)
    {
        gMenuTexture[i].free();
    }

    //Free global font
    TTF_CloseFont( gFont );
    gFont = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void randomise_shelf(Shelf shelf[])
{
    int randomHeight;

    srand(time(0));

    for (int i = 0; i < NUM_OF_OBSTACLES; ++i)
    {
        int max = SCREEN_HEIGHT - 100;
        int min = 50 + SCREEN_HEIGHT / 2;

        randomHeight = min + rand() % ((max+ 1) - min);

        if (i != 0)
        {
            shelf[i].mPosX = shelf[i - 1].mPosX + shelf[i].SHELF_WIDTH + 200;
            shelf[i].shiftColliders();
        }

        shelf[i].mPosY = randomHeight;
        shelf[i].shiftColliders();
    }
}

void randomise_lights(Lights lights[])
{
    int randomHeight;

    srand(time(0));

    for (int i = 0; i < NUM_OF_OBSTACLES; ++i)
    {
        int max = lights[i].LIGHTS_HEIGHT - 100;
        int min = (lights[i].LIGHTS_HEIGHT / 2) + 100;

        randomHeight = min + rand() % ((max+ 1) - min);

        if (i != 0)
        {
            lights[i].mPosX = lights[i - 1].mPosX + lights[i].LIGHTS_WIDTH + 250;
            lights[i].shiftColliders();
        }

        lights[i].mPosY = (randomHeight * -1);
        lights[i].shiftColliders();
    }
}

bool checkCollision(std::vector<SDL_Rect> &roach, std::vector<SDL_Rect> &obstacle) //std::vector<SDL_Rect> &shelf, std::vector<SDL_Rect> &lights)
{
    //The sides of the roach's colliders
    int leftRoach;
    int rightRoach;
    int topRoach;
    int bottomRoach;

    //The sides of the obstacle's colliders
    int leftObstacle;
    int rightObstacle;
    int topObstacle;
    int bottomObstacle;

    for (int obstacleColliderIdx = 0; obstacleColliderIdx < obstacle.size(); obstacleColliderIdx++)
    {
        //determine the sides of obstacle's colliders
        leftObstacle = obstacle[obstacleColliderIdx].x;
        rightObstacle = obstacle[obstacleColliderIdx].x + obstacle[obstacleColliderIdx].w;
        topObstacle = obstacle[obstacleColliderIdx].y;
        bottomObstacle = obstacle[obstacleColliderIdx].y + obstacle[obstacleColliderIdx].h;

        for (int roachColliderIdx = 0; roachColliderIdx < roach.size(); roachColliderIdx++)
        {
            //determine the sides of roach's colliders
            leftRoach = roach[roachColliderIdx].x;
            rightRoach = roach[roachColliderIdx].x + roach[roachColliderIdx].w;
            topRoach = roach[roachColliderIdx].y;
            bottomRoach = roach[roachColliderIdx].y + roach[roachColliderIdx].h;

            //If no sides from roach's are outside of shelf's
            if( ( ( bottomRoach <= topObstacle ) || ( topRoach >= bottomObstacle ) || ( rightRoach <= leftObstacle ) || ( leftRoach >= rightObstacle ) ) == false )
            {
                return true;
            }
        }
    }

    //If neither set of collision boxes touched
    return false;
}

void showMenu()
{
    Uint32 time;
    int x;
    int y;
    const char *labels[NUM_OF_MENU] = {"New Game", "High Score", "Exit"};
    bool isSelected[NUM_OF_MENU] = {false, false, false};
    SDL_Color color[2] = {{0, 0, 0}, {193, 0, 0}};

    int offset1 = 50;
    int offset2 = 0;

    for (int i = 0; i < NUM_OF_MENU; ++i)
    {
        if( !gMenuTexture[i].loadFromRenderedText(labels[i], color[0]) )
        {
            printf( "Unable to render menu texture!\n" );
        }

        gMenuTexture[i].setX(( SCREEN_WIDTH - gMenuTexture[i].getWidth() ) / 2);
        gMenuTexture[i].setY((offset2 + (100 + SCREEN_HEIGHT - gMenuTexture[i].getHeight() ) / 2 ) - offset1);
        offset1 -= 10;
        offset2 += gMenuTexture[i].getHeight();
    }

    SDL_Event e;

    while(1)
    {
        time = SDL_GetTicks();

        while(SDL_PollEvent(&e))
        {
            switch(e.type) {
            case SDL_QUIT:
                return;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState( &x, &y );

                for (int i = 0; i < NUM_OF_MENU; ++i)
                {
                    if (x >= gMenuTexture[i].getX() && x <= gMenuTexture[i].getX() + gMenuTexture[i].getWidth() &&
                        y >= gMenuTexture[i].getY() && y <= gMenuTexture[i].getY() + gMenuTexture[i].getHeight())
                    {
                        if (!isSelected[i])
                        {
                            isSelected[i] = true;
                            if( !gMenuTexture[i].loadFromRenderedText(labels[i], color[1]) )
                            {
                                printf( "Unable to render menu texture!\n" );
                            }
                        }
                    }
                    else
                    {
                        if (isSelected[i])
                        {
                            isSelected[i] = false;
                            if( !gMenuTexture[i].loadFromRenderedText(labels[i], color[0]) )
                            {
                                printf( "Unable to render menu texture!\n" );
                            }
                        }
                    }
                }
                break;
            case  SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState( &x, &y );

                for (int i = 0; i < NUM_OF_MENU; ++i)
                {
                    if (x >= gMenuTexture[i].getX() && x <= gMenuTexture[i].getX() + gMenuTexture[i].getWidth() &&
                        y >= gMenuTexture[i].getY() && y <= gMenuTexture[i].getY() + gMenuTexture[i].getHeight())
                    {
                        if (i == 0)
                        {
                            do {
                                startGame();
                            }while(showScore() == 99);
                        }
                        else if (i == 1)
                        {
                            showScore(true);
                        }
                        else if (i == 2)
                        {
                            return;
                        }
                    }
                }
            }
        }

        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 239, 228, 176, 0x0 );
        SDL_RenderClear( gRenderer );

        gCockyTexture.render((SCREEN_WIDTH - gCockyTexture.getWidth()) / 2, 50);
        for (int i = 0; i < NUM_OF_MENU; ++i)
        {
            gMenuTexture[i].render(gMenuTexture[i].getX(), gMenuTexture[i].getY());
        }
        //Update screen
        SDL_RenderPresent( gRenderer );

        if(1000/30 > (SDL_GetTicks()-time))
            SDL_Delay(1000/30 - (SDL_GetTicks()-time));
    }
}

void startGame()
{
    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    //The roach, shelf and lights that will be moving around on the screen
    Roach roach;
    Shelf shelf_arr[NUM_OF_OBSTACLES];
    Lights lights_arr[NUM_OF_OBSTACLES];

    currentScore = 0;
    endGame = false;
    startTime = SDL_GetTicks();

    randomise_shelf(shelf_arr);
    randomise_lights(lights_arr);

    //The background scrolling offset
    int scrollingOffset = 0;

    Uint32 oldTick = SDL_GetTicks();

    //While application is running
    while( !quit )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }

            //Handle input for the roach
            roach.handleEvent( e );
        }

        //Apply acceleration and gravity
        Uint32 currentTick = SDL_GetTicks();
        for (Uint32 i = oldTick; i < currentTick; ++i)
        {
            roach.gravitate();

            for (int j = 0; j < NUM_OF_OBSTACLES; ++j)
            {
                shelf_arr[j].accelerate();
                lights_arr[j].accelerate();
            }
        }
        oldTick = currentTick;

        roach.move(roach.getColliders(), roach.getColliders());
        for (int j = 0; j < NUM_OF_OBSTACLES; ++j)
        {
            shelf_arr[j].move(roach.getColliders());
            lights_arr[j].move(shelf_arr[j].mPosX, shelf_arr[j].mPosY, roach.getColliders());
        }

        //Scroll background
        --scrollingOffset;
        if( scrollingOffset <= -gBGTexture.getWidth() )
        {
            scrollingOffset = 0;
        }

        //Scoring
        calculateScore();

        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        //Render background
        gBGTexture.render( scrollingOffset, 0 );
        gBGTexture.render( scrollingOffset + gBGTexture.getWidth(), 0 );

        //Render objects
        roach.render();
        for (int j = 0; j < NUM_OF_OBSTACLES; ++j)
        {
            shelf_arr[j].render();
            lights_arr[j].render(false);
        }

        gScoreTexture.render(10, 10);

        //Update screen
        SDL_RenderPresent( gRenderer );

        if (endGame)
        {
            evaluateScore();
            SDL_Delay(2000);
            SDL_PumpEvents();
            SDL_FlushEvent(SDL_KEYDOWN);
            return;
        }
    }
}

int showScore(bool isHighScore)
{
    SDL_Event e;
    char c[30];
    SDL_Color color = {250, 202, 10};

    while(1)
    {
        while(SDL_PollEvent(&e))
        {
            switch(e.type) {
            case SDL_QUIT:
                return 0;
            case SDL_KEYDOWN:
                if(e.key.keysym.sym == SDLK_ESCAPE)
                {
                    return 0;
                }
                if(e.key.keysym.sym == SDLK_SPACE && !isHighScore)
                {
                    return 99; //magic #
                }
            }
        }

        if (!isHighScore)
        {
            sprintf(c, "Your score: %d", currentScore);
            if( !gScoreTexture.loadFromRenderedText(c, color) )
            {
                printf( "Unable to render score texture!\n" );
            }

            if( !gGenericTexture.loadFromRenderedText("Press [SPACE] to restart or [ESC] to exit.", color) )
            {
                printf( "Unable to render message texture!\n" );
            }
        }
        else
        {
            fstream file;
            char s[20];

            //Check high score from the file
            file.open("hs.hs", fstream::in);
            if (file.good())
            {
                file>>s;
                sprintf(s, "%d", atoi(s));
            }
            else
            {
                sprintf(s, "0");
            }

            if (file.is_open())
            {
               file.close();
            }

            sprintf(c, "High Score: %s", s);
            if( !gScoreTexture.loadFromRenderedText(c, color) )
            {
                printf( "Unable to render score texture!\n" );
            }

            if( !gGenericTexture.loadFromRenderedText("Press [ESC] to exit.", color) )
            {
                printf( "Unable to render message texture!\n" );
            }
        }

        //Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0x0 );
        SDL_RenderClear( gRenderer );

        gScoreTexture.render((SCREEN_WIDTH - gScoreTexture.getWidth()) / 2, (SCREEN_HEIGHT - gScoreTexture.getHeight()) / 2);
        gGenericTexture.render((SCREEN_WIDTH - gGenericTexture.getWidth()) / 2, (SCREEN_HEIGHT - gScoreTexture.getHeight()) / 2 + gGenericTexture.getHeight() + 50);

        //Update screen
        SDL_RenderPresent( gRenderer );
    }
    return 0;
}

void evaluateScore()
{
    fstream file;
    Uint32 highScore = 1;
    char s[20];

    //Check high score first from the file
    file.open("hs.hs", fstream::in);
    if (file.good())
    {
        file>>s;
        highScore = atoi(s);

        if (highScore >= 0 && currentScore > highScore)
        {
            highScore = currentScore;
        }
        else if (currentScore < highScore)
        {
            //magic #
            highScore = 1;
        }
    }
    else
    {
        highScore = (currentScore != 0) ? currentScore : 0;
    }

    if (file.is_open())
    {
       file.close();
    }

    //magic #; score of 1 should not happen while in game
    if (highScore != 1)
    {
        //Evaluate and update as necessary
        file.open("hs.hs", fstream::out | fstream::trunc);
        if (file.good())
        {
            sprintf(s, "%d", highScore);
            file<<s;
        }

        if (file.is_open())
        {
           file.close();
        }
    }
}

void calculateScore()
{
    SDL_Color textColor = { 72, 45, 30 };
    char c[30];

    if ((SDL_GetTicks() - startTime) % 100 == 0 && SDL_GetTicks() - startTime >= 3000)
    {
        currentScore += 5;
    }

    sprintf(c, "Score: %d", currentScore);

    //Render text
    if( !gScoreTexture.loadFromRenderedText( c, textColor ) )
    {
        printf( "Unable to render time texture!\n" );
    }
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
			startTime = 0;
			endGame = false;

			showMenu();
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
