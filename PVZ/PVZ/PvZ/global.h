#pragma once
#include <iostream>
#include <ctime>
#include <string>
#include <graphics.h>
#include <conio.h>
#include <Windows.h>
#include <io.h>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <mmsystem.h>
#include <fstream>
#include <sstream>
#include "plant.h"
#include "zombie.h"
using namespace std;

#define GRASS 0               // 草地（空地块）
#define GRAVE1 1              // 墓碑1（共8种墓碑）
#define GRAVE2 2              // 墓碑2
#define GRAVE3 3              // 墓碑3
#define GRAVE4 4              // 墓碑4
#define GRAVE5 5              // 墓碑5
#define GRAVE6 6              // 墓碑6
#define GRAVE7 7              // 墓碑7
#define GRAVE8 8              // 墓碑8
#define POTATO 9              // 土豆雷-埋地状态
#define POTATOMINE 10         // 土豆雷-出土就绪状态
#define POTATOBOOM 11         // 土豆雷-爆炸状态
#define GRAVEBUSTER_GRAVE1 12 // 墓碑吞噬者-作用于墓碑1
#define GRAVEBUSTER_GRAVE2 13 // 墓碑吞噬者-作用于墓碑2
#define GRAVEBUSTER_GRAVE3 14 // 墓碑吞噬者-作用于墓碑3
#define GRAVEBUSTER_GRAVE4 15 // 墓碑吞噬者-作用于墓碑4
#define GRAVEBUSTER_GRAVE5 16 // 墓碑吞噬者-作用于墓碑5
#define GRAVEBUSTER_GRAVE6 17 // 墓碑吞噬者-作用于墓碑6
#define GRAVEBUSTER_GRAVE7 18 // 墓碑吞噬者-作用于墓碑7
#define GRAVEBUSTER_GRAVE8 19 // 墓碑吞噬者-作用于墓碑8
#define ICESHROOM 20          // 寒冰菇
#define NORMALZOMBIE 21       // 普通僵尸
#define CONEHEADZOMBIE 22     // 路障僵尸
#define BUCKETHEADZOMBIE 23   // 铁桶僵尸


extern int zombieNum;    // 僵尸唯一编号计数器（每生成1个僵尸+1）
extern int plantNum;     // 植物唯一编号计数器（每生成1个植物+1）
extern int sunNum;       // 阳光唯一编号计数器（每生成1个阳光+1）
extern int bangNum;      // 爆炸效果唯一编号计数器（每生成1个爆炸+1）
extern double groanFrequency; // 僵尸呻吟声随机触发概率


extern IMAGE potatoBoom;                // 土豆雷爆炸图
extern IMAGE potato;                    // 土豆雷埋地图
extern IMAGE grave[8];                  // 8种墓碑图
extern IMAGE hammer[13];                // 锤子动画帧（13帧）
extern IMAGE tmpImg;                    // 临时图片1：用于僵尸帧缓存
extern IMAGE tmpImg2;                   // 临时图片2：用于僵尸冰冻效果缓存
extern IMAGE potatoMinePictures[8];    // 土豆雷动画帧（8帧）
extern IMAGE iceshroomPictures[11];     // 寒冰菇动画帧（11帧）
extern IMAGE gravebusterPictures[28];   // 墓碑吞噬者动画帧（28帧）
extern IMAGE sunPictures[22];           // 阳光动画帧（22帧）
extern IMAGE normalZombieWalkPictures[47];   // 普通僵尸行走帧（47帧）
extern IMAGE normalZombieEmergePictures[20]; // 普通僵尸出土帧（20帧）
extern IMAGE normalZombieEatPictures[10];    // 普通僵尸啃食帧（10帧）
extern IMAGE coneheadZombieWalkPictures[47]; // 路障僵尸行走帧（47帧）
extern IMAGE coneheadZombieEmergePictures[20];// 路障僵尸出土帧（20帧）
extern IMAGE coneheadZombieEatPictures[10];   // 路障僵尸啃食帧（10帧）
extern IMAGE bucketheadZombieWalkPictures[47];// 铁桶僵尸行走帧（47帧）
extern IMAGE bucketheadZombieEmergePictures[20];// 铁桶僵尸出土帧（20帧）
extern IMAGE bucketheadZombieEatPictures[10];   // 铁桶僵尸啃食帧（10帧）
extern IMAGE plantsBar;    // 植物选择栏背景
extern IMAGE menu;         // 菜单按钮图
extern IMAGE background;   // 游戏背景图
extern IMAGE selectID;     // 存档选择界面背景
extern IMAGE iceTrap;      // 寒冰菇冻结标记图
extern IMAGE snow;         // 寒冰菇雪花特效图
extern IMAGE lawnmower;    // 除草机图
extern IMAGE loseGame;     // 游戏失败图
extern IMAGE winGame;      // 游戏胜利图
extern IMAGE bang;         // 锤击爆炸特效图

