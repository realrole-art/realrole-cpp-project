#include "global.h"
#include "plant.h"
#include "zombie.h"
#pragma warning (disable:4996)
#pragma comment( lib, "MSIMG32.LIB")
#pragma comment( lib, "winmm.lib")
#include <random>
#include <filesystem>
#include <stdexcept>

// 启用std::filesystem命名空间
namespace fs = std::filesystem;

// 游戏参数
int mapState[32][32];		// 地图状态。0：空，1：墓碑，2：地雷（没出土），3：地雷（已出土），4：寒冰菇
int currentSunshine;
LinkList<Sun> suns;
LinkList<Plant> plants;
LinkList<Zombie> zombies;
LinkList<Bang> bangs;
Lawnmower* lawnmowers[5];
double normalfrequency;
double coneheadfrequency;
double bucketheadfrequency;
double SunsFrequency = 0.005;
int isNewGame;
int isHitting;
int hammerRadius;
int drawingHint;   
int hintCountDown;
int snowCountDown;
int graveNum;
int Win1Lose2;

/*
* 函数功能：初始化游戏所有核心参数、地图状态和动态元素链表，为新游戏/读档做准备
* 实现思路：1. 初始化5行9列游戏实际地图为草地状态；2. 置零当前阳光数，清空所有动态元素链表（植物、僵尸、阳光、爆炸）；
* 3. 为5行除草机分配内存并初始化对象；4. 初始化僵尸/阳光生成概率、锤击状态、提示文字、雪花特效、墓碑数量、胜负标识等所有游戏全局参数为默认值。
*/
void init()
{
	//恢复草地状态
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			mapState[i][j] = GRASS;
		}
	}

	// 阳光置零
	currentSunshine = 0;

	//清空元素链表
	plants.DeleteAllNode();
	zombies.DeleteAllNode();
	suns.DeleteAllNode();
	bangs.DeleteAllNode();

	//除草机分配内存
	for (int i = 0; i < 5; i++)
	{
		if (lawnmowers[i] != nullptr)
		{
			delete lawnmowers[i];
		}
		lawnmowers[i] = new Lawnmower;
	}

	//分配全局参数为默认值
	normalfrequency = 0.001;
	coneheadfrequency = 0.0005;
	bucketheadfrequency = 0.0005;
	SunsFrequency = 0.005;
	isNewGame = 1;
	isHitting = 0;
	hammerRadius = 0;
	drawingHint = 70;
	snowCountDown = 0;
	graveNum = 0;//提前完成后面的生成随机数的函数，但使用了<random>
	Win1Lose2 = 0;
}

/*
* 函数功能：获取指定文件夹下的所有文件名称，存入全局vector容器files中
* 实现思路：1. 清空files容器避免残留数据；2. 使用C语言_findfirst/_findnext遍历指定路径下的所有文件，将文件名依次存入files；
* 3. 遍历结束后关闭文件句柄，删除files中前两个默认的.和..目录名，仅保留实际文件名称。
*/
void getFiles(string path)
{
	files.clear(); //Clear the global container before each load to prevent duplicate appending
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			files.push_back(entry.path().filename().string());
		}
	}
}
/*
* 函数功能：获取指定文件夹下的所有文件数量
* 实现思路：遍历指定文件夹路径下的文件或目录，若是文件则计数加一，返回最终计数
*/
int getFileNumInFolder(string filePath) {
	int count = 0;
	for (const auto& entry : std::filesystem::directory_iterator(filePath))
	{
		if (entry.is_regular_file())
		{
			count++;
		}
	}
	return count;
}

/*
* 函数功能：预处理存档文件夹，若文件夹中存档数量大于五，则自动删除文件夹第一个到第五个文件之后的所有文件（目的是为了防止人为手动向文件夹目录中添加文件时导致的错误）、
* 实现思路：遍历指定文件夹下的文件并计数，删除多余的文件
*/
int preprocessArchives(const std::wstring& dir_path)
{
	// 1. 校验目录是否存在（抛异常替代错误码）
	if (!fs::exists(dir_path))
	{
		throw std::invalid_argument("目录不存在：" + fs::path(dir_path).string());
	}
	if (!fs::is_directory(dir_path))
	{
		throw std::invalid_argument("路径不是目录：" + fs::path(dir_path).string());
	}
	// 2. 遍历目录，收集所有普通文件（排除文件夹、符号链接等）
	std::vector<fs::path> normal_files; // C++容器存储文件路径，自动管理内存
	for (const auto& entry : fs::directory_iterator(dir_path))
	{
		// 筛选：仅保留普通文件（is_regular_file是C++标准判断普通文件的方式）
		if (fs::is_regular_file(entry.status()))
		{
			normal_files.push_back(entry.path()); // 自动拼接完整路径，无需手动拼接
		}
	}

	// 3. 计数并删除第6个及以后的文件
	size_t deleted_count = 0;
	for (size_t i = 5; i < normal_files.size(); ++i)
	{
		// 前5个文件跳过，从第6个（索引5）开始删除
			try
			{
				// C++标准删除文件：fs::remove（返回bool表示是否成功）
				if (fs::remove(normal_files[i]))
				{
					std::wcout << L"成功删除：" << normal_files[i].wstring() << std::endl;
					++deleted_count;
				}
				else
				{
					std::wcerr << L"删除失败（文件不存在/权限不足）：" << normal_files[i].wstring() << std::endl;
				}
			}
			// 捕获文件系统异常（如权限不足、文件被占用）
			catch (const fs::filesystem_error& e)
			{
				std::cerr << "删除异常：" << e.what() << "，错误码：" << e.code() << std::endl;
			}
	}

	// 输出统计信息（C++风格流输出）
	std::wcout << L"\n===== 执行结果 =====" << std::endl;
	std::wcout << L"目录下普通文件总数：" << normal_files.size() << std::endl;
	std::wcout << L"保留前5个文件，成功删除后续文件数量：" << deleted_count << std::endl;

	return deleted_count;
}

