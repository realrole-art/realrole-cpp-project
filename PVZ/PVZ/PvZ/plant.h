#pragma once
#include "global.h"
class Plant
{
public:
	int type;               // 植物类型：关联global.h中定义的植物宏（如POTATOMINE、ICESHROOM）
	int HP;                 // 植物血量：单位为僵尸啃食次数（掉至0则植物死亡）
	int frameNo;            // 植物当前动画帧序号：控制自身动画播放（如土豆雷出土、寒冰菇特效）
	int No;                 // 植物唯一编号：关联全局plantNum计数器，用于链表管理和精准删除
	int x;                  // 植物所在的像素横坐标（对应地图格子的像素坐标）
	int y;                  // 植物所在的像素纵坐标（对应地图格子的像素坐标）
	int changeFrameCountDown; // 动画帧切换倒计时：单位为游戏帧，倒计时归0则切换下一帧，控制动画播放速度

	// 构造函数
	// 为当前植物分配全局唯一编号（取当前plantNum值）
	// 初始化帧切换倒计时为5帧：每5游戏帧切换一次动画，控制动画速度
	// 初始化植物基础血量为6
	Plant();

	~Plant()= default;
};
// 土豆雷
// 埋地（倒计时）→ 出土就绪 → 爆炸（倒计时）
class PotatoMine : public Plant
{
public:
	int underCountDown = 400; // 土豆雷埋地倒计时：单位游戏帧，归0前为埋地状态（POTATO），归0后出土就绪（POTATOMINE）
	int boomCountDown = 50;   // 土豆雷爆炸倒计时：单位游戏帧，触发爆炸后开始倒计时，归0后爆炸特效结束，移除土豆雷

	// 构造函数
	// 初始化土豆雷动画帧起始序号为0（从第0帧开始播放出土动画）
	// 初始化植物类型为土豆雷就绪状态（关联global.h的POTATOMINE宏）
	PotatoMine();

};

// 墓碑吞噬者
// 特有逻辑：仅能种植在墓碑上，动画帧与墓碑类型绑定，当前暂未扩展特有属性
// 注：type默认初始化为GRAVEBUSTER_GRAVE1，后续可根据实际种植的墓碑类型动态修改
class GraveBuster : public Plant
{
public:
	// 构造函数
	// 初始化动画帧起始序号为1（从第1帧开始播放吞噬墓碑动画，适配动画资源帧序）
	// 初始化类型为作用于墓碑1的吞噬者（后续可动态修改为对应墓碑的宏）
	GraveBuster();

};

// 寒冰菇
// // 寒冰菇核心逻辑：种植后触发全屏特效→僵尸先被**冻结**（无法移动）→冻结结束后**减速**（移动变慢），通过两个倒计时控制
class IceShroom : public Plant
{
public:
	int frozenCountDown = 200;  // 僵尸冻结倒计时：单位游戏帧，归0前僵尸完全冻结（无法移动/啃食）
	int slowingCountDown = 1000;// 僵尸减速倒计时：单位游戏帧，冻结结束后开始倒计时，归0前僵尸移动速度降低

	// 构造函数
	// 初始化动画帧起始序号为0（从第0帧开始播放寒冰菇爆炸冰冻特效）
	// 初始化植物类型为寒冰菇（关联global.h的ICESHROOM宏）
	IceShroom();

};

