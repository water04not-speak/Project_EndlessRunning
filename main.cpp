/* endlessRunning 开发日志
* 1.创建项目
* 2.导入素材
* 3.创建游戏界面
* 
* 选择图形库EasyX
* 1）创建游戏窗口
* 2）实现游戏背景
* 三重背景图以不同的速度同时进行移动
* 循环滚动背景图
* 3)实现游戏背景
* a.加载背景资源
* b.渲染背景（可能用到：坐标）
* 遇见问题：背景图片的png格式图片出现黑色
* 
* 4.实现玩家奔跑
* 5.实现玩家的跳跃
* 6.实现随机小乌龟
* 7.创建障碍物结构体数据类型
* 8.使用障碍物结构体后重新初始化
* 9.封装后多个障碍物的显示
* 10.实现玩家的下蹲技能
* 11.实现“柱子障碍物”
*/

#define _CRT_SECURE_NO_WARNINGS  // 放在所有 #include 之前！
#include<stdio.h>
#include<graphics.h>
//EasyX图形库引入，用于实现游戏窗口，图片显示，背景滚动，角色和障碍物渲染
#include<conio.h>
//提供如_kbhit()、_getch()等函数，
//用于检测和获取键盘输入，实现玩家操作（如跳跃、下蹲）。
#include<vector>
//引入C++中的容器vector
#include "tools.h"
//引入开源的一系列准备的工具函数

using namespace std;

#define WINDOW_WIDTH 1012
#define WINDOW_HEIGHT 396
#define OBSTACLE_COUNT 10
#define WIN_SCORE 20

//背景图片
IMAGE imageBgs[3];
int bgX[3]; //背景图片的X坐标
int bgSpeed[3] = { 1,3,6 };//1,2,4

IMAGE imgHeros[12];
int heroX;//玩家的x坐标
int heroY;//玩家的y坐标
int heroIndex;//玩家奔跑的图片帧索引

bool heroJump; //玩家是否跳跃

int jumpHeightMax; //跳跃的最大高度
int heroJumpOff; //玩家跳跃的高度偏移量
bool update;//是否需要马上刷新画面 (用int定义update也行)

//IMAGE imgTortoise;//小乌龟障碍物
//int torToiseX; //小乌龟的x坐标
//int torToiseY; //小乌龟的y坐标
//bool torToiseExist;//当前窗口是否有小乌龟存在

int heroBlood;//玩家总血量
int score;

typedef enum {
	TORTOISE, //小乌龟 0
	LION, //狮子 1
	HOOK1,
	HOOK2,
	HOOK3,
	HOOK4,
	OBSTACLE_TYPE_COUNT //5
}obstacle_type;

vector<vector<IMAGE>>obstacleImgs; //存放所有障碍物的图片资源

typedef struct obstacle {
	int type; //障碍物类型
	int imgIndex; //障碍物的图片索引
	int x, y; //障碍物的坐标
	int speed; //障碍物的移动速度
	int power; //障碍物的威力（碰撞时的伤害）
	bool exist; //障碍物是否存在
	bool hited;//表示是否已经发生碰撞
	bool passed;//表示是否已经被通过
}obstacle_t;

obstacle_t obstacles[OBSTACLE_COUNT]; //障碍物数组
int lastObsIndex;

IMAGE imgHeroDown[2];
bool heroDown;//表示玩家是否处于下蹲状态

IMAGE imgSZ[10];