/*
* 函数功能：从指定存档文件中以二进制形式读取游戏数据，恢复游戏当前状态
* 实现思路：1. 先调用init初始化基础参数，再拼接存档文件路径并以二进制读模式打开；
* 2. 依次读取地图状态、5行除草机的所有属性、游戏核心数值参数（阳光、生成概率、各种状态标识等）；
* 3. 以分隔符1234567为结束标志，循环读取僵尸数据并创建僵尸对象加入僵尸链表；
* 4. 以分隔符7654321为结束标志，根据植物类型循环读取植物数据，创建对应植物对象加入植物链表；
* 5. 以分隔符357421为结束标志，循环读取爆炸特效数据并创建对象加入爆炸链表；
* 6. 循环读取剩余的阳光数据并创建阳光对象加入阳光链表，最后关闭文件句柄。
*/
void readArchive(char name[])
{
	init();
	char path[] = "./archives/", tmppath[200] = { 0 };
	strcat(strcat(tmppath, path), name);
	FILE* fp = fopen(tmppath, "rb");
	::fread(&mapState, sizeof(mapState), 1, fp);

	for (int i = 0; i < 5; i++)
	{
		lawnmowers[i] = new Lawnmower();
		::fread(&lawnmowers[i]->location, sizeof(int), 1, fp);
		::fread(&lawnmowers[i]->isActivated, sizeof(int), 1, fp);
		int tempIsOut;
		::fread(&tempIsOut, sizeof(int), 1, fp);
		// 兼容旧存档：如果isOut=1，表示除草机已经使用，将isActivated设为2
		if (tempIsOut == 1) {
			lawnmowers[i]->isActivated = 2;
		}
	}

	fread(&currentSunshine, sizeof(int), 1, fp);
	fread(&normalfrequency, sizeof(double), 1, fp);
	fread(&coneheadfrequency, sizeof(double), 1, fp);
	fread(&bucketheadfrequency, sizeof(double), 1, fp);
	fread(&SunsFrequency, sizeof(double), 1, fp);
	fread(&isNewGame, sizeof(int), 1, fp);
	fread(&isHitting, sizeof(int), 1, fp);
	fread(&hammerRadius, sizeof(int), 1, fp);
	fread(&drawingHint, sizeof(int), 1, fp);
	fread(&hintCountDown, sizeof(int), 1, fp);
	fread(&snowCountDown, sizeof(int), 1, fp);
	fread(&graveNum, sizeof(int), 1, fp);
	fread(&Win1Lose2, sizeof(int), 1, fp);

	int separator;
	while (1)
	{
		fread(&separator, sizeof(int), 1, fp);
		if (separator != 1234567)
		{
			Zombie* tmpZombie = new Zombie();
			fseek(fp, -(int)sizeof(int), SEEK_CUR);
			fread(&tmpZombie->HP, sizeof(int), 1, fp);
			fread(&tmpZombie->row, sizeof(int), 1, fp);
			fread(&tmpZombie->location, sizeof(int), 1, fp);
			fread(&tmpZombie->emerge1walk2eat3, sizeof(int), 1, fp);
			fread(&tmpZombie->frameNo, sizeof(int), 1, fp);
			fread(&tmpZombie->height, sizeof(int), 1, fp);
			fread(&tmpZombie->No, sizeof(int), 1, fp);
			fread(&tmpZombie->changeFrameCountDown, sizeof(int), 1, fp);
			fread(&tmpZombie->isFrozen, sizeof(int), 1, fp);
			fread(&tmpZombie->isSlowed, sizeof(int), 1, fp);
			fread(&tmpZombie->type, sizeof(int), 1, fp);
			zombies.InsertNode(tmpZombie);
		}
		else break;
	}

	int tmpPlantType;
	while (1)
	{
		fread(&separator, sizeof(int), 1, fp);
		if (separator != 7654321)
		{
			fseek(fp, -(int)sizeof(int), SEEK_CUR);
			fread(&tmpPlantType, sizeof(int), 1, fp);
			switch (tmpPlantType)
			{
			case POTATOMINE:
			{
				PotatoMine* tmpPotatoMine = new PotatoMine();
				tmpPotatoMine->type = tmpPlantType;
				fread(&tmpPotatoMine->frameNo, sizeof(int), 1, fp);
				fread(&tmpPotatoMine->No, sizeof(int), 1, fp);
				fread(&tmpPotatoMine->x, sizeof(int), 1, fp);
				fread(&tmpPotatoMine->y, sizeof(int), 1, fp);
				fread(&tmpPotatoMine->changeFrameCountDown, sizeof(int), 1, fp);
				fread(&tmpPotatoMine->underCountDown, sizeof(int), 1, fp);
				fread(&tmpPotatoMine->boomCountDown, sizeof(int), 1, fp);
				plants.InsertNode(tmpPotatoMine);
				break;
			}
			case GRAVEBUSTER_GRAVE1:
			{
				GraveBuster* tmpGraveBuster = new GraveBuster();
				tmpGraveBuster->type = tmpPlantType;
				fread(&tmpGraveBuster->frameNo, sizeof(int), 1, fp);
				fread(&tmpGraveBuster->No, sizeof(int), 1, fp);
				fread(&tmpGraveBuster->x, sizeof(int), 1, fp);
				fread(&tmpGraveBuster->y, sizeof(int), 1, fp);
				fread(&tmpGraveBuster->changeFrameCountDown, sizeof(int), 1, fp);
				plants.InsertNode(tmpGraveBuster);
				break;
			}
			case ICESHROOM:
			{
				IceShroom* tmpIceShroom = new IceShroom();
				tmpIceShroom->type = tmpPlantType;
				fread(&tmpIceShroom->frameNo, sizeof(int), 1, fp);
				fread(&tmpIceShroom->No, sizeof(int), 1, fp);
				fread(&tmpIceShroom->x, sizeof(int), 1, fp);
				fread(&tmpIceShroom->y, sizeof(int), 1, fp);
				fread(&tmpIceShroom->changeFrameCountDown, sizeof(int), 1, fp);
				fread(&tmpIceShroom->frozenCountDown, sizeof(int), 1, fp);
				fread(&tmpIceShroom->slowingCountDown, sizeof(int), 1, fp);
				plants.InsertNode(tmpIceShroom);
				break;
			}
			}
		}
		else break;
	}

	while (1)
	{
		fread(&separator, sizeof(int), 1, fp);
		if (separator != 357421)
		{
			Bang* tmpBang = new Bang(0, 0);
			fseek(fp, -(int)sizeof(int), SEEK_CUR);
			fread(&tmpBang->No, sizeof(int), 1, fp);
			fread(&tmpBang->x, sizeof(int), 1, fp);
			fread(&tmpBang->y, sizeof(int), 1, fp);
			fread(&tmpBang->countDown, sizeof(int), 1, fp);
			bangs.InsertNode(tmpBang);
		}
		else
			break;
	}

	while (fread(&separator, sizeof(int), 1, fp))
	{
		Sun* tmpSun = new Sun(0, 0);
		fread(&tmpSun->x, sizeof(int), 1, fp);
		fread(&tmpSun->y, sizeof(int), 1, fp);
		fread(&tmpSun->frame, sizeof(int), 1, fp);
		fread(&tmpSun->No, sizeof(int), 1, fp);
		fread(&tmpSun->changeFrameCountDown, sizeof(int), 1, fp);
		fread(&tmpSun->goToCount, sizeof(int), 1, fp);
		fread(&tmpSun->goToCountFrame, sizeof(int), 1, fp);
		fread(&tmpSun->tempX, sizeof(int), 1, fp);
		fread(&tmpSun->tempY, sizeof(int), 1, fp);
		suns.InsertNode(tmpSun);
	}
	fclose(fp);
}

/*
* 函数功能：将当前游戏所有状态数据以二进制形式写入指定存档文件，实现游戏存档
* 实现思路：1. 拼接存档文件路径并以二进制写模式打开；2. 依次写入地图状态、5行除草机属性、游戏核心数值参数；
* 3. 遍历僵尸链表，将所有僵尸对象的属性依次写入文件；4. 写入僵尸数据结束分隔符1234567；
* 5. 遍历植物链表，根据植物类型向下转型后写入对应属性；6. 写入植物数据结束分隔符7654321；
* 7. 遍历爆炸特效链表，写入所有爆炸对象属性；8. 写入爆炸数据结束分隔符357421；
* 9. 遍历阳光链表，写入所有阳光对象属性，最后关闭文件句柄。
*/
void writeArchive(char name[])
{
	string username(name);
	string filepath = "./archives/" + username;
	FILE* fp = fopen(filepath.c_str(), "wb");

	// 写入地图状态（二进制格式）
	fwrite(&mapState, sizeof(mapState), 1, fp);

	// 写入除草机状态
	for (int i = 0; i < 5; i++) {
		fwrite(&lawnmowers[i]->location, sizeof(int), 1, fp);
		fwrite(&lawnmowers[i]->isActivated, sizeof(int), 1, fp);
		// 兼容旧格式：isOut=1表示已使用（isActivated=2）
		int tempIsOut = (lawnmowers[i]->isActivated == 2) ? 1 : 0;
		fwrite(&tempIsOut, sizeof(int), 1, fp);
	}

	// 写入游戏核心参数（二进制格式）
	fwrite(&currentSunshine, sizeof(int), 1, fp);
	fwrite(&normalfrequency, sizeof(double), 1, fp);
	fwrite(&coneheadfrequency, sizeof(double), 1, fp);
	fwrite(&bucketheadfrequency, sizeof(double), 1, fp);
	fwrite(&SunsFrequency, sizeof(double), 1, fp);
	fwrite(&isNewGame, sizeof(int), 1, fp);
	fwrite(&isHitting, sizeof(int), 1, fp);
	fwrite(&hammerRadius, sizeof(int), 1, fp);
	fwrite(&drawingHint, sizeof(int), 1, fp);
	fwrite(&hintCountDown, sizeof(int), 1, fp);
	fwrite(&snowCountDown, sizeof(int), 1, fp);
	fwrite(&graveNum, sizeof(int), 1, fp);
	fwrite(&Win1Lose2, sizeof(int), 1, fp);

	// 遍历僵尸链表，将所有僵尸对象的属性依次写入文件（二进制格式） 
	Node<Zombie>* curZombie = zombies.head;
	while (curZombie != nullptr) {
		Zombie* curNode = curZombie->content;
		fwrite(&curNode->HP, sizeof(int), 1, fp);
		fwrite(&curNode->row, sizeof(int), 1, fp);
		fwrite(&curNode->location, sizeof(int), 1, fp);
		fwrite(&curNode->emerge1walk2eat3, sizeof(int), 1, fp);
		fwrite(&curNode->frameNo, sizeof(int), 1, fp);
		fwrite(&curNode->height, sizeof(int), 1, fp);
		fwrite(&curNode->No, sizeof(int), 1, fp);
		fwrite(&curNode->changeFrameCountDown, sizeof(int), 1, fp);
		fwrite(&curNode->isFrozen, sizeof(int), 1, fp);
		fwrite(&curNode->isSlowed, sizeof(int), 1, fp);
		fwrite(&curNode->type, sizeof(int), 1, fp);
		curZombie = curZombie->next;
	}

	int separator1 = 1234567;
	fwrite(&separator1, sizeof(int), 1, fp);

	// 遍历植物链表，根据植物类型向下转型后写入对应属性（二进制格式）
	Node<Plant>* curPlant = plants.head;
	while (curPlant != nullptr) {
		Plant* curNode = curPlant->content;
		fwrite(&curNode->type, sizeof(int), 1, fp);
		fwrite(&curNode->frameNo, sizeof(int), 1, fp);
		fwrite(&curNode->No, sizeof(int), 1, fp);
		fwrite(&curNode->x, sizeof(int), 1, fp);
		fwrite(&curNode->y, sizeof(int), 1, fp);
		fwrite(&curNode->changeFrameCountDown, sizeof(int), 1, fp);

		// 根据植物类型写入额外属性
		if (curNode->type == POTATOMINE) {
			PotatoMine* potato = static_cast<PotatoMine*>(curNode);
			fwrite(&potato->underCountDown, sizeof(int), 1, fp);
			fwrite(&potato->boomCountDown, sizeof(int), 1, fp);
		} else if (curNode->type == ICESHROOM) {
			IceShroom* ice = static_cast<IceShroom*>(curNode);
			fwrite(&ice->frozenCountDown, sizeof(int), 1, fp);
			fwrite(&ice->slowingCountDown, sizeof(int), 1, fp);
		}
		curPlant = curPlant->next;
	}

	int separator2 = 7654321;
	fwrite(&separator2, sizeof(int), 1, fp);

	// 遍历爆炸特效链表，写入所有爆炸对象属性（二进制格式）
	Node<Bang>* curBang = bangs.head;
	while (curBang != nullptr) {
		Bang* curNode = curBang->content;
		fwrite(&curNode->No, sizeof(int), 1, fp);
		fwrite(&curNode->x, sizeof(int), 1, fp);
		fwrite(&curNode->y, sizeof(int), 1, fp);
		fwrite(&curNode->countDown, sizeof(int), 1, fp);
		curBang = curBang->next;
	}

	int separator3 = 357421;
	fwrite(&separator3, sizeof(int), 1, fp);

	// 遍历阳光链表，写入所有阳光对象属性（二进制格式）
	Node<Sun>* curSun = suns.head;
	while (curSun != nullptr) {
		Sun* curNode = curSun->content;
		fwrite(&curNode->x, sizeof(int), 1, fp);
		fwrite(&curNode->y, sizeof(int), 1, fp);
		fwrite(&curNode->frame, sizeof(int), 1, fp);
		fwrite(&curNode->No, sizeof(int), 1, fp);
		fwrite(&curNode->changeFrameCountDown, sizeof(int), 1, fp);
		fwrite(&curNode->goToCount, sizeof(int), 1, fp);
		fwrite(&curNode->goToCountFrame, sizeof(int), 1, fp);
		fwrite(&curNode->tempX, sizeof(int), 1, fp);
		fwrite(&curNode->tempY, sizeof(int), 1, fp);
		curSun = curSun->next;
	}

	fclose(fp);
}

