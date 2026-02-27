#include "zombie.h"
// 基类构造：初始化公共字段并分配唯一编号
Zombie::Zombie()
{
	No = zombieNum++;
	HP = 0;
	row = 0;
	location = 0;
	emerge1walk2eat3 = 1; // 出土
	frameNo = 19; // 出土动画最后一帧起始
	height = 115;
	changeFrameCountDown = 10;
	isFrozen = 0;
	isSlowed = 0;
	type = 0;
	emergeFrameCount = 0;
	slowMoveCounter = 0;
}
// 普通僵尸
NormalZombie::NormalZombie() : Zombie()
{
	HP = 1;
	type = NORMALZOMBIE;
}


// 路障僵尸
ConeheadZombie::ConeheadZombie() : Zombie()
{
	HP = 2;
	type = CONEHEADZOMBIE;
}

// 铁桶僵尸
BucketheadZombie::BucketheadZombie() : Zombie()
{
	HP = 3;
	type = BUCKETHEADZOMBIE;
}