//游戏的初始化
void init() {
//创建游戏窗口
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
//加载背景资源
	char name[64];
	for (int i = 0; i < 3; ++i) {
		sprintf(name, "res/bg%03d.png", i + 1);
		loadimage(&imageBgs[i],name);

		bgX[i] = 0;
	}
	
	//加载玩家Hero奔跑的图片帧素材
	for (int i = 0; i < 12; ++i) {
		sprintf(name, "res/hero%d.png", i + 1);
		loadimage(&imgHeros[i], name);
	}

	//设置玩家的初始位置
	heroX = WINDOW_WIDTH / 2 - imgHeros[0].getwidth() / 2;
	heroY = 345 - imgHeros[0].getheight(); 
	heroIndex = 0; //玩家奔跑

	heroJump = false; //初始玩家没有跳跃
	jumpHeightMax = 345 - imgHeros[0].getheight() - 120; //跳跃的最大高度，玩家跳跃时的最高点
	heroJumpOff = -4;

	update = true;

	//加载小乌龟素材
	//loadimage(&imgTortoise, "res/t1.png");
	//torToiseExist = false; //初始小乌龟不存在
	//torToiseY = 345 - imgTortoise.getheight()+4; //初始化小乌龟的y坐标
	IMAGE imgTort;
	loadimage(&imgTort, "res/t1.png");
	vector<IMAGE> imgTorArray;
	imgTorArray.push_back(imgTort);
	obstacleImgs.push_back(imgTorArray);

	IMAGE imgLion;
	vector<IMAGE> imgLionArray;
	for (int i = 0; i < 6; ++i) {
		sprintf(name, "res/p%d.png", i + 1);
		loadimage(&imgLion, name);
		imgLionArray.push_back(imgLion);
	}
	obstacleImgs.push_back(imgLionArray);

	//初始化障碍物池
	for (int i = 0; i < OBSTACLE_COUNT; ++i) {
		obstacles[i].exist = false; //初始所有障碍物都不存在
	}

	//加载下蹲素材
	loadimage(&imgHeroDown[0], "res/d1.png");
	loadimage(&imgHeroDown[1], "res/d2.png");
	heroDown = false;

	IMAGE imgH;

	for (int i = 0; i < 4; ++i) {
		vector<IMAGE> imgHookArray;
		sprintf(name, "res/h%d.png", i + 1);
		loadimage(&imgH, name,63,260,true);
		imgHookArray.push_back(imgH);
		obstacleImgs.push_back(imgHookArray);
	}

	heroBlood = 100;

	//预加载音效
	preLoadSound("res/hit.mp3");//解决第一次碰撞时没声音

	//播放背景音乐
	mciSendString("play res/bg.mp3 repeat",0,0,0);

	lastObsIndex = -1;
	score = 0;

	//加载数字图片
	for (int i = 0; i < 10; ++i) {
		sprintf(name, "res/sz/%d.png", i);
		loadimage(&imgSZ[i], name);
	}

}

void createObstacle() {
	int i;
	for (i = 0; i < OBSTACLE_COUNT; ++i) {
		if (obstacles[i].exist == false) {
			break;
		}
	}

	if (i >= OBSTACLE_COUNT) {
		return;
	}

	obstacles[i].exist = true;
	obstacles[i].hited = false;
	obstacles[i].imgIndex = 0;
	//obstacles[i].type = (obstacle_type)(rand() % OBSTACLE_TYPE_COUNT);
	obstacles[i].type= (obstacle_type)(rand() % 3);

	if (lastObsIndex >= 0 &&
		obstacles[lastObsIndex].type >= HOOK1 &&
		obstacles[lastObsIndex].type <= HOOK4 &&
		obstacles[i].type == LION &&
		obstacles[lastObsIndex].x > (WINDOW_WIDTH - 500)) {
	obstacles[i].type = TORTOISE;
	}
	lastObsIndex = i;

	if (obstacles[i].type == HOOK1) {
		obstacles[i].type += rand() % 4;
	}

	obstacles[i].x = WINDOW_WIDTH;
	obstacles[i].y = 345 + 5 - obstacleImgs[obstacles[i].type][0].getheight();
	if (obstacles[i].type == TORTOISE) {
		obstacles[i].speed = 0;
		obstacles[i].power = 5;//自己修改
	}
	else if (obstacles[i].type == LION) {
		obstacles[i].speed = 4;
		obstacles[i].power = 20;
	}
	else if (obstacles[i].type >=HOOK1 && obstacles[i].type<=HOOK4){
		obstacles[i].speed = 4;
		obstacles[i].power = 20;
		obstacles[i].y = 0;
	}

	obstacles[i].passed = false;

}