/*
* 函数功能：实现毫秒级精确延时，精度±1ms，同时降低CPU占用率
* 实现思路：1. 使用静态变量记录上一次的时钟tick值，避免每次调用重置；2. 根据传入的延时毫秒数计算目标tick值；
* 3. 若当前时钟已超过目标tick则直接更新静态变量并返回，否则循环调用Sleep(1)释放CPU控制权，直到达到目标tick值。
*/
void HpSleep(int ms)
{
	static clock_t lastTick = 0;//记录上一次时钟tick值
	clock_t targetTick = lastTick + ms;//计算目标tick值
	clock_t currentTick = clock();//获取当前时钟tick值
	if (currentTick >= targetTick)//若当前时钟已超过目标tick则直接返回
	{
		lastTick = currentTick;
		return;
	}
	while (clock() < targetTick)//循环等待直到达到目标tick值
	{
		Sleep(1);
	}
	lastTick = clock();//更新上一次时钟tick值
}

/*
* 函数功能：实现EasyX中图像的透明绘制，解决普通putimage绘制带白色背景的问题
* 实现思路：1. 获取目标图像和源图像的设备上下文句柄（HDC）；2. 获取源图像的宽高；
* 3. 初始化Windows混合模式结构体BLENDFUNCTION，设置为Alpha通道混合模式；
* 4. 调用Windows API AlphaBlend实现透明贴图，将源图像绘制到目标图像指定坐标。
*/
void transparentImage(IMAGE* dstimg, int x, int y, IMAGE* srcimg)//dsgrggrea
{
	HDC dstDC = GetImageHDC(dstimg);
	HDC srcDC = GetImageHDC(srcimg);
	int w = srcimg->getwidth();
	int h = srcimg->getheight();
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	AlphaBlend(dstDC, x, y, w, h, srcDC, 0, 0, w, h, bf);
}

/*
* 函数功能：绘制游戏地图中的所有植物和墓碑，同时处理植物的核心逻辑（土豆雷倒计时、墓碑吞噬者动画、寒冰菇特效）
* 实现思路：1. 双层循环遍历5行9列的游戏地图，根据mapState的格子状态执行对应操作；
* 2. 若为墓碑则调用透明绘制函数绘制对应墓碑图像；
* 3. 若为土豆雷则分埋地/就绪/爆炸状态，处理倒计时、出土动画、爆炸特效，倒计时结束后更新地图状态并播放音效；
* 4. 若为墓碑吞噬者则先绘制对应墓碑，再绘制吞噬动画，动画结束后删除植物并将格子恢复为草地，同时根据剩余墓碑数量调整游戏概率；
* 5. 若为寒冰菇则绘制冰冻特效动画，动画结束后触发全屏僵尸冰冻效果，播放音效并删除寒冰菇。
*/
void paintPlantsAndGraves()
{
	// 画植物和墓碑
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			switch (mapState[i][j])
			{
			case GRASS:
				break;
			case GRAVE1:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[0]);
				break;
			}
			case GRAVE2:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[1]);
				break;
			}
			case GRAVE3:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[2]);
				break;
			}
			case GRAVE4:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[3]);
				break;
			}
			case GRAVE5:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[4]);
				break;
			}
			case GRAVE6:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[5]);
				break;
			}
			case GRAVE7:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[6]);
				break;
			}
			case GRAVE8:
			{
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[7]);
				break;
			}
			case POTATO:
			{
				transparentImage(NULL, xys[i][j].x, xys[i][j].y + 37, &potato);
				Node<Plant>* cur = plants.head;
				while (cur != NULL)
				{
					if (cur->content->x == i && cur->content->y == j)
					{
						break;
					}
					else cur = cur->next;
				}
				if (cur != NULL)
				{
					PotatoMine* potato = static_cast<PotatoMine*>(cur->content);
					potato->underCountDown--;
					if (potato->underCountDown == 0)
					{
						mapState[i][j] = POTATOMINE;
						mciSendString("play ./Music/dirt_rise.mp3 from 0 ", 0, 0, 0);
					}
				}
				break;
			}
			case POTATOMINE:
			{
				Node<Plant>* cur = plants.head;
				while (cur != NULL)
				{
					if (cur->content->x == i && cur->content->y == j)break;
					else cur = cur->next;
				}
				if (cur != NULL)
				{
					transparentImage(NULL, xys[i][j].x, xys[i][j].y + 40, &potatoMinePictures[cur->content->frameNo]);
					cur->content->changeFrameCountDown--;
					if (cur->content->changeFrameCountDown == 0)
					{
						cur->content->changeFrameCountDown = 20;
						cur->content->frameNo++;
						if (cur->content->frameNo == 7)
						{
							cur->content->frameNo = 0;
						}
					}
				}
				break;
			}
			case POTATOBOOM:
			{
				transparentImage(NULL, xys[i][j].x - 25, xys[i][j].y + 20, &potatoBoom);
				Node<Plant>* cur = plants.head;
				while (cur != NULL)
				{
					if (cur->content->x == i && cur->content->y == j)break;
					else cur = cur->next;
				}

				if (cur != NULL)
				{
					PotatoMine* potato = static_cast<PotatoMine*>(cur->content);
					potato->boomCountDown--;
					if (potato->boomCountDown == 0)
					{
						plants.DeleteNode(potato->No);
						mapState[i][j] = GRASS;
					}
				}

				break;
			}
			case GRAVEBUSTER_GRAVE1:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[0]);
				goto label;
			case GRAVEBUSTER_GRAVE2:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[1]);
				goto label;
			case GRAVEBUSTER_GRAVE3:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[2]);
				goto label;
			case GRAVEBUSTER_GRAVE4:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[3]);
				goto label;
			case GRAVEBUSTER_GRAVE5:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[4]);
				goto label;
			case GRAVEBUSTER_GRAVE6:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[5]);
				goto label;
			case GRAVEBUSTER_GRAVE7:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[6]);
				goto label;
			case GRAVEBUSTER_GRAVE8:
				transparentImage(NULL, xys[i][j].x - 5, xys[i][j].y, &grave[7]);
				goto label;
				{
				label:
					Node<Plant>* cur = plants.head;
					while (cur != NULL)
					{
						if (cur->content->x == i && cur->content->y == j)break;
						else cur = cur->next;
					}
					if (cur != NULL)
					{
						transparentImage(NULL, xys[i][j].x - 10, xys[i][j].y - 10, &gravebusterPictures[cur->content->frameNo]);
						cur->content->changeFrameCountDown--;
						if (cur->content->changeFrameCountDown == 0)
						{
							cur->content->changeFrameCountDown = 10;
							cur->content->frameNo++;
							if (cur->content->frameNo > 27)
							{
								plants.DeleteNode(cur->content->No);
								mapState[i][j] = GRASS;
								graveNum--;
								if (graveNum == 5)
								{
									SunsFrequency = 0.2;
									normalfrequency = 0.003;
									coneheadfrequency = 0.0035;
									bucketheadfrequency = 0.0038;
								}
								else if (graveNum == 3)
								{
									SunsFrequency = 0.4;
									normalfrequency = 0.006;
									coneheadfrequency = 0.0065;
									bucketheadfrequency = 0.0068;
								}
							}
						}
					}
					break;
				}
			case ICESHROOM:
			{
				Node<Plant>* cur = plants.head;
				while (cur != NULL)
				{
					if (cur->content->x == i && cur->content->y == j)break;
					else cur = cur->next;
				}
				if (cur != NULL)
				{
					transparentImage(NULL, xys[i][j].x, xys[i][j].y + 15, &iceshroomPictures[cur->content->frameNo]);
					cur->content->changeFrameCountDown--;
					if (cur->content->changeFrameCountDown == 0)
					{
						cur->content->changeFrameCountDown = 8;
						cur->content->frameNo++;
						if (cur->content->frameNo > 10)
						{
							plants.DeleteNode(cur->content->No);
							mciSendString("play ./Music/shoop.mp3 from 0 ", 0, 0, 0);
							mapState[i][j] = 0;
							snowCountDown = 20;
							// 冻结所有僵尸
							Node<Zombie>* zomCur = zombies.head;
							while (zomCur != NULL)
							{
								zomCur->content->isFrozen = 200;
								zomCur = zomCur->next;
							}
						}
					}
				}
				break;
			}
			}
		}
	}
}

