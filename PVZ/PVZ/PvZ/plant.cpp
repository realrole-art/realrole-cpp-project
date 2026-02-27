#include "plant.h"
Plant::Plant():x(0),y(0)
{
	this->No = plantNum++;                    // 分配全局唯一编号
	this->changeFrameCountDown = 5;           // 初始化帧切换倒计时为5帧
	this->HP = 6;                             // 初始化植物基础血量为6
}

PotatoMine::PotatoMine()
{
	this->frameNo = 0;                        // 初始化动画帧起始序号为0
	this->type = POTATOMINE;                  // 初始化植物类型为土豆雷就绪状态
}

GraveBuster::GraveBuster()
{
	this->frameNo = 1;                        // 初始化动画帧起始序号为1
	this->type = GRAVEBUSTER_GRAVE1;          // 初始化类型为作用于墓碑1的吞噬者
}

IceShroom::IceShroom() {


	// 仅做对象初始化，不处理鼠标/图形/全局容器/睡眠等副作用
	type = ICESHROOM;                     // 植物类型常量（保留与 global.h 定义一致）
	HP = 6;                               // 可被啃食的生命（和其他 Plant 保持一致）
	frameNo = 0;                          // 动画起始帧
	No = ++plantNum;                      // 分配唯一编号（与其他植物一致）
	changeFrameCountDown = 8;             // 与 paintPlantsAndGraves 中对 ICESHROOM 的处理一致
	// 保持类成员默认的冻结/减速倒计时值（若需要可以显式设定）
	// frozenCountDown = 200;
	// slowingCountDown = 1000;
	x = 0;
	y = 0;


}