#include "global.h"

int zombieNum = 0;
int plantNum = 0;
int sunNum = 0;
int bangNum = 0;
double groanFrequency = 0.0005;

ExMessage mousemsg = { 0 };
coordinate xys[32][32];
CURSORFLAG cursor = Chammer;
RECT rect = { 0, 500, 820, 600 };
char sunshineNum[10] = { 0 };
char username[200] = { 0 };
vector<string> files;
Sun::Sun(int x, int y) : x(x), y(y)
{
	this->frame = 0;
	this->No = sunNum++;
	this->changeFrameCountDown = 5;
	this->goToCount = 0;
	this->goToCountFrame = 0;
	this->tempX = x;
	this->tempY = y;
}
Bang::Bang(int x, int y) : x(x), y(y)
{
	this->No = bangNum++;
	this->countDown = 20;
}

IMAGE grave[8];
IMAGE hammer[13];
IMAGE potatoMinePictures[8];
IMAGE iceshroomPictures[11];
IMAGE gravebusterPictures[28];
IMAGE sunPictures[22];

IMAGE normalZombieWalkPictures[47];
IMAGE normalZombieEmergePictures[20];
IMAGE normalZombieEatPictures[10];

IMAGE coneheadZombieWalkPictures[47];
IMAGE coneheadZombieEmergePictures[20];
IMAGE coneheadZombieEatPictures[10];

IMAGE bucketheadZombieWalkPictures[47];
IMAGE bucketheadZombieEmergePictures[20];
IMAGE bucketheadZombieEatPictures[10];

IMAGE potatoBoom;
IMAGE potato;
IMAGE tmpImg;
IMAGE tmpImg2;
IMAGE plantsBar;
IMAGE menu;
IMAGE background;
IMAGE selectID;
IMAGE iceTrap;
IMAGE snow;
IMAGE lawnmower;
IMAGE loseGame;
IMAGE winGame;
IMAGE bang;