/*
* 函数功能：为僵尸图像添加冰冻视觉效果，通过修改像素RGB值实现蓝色调冰冻效果
* 实现思路：1. 获取源图像的宽高，调整目标图像尺寸与源图像一致；2. 获取源图像和目标图像的像素缓冲区指针；
* 3. 遍历所有像素，提取源像素的R/G/B/Alpha通道值；4. 为B通道增加指定数值（绿色/红色可选），使图像偏蓝色，实现冰冻效果；
* 5. 限制RGB值不超过255，将处理后的像素值写入目标图像缓冲区，保留原Alpha通道实现透明。
*/
void addIce(IMAGE* targetImage, IMAGE* srcImage, int addRed = 0, int addGreen = 0, int addBlue = 50)
{
	int srcImgWidth = srcImage->getwidth(), srcImgHeight = srcImage->getheight();
	targetImage->Resize(srcImgWidth, srcImgHeight);
	DWORD* pTargetBuffer = GetImageBuffer(targetImage);
	DWORD* pSrcBuffer = GetImageBuffer(srcImage);
	int allPixel = srcImgHeight * srcImgWidth;

#define RGBA(r, g, b, a) ((b) + (g << 8) + (r << 16) + (a << 24))
	for (int i = 0; i < allPixel; ++i)
	{
		UCHAR r = (UCHAR)GetRValue(pSrcBuffer[i]);
		UCHAR g = (UCHAR)GetGValue(pSrcBuffer[i]);
		UCHAR b = (UCHAR)GetBValue(pSrcBuffer[i]);
		r = r + addRed;
		r = r > 255 ? 255 : r;
		g = g + addGreen;
		g = g > 255 ? 255 : g;
		b = b + addBlue;
		b = b > 255 ? 255 : b;
		pTargetBuffer[i] = (DWORD)RGBA(r, g, b, pSrcBuffer[i] >> 24);
	}
}

/*
* 函数功能：在指定坐标位置随机生成阳光，有概率一次性生成3个阳光对象
* 实现思路：1. 生成0-1之间的随机数，与阳光生成概率SunsFrequency比较；2. 若随机数小于概率，则循环3次创建阳光对象；
* 3. 为每个阳光对象分配随机偏移坐标，避免重叠，最后将阳光对象加入阳光链表。
*/
void generateSunshine(int x, int y)
{
	if ((double)rand() / RAND_MAX < SunsFrequency) //随机数小于SunsFrequency，则生成三个阳光
	{
		for (int i = 0; i < 3; i++)
		{
			Sun* s = new Sun(x + rand() % 40 - 20, y + rand() % 40 - 20);
			suns.InsertNode(s);
		}
	}
	else  //否则生成一个阳光
	{
		Sun* s = new Sun(x, y);
		suns.InsertNode(s);
	}
}

/*
* 函数功能：绘制游戏中所有僵尸，同时处理僵尸的核心逻辑（行为状态切换、动画播放、冰冻/减速效果、移动、除草机触发）
* 实现思路：1. 遍历僵尸链表，先移除坐标超出左边界的僵尸；2. 根据僵尸的行为状态（出土/行走/啃食）选择对应动画帧绘制；
* 3. 处理僵尸冰冻/减速状态：冰冻时绘制蓝色冰冻图像且禁止移动，减速时绘制冰冻图像且降低移动/动画速度；
* 4. 非冰冻减速状态下正常播放行走动画并让僵尸向左移动；5. 递减冰冻/减速倒计时，冰冻结束后切换为减速状态；
* 6. 判断僵尸是否触发除草机，若触发则激活除草机并播放音效；7. 若除草机已出动，移除对应行的僵尸并生成阳光；
* （注：啃食植物逻辑已注释，暂未实现）
*/
void paintZombies()
{
	Node<Zombie>* cur = zombies.head;
	while (cur != NULL)
	{
		// 移除超出左边界的僵尸
		if (cur->content->location < -150)
		{
			Node<Zombie>* toDelete = cur;
			cur = cur->next;
			zombies.DeleteNode(toDelete->content->No);
			continue; //跳出循环，不执行后面的绘制
		}

		int frameCount = 0;
		if (cur->content->emerge1walk2eat3 == 1)
		{
			// 出土状态
			if (cur->content->type == NORMALZOMBIE)
			{
				tmpImg = normalZombieEmergePictures[cur->content->frameNo];
			}
			else if (cur->content->type == CONEHEADZOMBIE)
			{
				tmpImg = coneheadZombieEmergePictures[cur->content->frameNo];
			}
			else if (cur->content->type == BUCKETHEADZOMBIE)
			{
				tmpImg = bucketheadZombieEmergePictures[cur->content->frameNo];
			}
			cur->content->emergeFrameCount++;
			if (cur->content->emergeFrameCount >= 20) {
				cur->content->emerge1walk2eat3 = 2; //出土动画播放完毕，切换为行走状态
				cur->content->emergeFrameCount = 0; //重置动画播放次数为0，开始行走动画
			}
			frameCount = 20;
		}
		else if (cur->content->emerge1walk2eat3 == 2)
		{
			// 行走状态
			if (cur->content->type == NORMALZOMBIE)
			{
				tmpImg = normalZombieWalkPictures[cur->content->frameNo];
			}
			else if (cur->content->type == CONEHEADZOMBIE)
			{
				tmpImg = coneheadZombieWalkPictures[cur->content->frameNo];
			}
			else if (cur->content->type == BUCKETHEADZOMBIE)
			{
				tmpImg = bucketheadZombieWalkPictures[cur->content->frameNo];
			}
			frameCount = 47;
		}

		//这个时候已经把僵尸的行为定义好了，就需要判断这个时候是否冰冻/减速

		// 处理冰冻/减速状态
		if (cur->content->isFrozen > 0)
		{
			// 冰冻状态：绘制蓝色冰冻图像，禁止移动
			addIce(&tmpImg2, &tmpImg);
			//对于僵尸的y轴坐标，xys[cur->content->row][0].y用于求出某一行第一格的底部y轴坐标，然后加上cur->content->height，使得
			transparentImage(NULL, cur->content->location, xys[cur->content->row][0].y - 40 + 10, &tmpImg2);
		}

		else if (cur->content->isSlowed > 0)
		{
			// 减速状态：绘制冰冻图像，降低移动/动画速度
			addIce(&tmpImg2, &tmpImg, 0, 0, 0);
			transparentImage(NULL, cur->content->location, xys[cur->content->row][0].y - 40 + 10, &tmpImg2);

			// 减速状态下，每20帧切换一次动画（正常是10帧）
			cur->content->changeFrameCountDown--;
			if (cur->content->changeFrameCountDown <= 0)
			{
				cur->content->changeFrameCountDown = 20;
				cur->content->frameNo++;
				if (cur->content->frameNo >= frameCount)
					cur->content->frameNo = 0;
			}
			if (cur->content->emerge1walk2eat3 == 2) {
				// 减速状态下，每5帧移动1像素（正常是每1帧移动1像素）
				
				cur->content->slowMoveCounter++;
				if (cur->content->slowMoveCounter >= 5)
				{
					cur->content->location -= 1;
					cur->content->slowMoveCounter = 0;
				}
			}
			cur->content->isSlowed--;
		}

		else
		{
			// 正常状态：绘制原色图像，正常移动
			transparentImage(NULL, cur->content->location, xys[cur->content->row][0].y - 40 + 10, &tmpImg);

			// 正常播放行走动画
			cur->content->changeFrameCountDown--;
			if (cur->content->changeFrameCountDown <= 0)
			{
				cur->content->changeFrameCountDown = 10;
				cur->content->frameNo++;
				if (cur->content->frameNo >= frameCount)
					cur->content->frameNo = 0;
			}

			// 正常移动
			if (cur->content->emerge1walk2eat3 == 2)
				cur->content->location -= 1;
		}

		// 递减冰冻倒计时，冰冻结束后切换为减速状态
		if (cur->content->isFrozen > 0)
		{
			cur->content->isFrozen--;
			if (cur->content->isFrozen == 0)
			{
				cur->content->isSlowed = 200;
			}
		}

		// 判断是否触发除草机
		if (cur->content->location <= xys[cur->content->row][0].x - 50 && !lawnmowers[cur->content->row]->isActivated)
		{
			lawnmowers[cur->content->row]->isActivated = 1;
		}

		// 除草机出动：清除被除草机碰撞的僵尸并生成阳光
		if (lawnmowers[cur->content->row]->isActivated == 1 && lawnmowers[cur->content->row]->location > -100)
		{
			// 只有当僵尸位置在除草机附近时才清除（除草机移动过程中清除遇到的僵尸）
			if (cur->content->location <= lawnmowers[cur->content->row]->location + 50 && cur->content->location >= lawnmowers[cur->content->row]->location - 30)
			{
				generateSunshine(cur->content->location, xys[cur->content->row][0].y);
				// 删除该僵尸
				Node<Zombie>* toDelete = cur;
				cur = cur->next;
				zombies.DeleteNode(toDelete->content->No);
				continue;
			}
		}

		cur = cur->next;
	}

}