void checkHit() {
	for (int i = 0; i < OBSTACLE_COUNT; ++i) {
		if (obstacles[i].exist && obstacles[i].hited==false) {
			int a1x, a1y, a2x, a2y;
			int off = 30;
			if (!heroDown) {//非下蹲（就是奔跑，跳跃）
				a1x = heroX+off;
				a1y = heroY + off;
				a2x = heroX + imgHeros[heroIndex].getwidth() - off;
				a2y = heroY + imgHeros[heroIndex].getheight();
			}
			else {
				a1x = heroX + off;
				a1y = 345 - imgHeroDown[heroIndex].getheight();
				a2x = heroX + imgHeroDown[heroIndex].getwidth();
				a2y = 345;
			}
			IMAGE img = obstacleImgs[obstacles[i].type][obstacles[i].imgIndex];
			int b1x = obstacles[i].x + off;
			int b1y = obstacles[i].y + off;
			int b2x = obstacles[i].x +img.getwidth()-off;
			int b2y = obstacles[i].y +img.getheight()-10;

			if (rectIntersect(a1x, a1y, a2x, a2y, b1x, b1y, b2x, b2y)) {
				heroBlood -= obstacles[i].power;
				printf("血量剩余%d\n", heroBlood);
				playSound("res/hit.mp3");
				obstacles[i].hited = true;
			}

		}
	}
}

//更新背景，角色，障碍物等的运动和碰撞检测等逻辑
void fly() {
	for (int i = 0; i < 3; ++i) {
		bgX[i] -= bgSpeed[i];
		if (bgX[i] < -WINDOW_WIDTH) {
			bgX[i] = 0;
		}
	}



	//实现跳跃
	if (heroJump) {
		if (heroY < jumpHeightMax) {
			heroJumpOff = 4;
		}

		heroY += heroJumpOff;

		if(heroY> 345 - imgHeros[0].getheight()) { //如果玩家回到地面
			heroJump = false; //玩家跳跃结束
			heroJumpOff = -4; //重置跳跃偏移量
		}
	}
	else if (heroDown) {
		static int count = 0;
		int delays[2] = { 7,20 };//下蹲延迟设置
		count++;
		if (count >= delays[heroIndex]) {
			count = 0;
			heroIndex++;
			if (heroIndex >= 2) {
				heroIndex = 0;
				heroDown = false;
			}
		}
	}
	else {//不跳跃
		heroIndex = (heroIndex + 1) % 12; //玩家奔跑的图片帧索引循环
	}


	//创建障碍物
	static int frameCount = 0;
	static int enemyFre = 50; //障碍物出现的频率
	frameCount++;
	if (frameCount > enemyFre) {
		frameCount = 0; //重置帧计数器
		enemyFre = 50 + rand() % 50; //随机生成障碍物出现的频率200~500
		createObstacle();
	}

	/*if (torToiseExist) {
		torToiseX -= bgSpeed[2];
		if (torToiseX < -imgTortoise.getwidth()) {
			torToiseExist = false;
		}
	}*/

	//更新所有障碍物的目标
	for (int i = 0; i < OBSTACLE_COUNT; ++i) {
		if (obstacles[i].exist) {
			obstacles[i].x -= obstacles[i].speed + bgSpeed[2];
			if (obstacles[i].x < -obstacleImgs[obstacles[i].type][0].getwidth() * 2) {
				obstacles[i].exist = false;
			}

			int len = obstacleImgs[obstacles[i].type].size();
			obstacles[i].imgIndex = (obstacles[i].imgIndex + 1) % len;

		}
	}

	//玩家和障碍物的“碰撞检测”处理
	checkHit();

}



//渲染“游戏背景”
void updateBg() {
	putimagePNG2(bgX[0], 0, &imageBgs[0]);
	putimagePNG2(bgX[1], 119, &imageBgs[1]);
	putimagePNG2(bgX[2], 330, &imageBgs[2]);
}

//跳跃
void jump() {
	heroJump = true; //玩家开始跳跃
	update = true; //需要马上刷新画面
}

//下蹲
void down() {
	heroDown = true;
	update = true;
	heroIndex = 0;

}

