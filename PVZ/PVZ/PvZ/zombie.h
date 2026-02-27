#pragma once
#include "global.h"

// 封装所有僵尸的**通用属性**和**默认构造逻辑**，作为抽象基类被具体僵尸派生
// 所有僵尸都具备血量、行为状态、动画帧、移动位置、冰冻减速等公共特征，抽离到基类实现代码复用
// 基类构造函数自动完成僵尸唯一编号分配、公共状态初始化，派生类仅需处理自身特有属性（血量/类型）

class Zombie
{
public:
	int HP;                 // 僵尸血量：不同僵尸血量不同（普通1/路障2/铁桶3），血量归0则僵尸死亡
	int row;                // 僵尸所在游戏行号：游戏地图为5行草地，标识僵尸在第几行移动/啃食
	int location;           // 僵尸的像素横坐标：控制僵尸从右向左移动，与地图格子像素坐标联动
	int emerge1walk2eat3;   // 僵尸当前行为状态：1-出土 2-行走 3-啃食植物，绑定对应动画帧数组
	int frameNo;            // 僵尸当前动画帧序号：根据行为状态播放对应动画帧，控制动画效果
	int height;             // 僵尸图像的固定高度（像素）：用于图像绘制的坐标校准，保证绘制对齐
	int No;                 // 僵尸唯一编号：关联全局zombieNum计数器，用于链表管理和精准删除
	int changeFrameCountDown; // 动画帧切换倒计时：单位为游戏帧，归0则切换下一帧，控制动画播放速度
	int isFrozen;           // 僵尸冰冻状态：0-未冰冻 1-被寒冰菇冻结，冻结时无法移动/啃食
	int isSlowed;           // 僵尸减速状态：0-未减速 1-被寒冰菇减速，减速时移动/动画速度降低
	int type;               // 僵尸类型：关联global.h中定义的僵尸宏（NORMALZOMBIE/CONEHEADZOMBIE等）
	int emergeFrameCount; // 僵尸出土动画播放计数：用于记录出土动画播放次数，播放完毕后切换为行走状态
	int slowMoveCounter; // 减速状态下的移动计数：用于控制减速时的移动频率

	// 构造函数
	// 分配全局唯一编号（取当前zombieNum的数值）
	// 初始化冰冻、减速状态
	// 初始化僵尸图像高度为115像素
	// 初始化动画帧起始序号为19：贴合出土动画帧序（出土共20帧，0~19，从最后一帧过渡到行走更自然）
	// 初始化僵尸行为状态为1（出土）：僵尸从墓碑生成时先执行出土动画
	// 初始化帧切换倒计时为10帧：每10游戏帧切换一次动画，控制僵尸动画播放速度
	Zombie();

    ~Zombie()= default;

};

class NormalZombie : public Zombie
{
public:
	// 构造函数
	// 初始化普通僵尸血量为1，类型为普通僵尸（关联global.h的NORMALZOMBIE宏）
	NormalZombie();

};

class ConeheadZombie : public Zombie
{
public:
	// 构造函数
	// 初始化路障僵尸血量为2：先打掉路障（1血），再打本体（1血）才会死亡。
	// 类型为路障僵尸（关联global.h的CONEHEADZOMBIE宏）
	ConeheadZombie();

};

class BucketheadZombie : public Zombie
{
public:
	// 构造函数
	// 初始化铁桶僵尸血量为3：打掉铁桶（2血），再打本体（1血）才会死亡
	// 类型为铁桶僵尸（关联global.h的BUCKETHEADZOMBIE宏）
	BucketheadZombie();

};