/*
* 函数功能：绘制游戏中所有阳光，同时处理阳光的动画播放和收集逻辑
* 实现思路：1. 遍历阳光链表，调用透明绘制函数绘制当前阳光动画帧；2. 递减帧切换倒计时，倒计时结束后切换下一帧，实现阳光旋转漂浮动画；
* 3. 若阳光被玩家点击（goToCount=1），则根据移动帧计数计算坐标，让阳光向屏幕左上角阳光栏移动；4. 阳光移动至阳光栏后，从链表中删除阳光对象并增加玩家阳光数；
* 5. 动画帧达到最大值后重置为0，实现循环动画。
*/
void paintSuns()
{
	// 目标位置（阳光栏）——阳光飞向并被计入的屏幕坐标
	const int targetX = 60;  // 阳光栏 x
	const int targetY = 20;  // 阳光栏 y
	const int moveFrames = 20; // 飞行动画持续帧数（插值步数）

	// 遍历 suns 链表，逐个绘制并处理交互/收集
	Node<Sun>* cur = suns.head;
	while (cur != NULL)
	{
		Sun* s = cur->content;
		Node<Sun>* next = cur->next; // 保存下一结点，因当前结点可能被删除

		// --- 已被点击：让阳光飞向阳光栏并在到达后完成收集 ---
		if (s->goToCount == 1)
		{
			// 首帧记录起始坐标，用于做帧间插值
			if (s->goToCountFrame == 0)
			{
				s->tempX = s->x; // 起始像素x
				s->tempY = s->y; // 起始像素y
			}

			// 递增飞行帧计数
			s->goToCountFrame++;
			int f = s->goToCountFrame;
			if (f > moveFrames) f = moveFrames;

			// 线性插值计算当前绘制位置：start + (target-start) * (f/moveFrames)
			int drawX = s->tempX + (targetX - s->tempX) * f / moveFrames;
			int drawY = s->tempY + (targetY - s->tempY) * f / moveFrames;

			// 以阳光图中心对齐绘制（资源假定约 60x60，故偏移 30）
			transparentImage(NULL, drawX - 30, drawY - 30, &sunPictures[s->frame]);

			// 到达目标后：增加阳光、更新显示字符串、播放音效并删除该阳光对象
			if (s->goToCountFrame >= moveFrames)
			{
				currentSunshine += 25; // 收集数值（可按需调整）
				sprintf(sunshineNum, "%d", currentSunshine);
				// 播放收集音效（确保已在 loading() 中 open）
				mciSendString("play ./Music/sunshine.mp3", 0, 0, 0);
				// 从链表中删除并释放内存
				suns.DeleteNode(s->No);
			}
		}
		else
		{
			// --- 未被点击：正常在场景中绘制并播放自转动画 ---
			// 使用当前帧索引绘制（假设 sunPictures 长度为 22）
			transparentImage(NULL, s->x - 30, s->y - 30, &sunPictures[s->frame]);

			// 动画帧切换倒计时：每 changeFrameCountDown 帧切换一次
			if (s->changeFrameCountDown <= 0)
			{
				s->changeFrameCountDown = 5; // 重置为 5 帧
				s->frame = (s->frame + 1) % 22; // 循环帧索引
			}
			else
			{
				s->changeFrameCountDown--;
			}

			// 检测鼠标点击：如果玩家在阳光范围内点击，则触发飞向阳光栏的逻辑
			// 这里使用当前捕获的 mousemsg（在主循环中通过 getmessage 更新）
			if (mousemsg.message == WM_LBUTTONDOWN)
			{
				int mx = mousemsg.x;
				int my = mousemsg.y;
				// 点击判定区域：以阳光中心 +/-30 像素为可点击区域
				if (mx >= s->x - 30 && mx <= s->x + 30 && my >= s->y - 30 && my <= s->y + 30)
				{
					s->goToCount = 1;        // 标记为已点击（进入飞行状态）
					s->goToCountFrame = 0;   // 飞行帧计数重置
					s->tempX = s->x;         // 记录飞行起点
					s->tempY = s->y;
				}
			}
		}

		// 移动到下一个结点（注意：当前结点可能已被删除，上面的 next 保护了遍历）
		cur = next;
	}
}

/*
* 函数功能：绘制游戏中所有爆炸特效，同时处理特效的持续时长，倒计时结束后移除特效
* 实现思路：1. 遍历爆炸特效链表，对倒计时大于0的特效绘制爆炸图像并递减倒计时；2. 记录当前结点为前驱结点，遍历至下一个结点；
* 3. 若前驱结点的特效倒计时小于等于0，从链表中删除该爆炸特效对象，释放内存。
*/
void paintBangs()
{
	Node<Bang>* cur = bangs.head;
	Node<Bang>* pre;
	while (cur != NULL)
	{
		cur->content->countDown--;
		transparentImage(NULL, cur->content->x, cur->content->y, &bang);
		pre = cur;
		cur = cur->next;
		if (pre->content->countDown == 0)
		{
			bangs.DeleteNode(pre->content->No);
		}
	}

}

/*
* 函数功能：根据当前鼠标光标类型（cursor）和锤击状态，绘制对应的鼠标光标图像，实现光标跟随效果
* 实现思路：1. 获取鼠标当前坐标，根据光标类型绘制对应图像，图像坐标偏移以让鼠标指针对准图像中心；2. 若为锤子光标，未锤击时绘制默认锤子帧；
* 3. 若处于锤击状态（isHitting=1），按hammerRadius依次绘制锤子旋转动画帧，实现锤击动画；4. 锤子动画帧播放完毕后，重置锤击状态和帧序号；
* 5. 若为植物光标，直接绘制对应植物的初始动画帧。
*/
void paintCursor()
{
	if (cursor == Chammer)
	{
		// 如果没锤，画正常角度锤子
		if (!isHitting)
			transparentImage(NULL, mousemsg.x - 45, mousemsg.y - 45, &hammer[0]);
		else
		{
			// 画旋转锤子
			transparentImage(NULL, mousemsg.x - 45, mousemsg.y - 45, &hammer[hammerRadius]);
			hammerRadius++;
			if (hammerRadius == 13)
			{
				hammerRadius = 0;
				isHitting = 0;
			}
		}
	}
	else if (cursor == CpotatoMine)
		transparentImage(NULL, mousemsg.x - 45, mousemsg.y - 45, &potatoMinePictures[0]);
	else if (cursor == Ciceshroom)
		transparentImage(NULL, mousemsg.x - 45, mousemsg.y - 45, &iceshroomPictures[0]);
	else
		transparentImage(NULL, mousemsg.x - 45, mousemsg.y - 45, &gravebusterPictures[0]);
}