//处理用户按键的输入
void keyEvent() {
	char ch;
	if (_kbhit()) { //如果有按键按下，_kbhit()返回 true
		ch = _getch(); //获取按下的按键
		if (ch == ' ') {
			jump();
		}
		else if (ch == 'z') {
			down();
		}
	}
}

void updateEnemy() {
	//渲染小乌龟
	//if (torToiseExist) {
	//	putimagePNG2(torToiseX,torToiseY,WINDOW_WIDTH, &imgTortoise);
	//	
	//}

	for (int i = 0; i < OBSTACLE_COUNT; ++i) {
		if (obstacles[i].exist) {
			putimagePNG2(obstacles[i].x, obstacles[i].y, WINDOW_WIDTH,
				&obstacleImgs[obstacles[i].type][obstacles[i].imgIndex]);
		}
	}


}

void updateHero() {
	if (!heroDown) {
		putimagePNG2(heroX, heroY, &imgHeros[heroIndex]);
	}
	else {
		int y = 345 - imgHeroDown[heroIndex].getheight();
			putimagePNG2(heroX, y, &imgHeroDown[heroIndex]);
	}
}

void updateBloodBar() {
	drawBloodBar(10, 10, 200, 10, 2, BLUE, DARKGRAY, RED, heroBlood / 100.0);
}

void checkOver() {
	if (heroBlood <= 0) {
		loadimage(0, "res/over.png");//加载结束界面
		FlushBatchDraw();//刷新
		mciSendString("stop res/bg.mp3", 0, 0, 0);//关闭背景音乐
		system("pause");//暂停游戏

		//暂停之后，可以充币复活，或者直接开始下一局
		heroBlood = 100;
		score = 0;
		mciSendString("play res/bg.mp3 repeat", 0, 0, 0);

	}
}

//显示初始化界面
void initImg() {
	loadimage(0, "res/over.png");
	system("pause");
}

//检查分数
void checkScore() {
	for (int i = 0; i < OBSTACLE_COUNT; ++i) {
		if (obstacles[i].exist &&
			 obstacles[i].passed == false &&
			 obstacles[i].hited==false &&
			 obstacles[i].x + obstacleImgs[obstacles[i].type][0].getwidth() < heroX) {
			score++;
			obstacles[i].passed = true;
			printf("score:%d\n", score);
		}
	}
}

//更新分数
void updateScore() {
	char str[8];
	sprintf(str, "%d", score);

	int x = 20;
	int y = 25;

	for (int i = 0; str[i]; i++) {
		int sz = str[i] - '0';
		putimagePNG(x, y, &imgSZ[sz]);
		x += imgSZ[sz].getwidth()+5;
	}
}

//检查是否胜利
void checkWin() {
	FlushBatchDraw();//刷新
	if (score >= WIN_SCORE) {
		mciSendString("play res/win.mp3", 0, 0, 0);
		Sleep(1000);//暂停1秒
		loadimage(0, "res/win.png");//直接加载窗口
		FlushBatchDraw();
		mciSendString("stop res/bg.mp3", 0, 0, 0);
		system("pause");

		heroBlood = 100;
		score = 0;
		mciSendString("play res/bg.mp3 repeat", 0, 0, 0);

	}
}

int main()
{
	init();//初始化游戏

	initImg();

	int timer = 0;
	while (1) {
		keyEvent();
		timer+=getDelay();
		if (timer > 30) {
			timer = 0;
			update = true; //每隔30毫秒需要刷新画面
		}

		if (update) {
			update = false; //重置刷新标志
			fly();//先更新状态
			BeginBatchDraw();//开始批量绘制，防止屏幕闪烁
			updateBg();//渲染背景
			//putimagePNG2(heroX, heroY, &imaHeros[heroIndex]);
			updateHero();//渲染角色
			updateEnemy();//渲染障碍物
			updateBloodBar();//渲染血条
			updateScore();//渲染分数
			checkWin();//
			EndBatchDraw();//一次性刷新所有绘制的内容到屏幕上

			checkOver();
			checkScore();
			
			
		}


		//Sleep(30);//休眠

	}


	system("pause");
	return 0;
}