extern ExMessage mousemsg; // EasyX鼠标消息结构体：存储鼠标坐标/点击状态
// 坐标结构体：存储地图每个格子的x/y像素坐标
struct coordinate
{
	int x; // 像素x坐标
	int y; // 像素y坐标
};

// 枚举：鼠标光标类型（当前选中的操作/植物）
enum CURSORFLAG
{
	Chammer,        // 锤子光标（默认，可锤僵尸/收集阳光/选植物）
	CpotatoMine,    // 土豆雷光标（选中种植土豆雷）
	Ciceshroom,     // 寒冰菇光标（选中种植寒冰菇）
	Cgravebuster    // 墓碑吞噬者光标（选中种植墓碑吞噬者）
};

extern coordinate xys[32][32];  // 地图格子坐标数组（实际用5行9列）
extern CURSORFLAG cursor;       // 当前鼠标光标类型（默认Chammer）
extern RECT rect; // 游戏底部提示文字绘制区域
extern char sunshineNum[10];    // 存储当前阳光数的字符数组（用于绘制到界面）
extern char username[200];      // 存储当前存档用户名
extern vector<string> files;    // 存储archives文件夹下的所有存档文件名

// 爆炸效果类：锤击僵尸/阳光时的爆炸特效
class Bang
{
public:
	int No;          // 爆炸唯一编号
	int x;           // 爆炸x像素坐标
	int y;           // 爆炸y像素坐标
	int countDown;   // 爆炸持续倒计时（倒计时结束后删除特效）

	// 构造函数：初始化爆炸坐标和倒计时，爆炸持续20帧
	Bang(int x, int y);
	Bang() = default;
};

// 阳光类：游戏中可收集的阳光（收集后增加阳光数）
class Sun
{
public:
	int x;                  // 阳光当前x坐标
	int y;                  // 阳光当前y坐标
	int frame;              // 阳光当前动画帧序号
	int No;                 // 阳光唯一编号
	int changeFrameCountDown; // 阳光帧切换倒计时（控制动画速度）
	int goToCount;          // 阳光是否被点击：0-未点击（漂浮），1-已点击（向左上角移动）
	int goToCountFrame;     // 阳光移动到左上角的帧计数
	int tempX;              // 阳光被点击时的初始x坐标（用于移动计算）
	int tempY;              // 阳光被点击时的初始y坐标（用于移动计算）

	// 构造函数：初始化阳光的初始坐标和所有动画/状态参数
	// @param x: 阳光生成的初始像素横坐标  @param y: 阳光生成的初始像素纵坐标
	// 初始化：每5帧切换一次动画、初始未被点击、移动帧计数为0
	Sun(int x, int y);
	Sun() = default;
};



// 除草机为被动防御，当僵尸到达本行最左侧时触发，清除本行所有僵尸
class Lawnmower
{
public:
	int location = -20; // 除草机的像素横坐标，初始-20表示隐藏在游戏界面左侧外
	int isActivated = 0; // 除草机激活状态：0-未激活 1-已激活（触发后不可复用）
	int isOut = 0;       // 除草机出动状态：0-未出动 1-正在出动（向右侧移动清僵尸）
};


// 模板结点类：适用于**任意类型**的游戏动态元素（僵尸、植物、阳光、特效等）
// 作为链表的基本单元，封装元素指针和后继结点指针，提高代码复用性
// @template T: 结点存储的游戏元素类型（如Sun、Bang、Zombie等）
template<class T>
class Node
{
public:
	T* content;
	Node* next = NULL;
	Node(T* t)
	{
		content = t;
	}
};

template<class T>
class LinkList
{
public:
	Node<T>* head;  
	Node<T>* tail;

	LinkList()
	{
		head = NULL;
		tail = NULL;
	};

	LinkList(Node<T> node)
	{ 
		head = node; 
		tail = node; 
	};

	~LinkList()
	{ 
		DeleteAllNode();
	}     

	void InsertNode(T* t)
	{
		Node<T>* node=new Node<T>(t);
		if (head == NULL)
		{
			head = node;
			tail = node;
		}
		else
		{
			tail->next = node;
			tail = node;
		}
	};

	void DeleteNode(int No)
	{
		Node<T>* cur = head,*pre=NULL;
		while (cur != NULL && cur->content->No != No)
		{
			pre = cur;
			cur = cur->next;
		}

		if (pre == NULL)
		{
			head = cur->next;
		}
		else if (cur == NULL)
		{
			cout << "没有找到符合条件的结点！" << endl;
			return;
		}
		else
		{
			pre->next = cur->next;
		}

		if (cur == tail)
		{
			tail = pre;
		}
		delete cur;
	};

	void DeleteAllNode()
	{
		Node<T>* cur = head,*pre=NULL;
		while (tail != NULL)
		{
			pre = cur;
			cur = cur->next;
			DeleteNode(pre->content->No);
		}
	};
};