/*
* 函数功能：在地图的墓碑格子上随机生成不同类型的僵尸，同时随机播放僵尸呻吟声音效
* 实现思路：1. 双层循环遍历地图中3-8列的格子（仅墓碑格子生成僵尸）；2. 生成0-1之间的随机数，根据不同概率阈值依次生成普通/路障/铁桶僵尸；
* 3. 创建对应僵尸对象，设置僵尸所在行和初始坐标，加入僵尸链表并播放僵尸出土音效；4. 再次生成随机数，与僵尸呻吟概率比较，随机播放6种呻吟声中的一种。
*/
void randomZombies()
{
	// 随机产生僵尸
	for (int i = 0; i < 5; i++)
	{
		for (int j = 3; j < 9; j++)
		{
			if (1 <= mapState[i][j] && mapState[i][j] <= 8)
			{
				double p = rand() / (double)RAND_MAX;
				if (p < normalfrequency)
				{
					NormalZombie* normalZombie = new NormalZombie();
					normalZombie->row = i;
					normalZombie->location = xys[i][j].x - 75;
					zombies.InsertNode(normalZombie);
					mciSendString("play ./Music/dirt_rise.mp3 from 0", 0, 0, 0);
				}
				else if (normalfrequency <= p && p < coneheadfrequency)
				{
					ConeheadZombie* coneheadZombie = new ConeheadZombie();
					coneheadZombie->row = i;
					coneheadZombie->location = xys[i][j].x - 75;
					zombies.InsertNode(coneheadZombie);
					mciSendString("play ./Music/dirt_rise.mp3 from 0", 0, 0, 0);
				}
				else if (coneheadfrequency <= p && p < bucketheadfrequency)
				{
					BucketheadZombie* bucketheadZombie = new BucketheadZombie();
					bucketheadZombie->row = i;
					bucketheadZombie->location = xys[i][j].x - 75;
					zombies.InsertNode(bucketheadZombie);
					mciSendString("play ./Music/dirt_rise.mp3 from 0", 0, 0, 0);
				}
			}
		}
	}

	// 随机呻吟声
	double p = rand() / (double)RAND_MAX;
	if (p < groanFrequency)
	{
		int px = rand() % 6 + 1;
		switch (px)
		{
		case 1:
			mciSendString("play ./Music/groan.mp3 from 0", 0, 0, 0);
			break;
		case 2:
			mciSendString("play ./Music/groan2.mp3 from 0", 0, 0, 0);
			break;
		case 3:
			mciSendString("play ./Music/groan3.mp3 from 0", 0, 0, 0);
			break;
		case 4:
			mciSendString("play ./Music/groan4.mp3 from 0", 0, 0, 0);
			break;
		case 5:
			mciSendString("play ./Music/groan5.mp3 from 0", 0, 0, 0);
			break;
		case 6:
			mciSendString("play ./Music/groan6.mp3 from 0", 0, 0, 0);
			break;
		}
	}
}

/*
* 函数功能：在存档选择界面绘制archives文件夹下的所有存档名称，处理存档过多、无存档、正常存档三种场景
* 实现思路：1. 调用getFiles获取存档文件夹下的文件名；2. 设置文字背景透明和文字颜色，适配界面风格；
* 3. 若存档数量超过5个，绘制存档过多的提示文字；4. 若无存档，绘制无存档的提示文字；
* 5. 若有正常存档，循环绘制每个存档名称，按顺序排列在存档选择区域，保证视觉对齐。
*/
void paintNames()
{

	
		// 画出存档名称
		//getFiles("./archives");//Retrieve all files under the archive folder and store their filenames in the global filename container
		RECT rect;
		setbkmode(TRANSPARENT);
		settextcolor(RGB(222, 186, 97));
		if (files.size() > 5)
		{
			settextstyle(20, 0, "华文隶书");
			rect = { 268, 135, 538, 335 };
			drawtext("存档过多，请删除archives", &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			rect = { 268, 175, 538, 375 };
			drawtext("文件夹下的存档并重启!", &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		else if (files.size() == 0)
		{
			settextstyle(40, 0, "华文隶书");
			rect = { 268, 159, 538, 360 };
			drawtext("没有存档!", &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		else
		{
			int h = 189;
			settextstyle(35, 0, "华文隶书");
			for (int i = 0; i < files.size(); ++i)
			{
				rect = { 268, h, 538, h + 40 };
				drawtext(files[i].c_str(), &rect, DT_CENTER);
				h += 40;
			}
		}
	
}

/*
* 函数功能：在游戏界面底部绘制提示文字（阳光不足/此处不能种植物），提示文字持续一段时间后自动消失
* 实现思路：1. 若有提示标识（drawingHint≠0），设置文字颜色和字体，根据标识绘制对应提示文字；2. 递减提示倒计时，倒计时结束后；
* 3. 重置倒计时为默认值并清空提示标识，实现提示文字自动消失。
*/
void drawHint()
{
	if (drawingHint != 0)
	{
		settextcolor(WHITE);
		settextstyle(40, 0, "隶书");
		if (drawingHint == 1)
		{
			drawtext("此处不能种植物！", &rect, DT_CENTER);
		}
		else if (drawingHint == 2)
			drawtext("阳光不足！", &rect, DT_CENTER);

		hintCountDown--;
		if (hintCountDown == 0)
		{
			hintCountDown = 70;
			drawingHint = 0;
		}
	}
}

/*
* 函数功能：为新游戏随机生成墓碑，严格控制墓碑总数在6-13个之间，墓碑仅生成在地图3-8列的草地格子
* 实现思路：1. 循环判断墓碑数量，若不在6-13之间则重置数量并重新生成；2. 遍历5行地图，为每行随机生成0-3个墓碑；
* 3. 为每个墓碑随机选择3-8列的格子，若格子为草地则设置为随机墓碑类型并增加墓碑数；4. 若格子非草地则重新选择列数，避免墓碑重叠。
*/
void randomGraves()
{
    // 重置墓碑数量
    graveNum = 0;
    // 目标墓碑数量：6-13个
    int targetGraveNum = 6 + rand() % 8;
    // 仅遍历5行9列的有效地图
    while (graveNum < targetGraveNum) {
        int i = rand() % 5; // 行：0-4
        int j = 3 + rand() % 6; // 列：0-8// 仅在3-8列（j>=3）的草地格子生成墓碑

		//判断该位置是否可用（即该位置是否只有草地）
        if (mapState[i][j] == GRASS) {
            // 随机墓碑类型（1-8）
            mapState[i][j] = 1 + rand() % 8;
            graveNum++;
        }
    }
}
/*
* 函数功能：游戏主循环函数，整合游戏所有绘图、逻辑计算、鼠标事件处理、胜负判断逻辑，是游戏运行的核心
* 实现思路：1. 若为新游戏则调用randomGraves生成墓碑并初始化光标，播放游戏BGM和僵尸来袭音效；2. 进入无限循环，每次循环执行绘图和逻辑计算；
* 3. 绘图流程：清空画布→绘制背景/植物栏/菜单/阳光数→绘制植物墓碑→绘制僵尸→绘制除草机→绘制提示文字→绘制阳光→绘制爆炸→绘制鼠标光标→绘制雪花特效，最后刷新画布；
* 4. 逻辑计算：处理除草机出动逻辑（激活后向右移动，超出右边界则标记为已出动）；
* 5. 鼠标事件处理：捕获鼠标左键/右键点击，处理菜单存档、植物选择、锤击僵尸/收集阳光、种植植物、右键取消种植等操作；
* 6. 游戏逻辑：判断土豆雷是否被僵尸触发，触发后播放爆炸音效并移除僵尸；随机生成僵尸；判断游戏胜负（墓碑为0且无僵尸则赢，僵尸攻入左边界则输）；
* 7. 胜负判断成立后跳出循环，关闭游戏BGM，结束游戏主循环。
*/
void beginGame()
{
	
		// 如果是新游戏
		if (isNewGame)
		{
			// 随机产生墓碑
			randomGraves();
			isNewGame = 0;
			cursor = Chammer;
		}
		mciSendString("open ./Music/Loonboon.mp3 alias BGM2", 0, 0, 0);
		mciSendString("play BGM2 repeat", 0, 0, 0);
		mciSendString("play ./Music/theZombiesareComing.mp3 from 0", 0, 0, 0);
		while (1)
		{
			// 绘图
			cleardevice();
			// 画背景、植物条、菜单、阳光数
			putimage(0, 0, &background);
			putimage(0, 0, &plantsBar);
			transparentImage(NULL, 685, 0, &menu);
			RECT r = { 8, 63, 68, 85 };		// 12, 62, 68, 84
			settextstyle(20, 0, "微软雅黑", 0, 0, FW_BOLD, false, false, false);
			settextcolor(BLACK);
			drawtext(itoa(currentSunshine, sunshineNum, 10), &r, DT_CENTER);
			// 画植物和墓碑
			paintPlantsAndGraves();
			// 画僵尸
			paintZombies();
			// 画除草机
			for (int i = 0; i < 5; i++)
			{
				if (lawnmowers[i]->isActivated == 0 || (lawnmowers[i]->isActivated == 1 && lawnmowers[i]->location <= 900))
				{
					transparentImage(NULL, lawnmowers[i]->location, xys[i][0].y + 45, &lawnmower);
				}
			}

			drawHint();		// 画提示

			paintSuns();		// 画太阳

			paintBangs();		// 画 bang

			paintCursor();		// 画鼠标

			// 画雪花
			if (snowCountDown > 0)
			{
				snowCountDown--;
				transparentImage(NULL, 0, 0, &snow);
			}
			FlushBatchDraw();

			// 计算
			// 除草机状态
			for (int i = 0; i < 5; i++)
			{
				if (lawnmowers[i]->isActivated == 1)
				{
					lawnmowers[i]->location += 5;
					if (lawnmowers[i]->location > 900)
					{
						// 除草机离开屏幕后，标记为不可用
						lawnmowers[i]->isActivated = 2; // 2表示已经使用完毕
					}
				}
			}

			// 如果点了鼠标
			while (peekmessage(&mousemsg, EM_MOUSE))
			{
				if (mousemsg.message == WM_LBUTTONDOWN)
				{
					if (mousemsg.x > 692 && mousemsg.y > 0 && mousemsg.x < 815 && mousemsg.y < 44)
					{
						// 如果点击了菜单,存档退出
						writeArchive(username);
						goto stopGame;
					}
					if (cursor == Chammer)
					{
						// 如果鼠标是锤子
						// 如果点了土豆雷
						if (mousemsg.x > 86 && mousemsg.y > 10 && mousemsg.x < 133 && mousemsg.y < 79)
						{
							if (currentSunshine >= 25)
								cursor = CpotatoMine;
							else
								drawingHint = 2;
						}
						// 如果点了墓碑吞噬者
						else if (mousemsg.x > 145 && mousemsg.y > 10 && mousemsg.x < 191 && mousemsg.y < 79)
						{
							if (currentSunshine >= 75)
								cursor = Cgravebuster;
							else
								drawingHint = 2;
						}
						// 如果点了寒冰菇
						else if (mousemsg.x > 204 && mousemsg.y > 10 && mousemsg.x < 253 && mousemsg.y < 79)
						{
							if (currentSunshine >= 75)
								cursor = Ciceshroom;
							else
								drawingHint = 2;
						}
						else
						{
							hammerRadius = 0;
							isHitting = 1;
							mciSendString("play ./Music/hit.mp3 from 0", 0, 0, 0);
							Node<Zombie>* cur = zombies.head;
							while (cur != NULL)
							{
								Zombie* zombie = cur->content;
								if (mousemsg.x > zombie->location + 97 && mousemsg.y > xys[zombie->row][0].y - 40
									&& mousemsg.x < zombie->location + 164 && mousemsg.y < xys[zombie->row][0].y + zombie->height)
								{
									// 如果锤到了僵尸，僵尸减血或死亡 
									bangs.InsertNode(new Bang(mousemsg.x - 70, mousemsg.y - 30));
									zombie->HP--;
									if (zombie->HP == 0)
									{
										zombies.DeleteNode(zombie->No);
										generateSunshine(zombie->location, xys[zombie->row][0].y);
									}
									else if (zombie->HP == 1)
										zombie->type = NORMALZOMBIE;

									goto skipLittleWhile;
								}
								cur = cur->next;
							}
							Node<Sun>* curSun = suns.head;
							while (curSun != NULL)
							{
								Sun* sun = curSun->content;
								if (mousemsg.x > sun->x + 20 && mousemsg.y > sun->y - 18 && mousemsg.x < sun->x + 108 && mousemsg.y < sun->y + 80)
								{
									// 如果锤中太阳
									curSun->content->goToCount = 1;
									mciSendString("play ./Music/sunshine.mp3 from 0", 0, 0, 0);
									goto skipLittleWhile;
								}
								curSun = curSun->next;
							}
						}
					}
					// 如果鼠标是植物
					else
					{
						int i, j, isInPlantZone = 0;
						for (i = 0; i < 5; i++)
						{
							for (j = 0; j < 9; j++)
							{
								if (mousemsg.x > xys[i][j].x && mousemsg.y > xys[i][j].y
									&& mousemsg.x < xys[i][j].x + 80 && mousemsg.y < xys[i][j].y + 100)
								{
									isInPlantZone = 1;
									break;
								}
							}
							if (isInPlantZone)break;
						}

						if ((cursor != Cgravebuster && mapState[i][j] != GRASS) || isInPlantZone == 0)
						{
							drawingHint = 1;
							continue;
						}

						switch (cursor)
						{
						case CpotatoMine:
						{
							currentSunshine -= 25;
							mapState[i][j] = POTATO;
							PotatoMine* potatoMine = new PotatoMine();
							potatoMine->x = i;
							potatoMine->y = j;
							plants.InsertNode(potatoMine);
							mciSendString("play ./Music/plant.mp3 from 0", 0, 0, 0);
							break;
						}
						case Ciceshroom:
						{
							currentSunshine -= 75;
							mapState[i][j] = ICESHROOM;
							IceShroom* iceshroom = new IceShroom();
							iceshroom->x = i;
							iceshroom->y = j;
							plants.InsertNode(iceshroom);
							mciSendString("play ./Music/plant.mp3 from 0", 0, 0, 0);
							break;
						}
						case Cgravebuster:
						{
							if (mapState[i][j] < 1 || mapState[i][j]>8)
							{
								drawingHint = 1;
								continue;
							}
							currentSunshine -= 75;
							switch (mapState[i][j])
							{
							case GRAVE1:
								mapState[i][j] = GRAVEBUSTER_GRAVE1;
								break;
							case GRAVE2:
								mapState[i][j] = GRAVEBUSTER_GRAVE2;
								break;
							case GRAVE3:
								mapState[i][j] = GRAVEBUSTER_GRAVE3;
								break;
							case GRAVE4:
								mapState[i][j] = GRAVEBUSTER_GRAVE4;
								break;
							case GRAVE5:
								mapState[i][j] = GRAVEBUSTER_GRAVE5;
								break;
							case GRAVE6:
								mapState[i][j] = GRAVEBUSTER_GRAVE6;
								break;
							case GRAVE7:
								mapState[i][j] = GRAVEBUSTER_GRAVE7;
								break;
							case GRAVE8:
								mapState[i][j] = GRAVEBUSTER_GRAVE8;
								break;
							default:
								continue;
								break;
							}
							GraveBuster* gravebuster = new GraveBuster();
							gravebuster->x = i;
							gravebuster->y = j;
							plants.InsertNode(gravebuster);
							mciSendString("play ./Music/gravebusterchomp.mp3 from 0", 0, 0, 0);
							break;
						}
						}
						cursor = Chammer;
					}
				}
				else if (mousemsg.message == WM_RBUTTONDOWN)
				{
					cursor = Chammer;
				}
			}

			//判断土豆雷是否被触发
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 9; j++)
				{
					if (mapState[i][j] == POTATOMINE)
					{
						Node<Zombie>* curZombie = zombies.head, * pre = NULL;
						while (curZombie != NULL)
						{
							pre = curZombie;
							curZombie = curZombie->next;
							if (pre->content->row == i && pre->content->location > xys[i][j].x - 135 && pre->content->location < xys[i][j].x - 20)
							{
								mapState[i][j] = POTATOBOOM;
								mciSendString("play ./Music/potato_mine.mp3 from 0", 0, 0, 0);
								zombies.DeleteNode(pre->content->No);
							}
						}
					}
				}
			}

			//随机产生僵尸
			randomZombies();

			//判断输赢
			if (graveNum == 0 && zombies.head == NULL)
			{
				Win1Lose2 = 1; //win
				mciSendString("play ./Music/trophy.mp3 from 0", 0, 0, 0);
				goto stopGame;
			}

			//遍历僵尸链表判断是否存在僵尸进入房子
			Node<Zombie>* cur = zombies.head;
			while (cur != NULL)
			{
				if (cur->content->location < -150)
				{
					Win1Lose2 = 2; //lose
					mciSendString("play ./Music/losemusic.mp3 from 0", 0, 0, 0);
					goto stopGame;
				}
				cur = cur->next;
			}

		  skipLittleWhile:
			//延时
			HpSleep(15);
		}
	stopGame:
		mciSendString("close BGM2", 0, 0, 0);
		//goto label3;
	
}

/*
* 函数功能：批量加载游戏中的序列帧图像（如僵尸动画、植物动画），存入指定IMAGE数组，简化重复的图片加载代码
* 实现思路：1. 循环n次，依次拼接图片文件路径（基础路径+帧序号+后缀.png）；2. 调用EasyX的loadimage函数加载图片到IMAGE数组对应下标；
* 3. 帧序号从begin开始，适配不同图片资源的命名规则（部分从0开始，部分从1开始）。
*/
void loadImages(IMAGE imgs[], char path[],int n,int begin)
{
	for (int i = 0; i < n; i++)
	{
		char tmpPath[200], frameNo[4];
		strcpy_s(tmpPath, 200, path);
		strcat(strcat(tmpPath, itoa(i + begin, frameNo, 10)), ".png");
		loadimage(&imgs[i], tmpPath);
	}
}

/*
* 函数功能：游戏初始化加载函数，批量加载所有游戏图像资源，同时打开所有音效文件为后续播放做准备
* 实现思路：1. 调用loadImages批量加载墓碑、锤子、阳光、植物动画、僵尸动画等所有序列帧图像；
* 2. 调用loadimage加载单个静态图像（如爆炸、土豆雷埋地、背景、菜单等）；
* 3. 调用mciSendString依次打开所有游戏音效文件，部分音效设置别名，方便后续播放控制。
*/
void loading()
{
	loadImages(grave, "./graphics/GraveStones/", 8, 1);
	loadImages(hammer, "./graphics/Screen/hammer/hammer", 13, 1);
	loadImages(sunPictures, "./graphics/Plants/Sun/Sun_", 22, 0);
	loadImages(potatoMinePictures, "./graphics/Plants/PotatoMine/PotatoMine/PotatoMine_", 8, 0);
	loadImages(iceshroomPictures, "./graphics/Plants/IceShroom/IceShroom/IceShroom_", 11, 0);
	loadImages(gravebusterPictures, "./graphics/Plants/GraveBuster/GraveBuster-", 28, 1);
	loadImages(normalZombieWalkPictures, "./graphics/Zombies/NormalZombie/Zombie/Zombie-", 47, 1);
	loadImages(coneheadZombieWalkPictures, "./graphics/Zombies/ConeheadZombie/ConeheadZombie/ConeheadZombie-", 47, 1);
	loadImages(bucketheadZombieWalkPictures, "./graphics/Zombies/BucketheadZombie/BucketheadZombie/BucketheadZombie-", 47, 1);
	loadImages(normalZombieEmergePictures, "./graphics/Zombies/NormalZombie/ZombieEmerge/Zombie-", 20, 1);
	loadImages(coneheadZombieEmergePictures, "./graphics/Zombies/ConeheadZombie/ConeheadZombieEmerge/Zombie-", 20, 1);
	loadImages(bucketheadZombieEmergePictures, "./graphics/Zombies/BucketheadZombie/BucketheadZombieEmerge/Zombie-", 20, 1);
	loadImages(normalZombieEatPictures, "./graphics/Zombies/NormalZombie/ZombieAttack/ZombieAttack_", 10, 0);
	loadImages(coneheadZombieEatPictures, "./graphics/Zombies/ConeheadZombie/ConeheadZombieAttack/ConeheadZombieAttack_", 10, 0);
	loadImages(bucketheadZombieEatPictures, "./graphics/Zombies/BucketheadZombie/BucketheadZombieAttack/BucketheadZombieAttack_", 10, 0);

	loadimage(&potatoBoom, "./graphics/Plants/PotatoMine/PotatoMineExplode/PotatoMineExplode_0.png");
	loadimage(&potato, "./graphics/Plants/PotatoMine/PotatoMineInit/PotatoMineInit_0.png");
	loadimage(&plantsBar, "./graphics/Screen/ChooserBackground.png");
	loadimage(&background, "./graphics/Screen/Background.jpg");
	loadimage(&selectID, "./graphics/Screen/selectID.png");
	loadimage(&iceTrap, "./graphics/Plants/IceShroom/IceShroomTrap_0.png");
	loadimage(&snow, "./graphics/Plants/IceShroom/IceShroomSnow_0.png");
	loadimage(&menu, "./graphics/Screen/menu.png");
	loadimage(&lawnmower, "./graphics/Screen/lawnmower.png");
	loadimage(&loseGame, "./graphics/Screen/lose.png");
	loadimage(&winGame, "./graphics/Screen/win.png");
	loadimage(&bang, "./graphics/Screen/bang.png");

	mciSendString("open ./Music/chomp.mp3", 0, 0, 0);
	mciSendString("open ./Music/dirt_rise.mp3", 0, 0, 0);
	mciSendString("open ./Music/gravebusterchomp.mp3", 0, 0, 0);
	mciSendString("open ./Music/groan.mp3", 0, 0, 0);
	mciSendString("open ./Music/groan2.mp3", 0, 0, 0);
	mciSendString("open ./Music/groan3.mp3", 0, 0, 0);
	mciSendString("open ./Music/groan4.mp3", 0, 0, 0);
	mciSendString("open ./Music/groan5.mp3", 0, 0, 0);
	mciSendString("open ./Music/groan6.mp3", 0, 0, 0);
	mciSendString("open ./Music/hit.mp3", 0, 0, 0);
	mciSendString("open ./Music/lawnmower.mp3 alias lawnmower", 0, 0, 0);
	mciSendString("open ./Music/losemusic.mp3", 0, 0, 0);
	mciSendString("open ./Music/plant.mp3", 0, 0, 0);
	mciSendString("open ./Music/potato_mine.mp3", 0, 0, 0);
	mciSendString("open ./Music/shoop.mp3", 0, 0, 0);
	mciSendString("open ./Music/theZombiesareComing.mp3", 0, 0, 0);
	mciSendString("open ./Music/trophy.mp3", 0, 0, 0);
	mciSendString("open ./Music/sunshine.mp3", 0, 0, 0);
}


int main()
{
	srand((unsigned)time(NULL));
	loading();    

	//initialize the mapping of pixel for MapState
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			xys[i][j].x = 40 + j * 82;
			xys[i][j].y = 70 + i * 100;
			//cout << "xys[" << i << "][" << j << "]:" << xys[i][j].x << "," << xys[i][j].y << endl;;
		}
	}
	
	initgraph(820, 600);//create a game window
	BeginBatchDraw();
	preprocessArchives(L"./archives");//preprocess archives to delete redundant files

	//Load the save list once during initialization (To Avoid duplicate loading)
	getFiles("./archives"); // Load only once; refresh only after adding or deleting archives subsequently
	
	//set background music
	labelBGM:
	mciSendString("open ./Music/Cerebrawl.mp3 alias BGM1", 0, 0, 0); //open the music of Homepage
	mciSendString("play BGM1 repeat", 0, 0, 0);//set music loop playback

	label2:  //flow control tags
	cleardevice();

	//the main background in the game
	putimage(0, 0, &background);

	//navigate to different interfaces based on the game state
	if (Win1Lose2 == 0)
	{
		transparentImage(NULL, 177, 35, &selectID);//ID selection interface
		paintNames();
	}
	else if (Win1Lose2 == 1)
	{
		transparentImage(NULL, 230, 140, &winGame);//victory interface
	}
	else if (Win1Lose2 == 2)
	{
		transparentImage(NULL, 230, 140, &loseGame);//failure interface
	}

	FlushBatchDraw();// Refreshes the undisplayed drawing

    
	while (1)
	{
		getmessage(&mousemsg, EM_MOUSE);
		if (mousemsg.message == WM_LBUTTONDOWN)
		{
			cout << mousemsg.x << "," << mousemsg.y << endl;
			if (Win1Lose2 == 0)
			{       //点击 “没有我的名字”
				if (mousemsg.x > 236 && mousemsg.y > 436 && mousemsg.x < 391 && mousemsg.y < 474)
				{
					char s[10];
					int input_len = InputBox(s, 10, "请输入你的姓名：", NULL, NULL, 0, 0, false);// First receive the return value of InputBox

					

					// Proceed only when the input is valid (return value > 0 and content is not empty).
					if (input_len > 0 && strlen(s) > 0)
					{     // Check if the username already exists
						bool nameExists = false;
						for (const auto& fileName : files) {
							if (fileName == s) {
								nameExists = true;
								break;
							}
						}
						if (nameExists) {
							HWND hWnd = GetHWnd();
							MessageBox(hWnd, _T("该用户名已存在！"), _T("提示"), MB_OK | MB_TOPMOST);
							continue;
						}
					    int fileNum = getFileNumInFolder("./archives");//Get the number of archived files
						if (fileNum < 5) {
							init();
							writeArchive(s);
							getFiles("./archives");
							goto label2;
						}
						else
						{

						}
						
					}
					// Otherwise (if the input is canceled or empty), do nothing and return to the main logic
					else
					{
						// Prompt for input
						HWND hWnd = GetHWnd();//Get the EasyX window handle
						MessageBox(hWnd, _T("请输入有效的姓名！"), _T("提示"), MB_OK | MB_TOPMOST);//notice that some of the arguments ensure the prompt window is kept on top
					}
				}   //点击 退出（存档）
				else if (mousemsg.x > 410 && mousemsg.y > 438 && mousemsg.x < 566 && mousemsg.y < 473)
				{
					
					if (strcmp(username, "") != 0)
						writeArchive(username);
					return 0;
				}   //点击 (已经存档的) 用户名字
				else if (mousemsg.x > 268 && mousemsg.y > 190 && mousemsg.x < 538 && mousemsg.y < 385)
				{
					
					if (190 <= mousemsg.y && mousemsg.y < 229)
					{
						if (0 < files.size() && files.size() < 6)
							strcpy(username, (char*)files[0].c_str());
						else continue;
					}
					else if (229 <= mousemsg.y && mousemsg.y < 268)
					{
						if (1 < files.size() && files.size() < 6)
							strcpy(username, (char*)files[1].c_str());
						else continue;
					}
					else if (268 <= mousemsg.y && mousemsg.y < 307)
					{
						if (2 < files.size() && files.size() < 6)
							strcpy(username, (char*)files[2].c_str());
						else continue;
					}
					else if (307 <= mousemsg.y && mousemsg.y < 346)
					{
						if (3 < files.size() && files.size() < 6)
							strcpy(username, (char*)files[3].c_str());
						else continue;
					}
					else if (346 <= mousemsg.y && mousemsg.y < 385)
					{
						if (4 < files.size() && files.size() < 6)
							strcpy(username, (char*)files[4].c_str());
						else continue;
					}
					readArchive(username);
					mciSendString("close BGM1", 0, 0, 0);

					beginGame();

					if (Win1Lose2 == 0)
						goto labelBGM;
					else goto label2;
				}
			}
			else
			{
				if (mousemsg.x > 297 && mousemsg.y > 331 && mousemsg.x < 500 && mousemsg.y < 369)
				{
					init();
					mciSendString("close BGM1", 0, 0, 0);
					beginGame();
					if (Win1Lose2 == 0)
						goto labelBGM;
					else goto label2;
				}
			}
		}

	}
	
	getch();
	EndBatchDraw();
	closegraph();

	return 0;
}

