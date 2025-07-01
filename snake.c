#include <curses.h>
#include <stdlib.h>



#define UP    1
#define DOWN  -1
#define LEFT  2
#define RIGHT -2

struct Snake
{
	int hang,lie;
	struct Snake *next;
};

struct Snake *head = NULL;
struct Snake *tail = NULL;
int key;
int dir;

//定义蛇的食物
struct Snake food;


//初始化食物
void initFood()
{
	int x = rand()%20;	//xrand()生成一个随机数,生成较大的数采用取余，%20取余
	int y = rand()%20;

	food.hang = x;
	food.lie = y;


}

void initNcurse()
{
	initscr();//ncurse界面初始化函数
	keypad(stdscr,1);//从标准的stdscr中接收功能键，1表示接收
	noecho();
}

//封装函数获取节点
int hasSnakeNode(int i,int j)
{
	struct Snake *p;
	p = head;
	while(p != NULL){
		if(p->hang == i && p->lie == j){
			return 1;
		}
		p = p->next;
	}
	return 0;
}

//封装函数获取食物
int hasFood(int i,int j)
{
	if(food.hang == i && food.lie == j){
		return 1;
	}
	return 0;
}

//地图
void gamePic()
{
	int hang,lie;

	//ncurse光标的改变
	//改变光标，使蛇身移动之后覆盖原地图
	move(0,0);

	//第0行的边界
	for(hang = 0;hang < 20;hang++){
		if(hang == 0){
			for(lie = 0;lie < 20;lie++){
				printw("--");
			}
			printw("\n");
		}
	//第0行与第19行的“|”和空格与下边一致
		if(hang >= 0 || hang <= 19){
			for(lie = 0;lie <= 20;lie++){
				if(lie == 0 || lie == 20){
					printw("|");
//在打印空格的地方加入遍历，显示蛇的身体
				}else if(hasSnakeNode(hang,lie)){//函数封装遍历
					printw("[]");
				}else if(hasFood(hang,lie)){	//判断是否有食物，有用##显示
					printw("##");
				}
				else{
					printw("  ");
				}
			}
			printw("\n");
		}
	//第19行的边界
		if(hang == 19){
			for(lie = 0;lie < 20;lie++){
				printw("--");
			}
			printw("\n");
			printw("By laowang,food.hang=%d,food.lie=%d\n",food.hang,food.lie);
		}
	}
}
//添加节点
void addNode()
{
	struct Snake *new = (struct Snake *)malloc(sizeof(struct Snake));

	new->next = NULL;

	switch(dir){
		case UP:
			new->hang = tail->hang-1;
			new->lie = tail->lie;
			break;
		case DOWN:
			new->hang = tail->hang+1;
			new->lie = tail->lie;
			break;
		case LEFT:
			new->hang = tail->hang;
			new->lie = tail->lie-1;
			break;
		case RIGHT:
			new->hang = tail->hang;
			new->lie = tail->lie+1;
			break;
	}

	tail->next = new;
	tail = new;
}
//初始化节点
void initSnake()
{
	struct Snake *p;
	//初始方向
	dir = RIGHT;

	while(head != NULL){	//释放旧的链表
		p = head;
		head = head->next;
		free(p);
	}

	initFood();
	head = (struct Snake *)malloc(sizeof(struct Snake));
	head->hang = 1;
	head->lie = 1;		//头结点的位置
	head->next = NULL;

	tail = head;

	addNode();
	addNode();
	addNode();//4个节点
}

//删除节点
void deleNode()
{
	struct Snake *p;
	p = head;		//p指向头，使删除的节点释放
	head = head->next;

	free(p);
}

//蛇自杀死亡
int ifSnakeDie()
{
	//定义变量，遍历贪吃蛇
	struct Snake *p;
	p = head;

	if(tail->hang < 0 || tail->hang == 20 || tail->lie == 0 || tail->lie == 20){
		return 1;
	}

	while(p->next != NULL){
		if(p->hang == tail->hang && p->lie == tail->lie){
			return 1;
		}
		p = p->next;
	}

	return 0;
}

//蛇身体的移动
void moveSnake()
{
	addNode();
	//判断是否吃到食物，吃到,不删除节点
	if(hasFood(tail->hang,tail->lie)){
		initFood();
	}else{
		deleNode();
	}

//撞墙(链尾撞)（hang=0撞上,hang=20撞下,lie=0撞左,lie=20撞右）
	if(ifSnakeDie()){
		initSnake();//死后重新初始化蛇节点。重新初始化后要释放前节点

	}

}
//界面刷新封装（线程）
void* refreshJiemian()
{
	while(1){
		//自由运动
			moveSnake();
			gamePic();//改变之后需要重新扫描地图才能实现
			//刷新界面
			refresh();
			usleep(100000);//以微秒为单位；sleep(1);以秒为单位

	}
}

//限制蛇身体直上直下的转向
void turn(int direction)
{
	if(abs(dir) != abs(direction)){	//abs:取绝对值
		dir = direction;
	}
}

//方向改变封装（线程）
void* changeDir()
{
	//int key;
	while(1){
		key = getch();
		switch(key){
			case KEY_DOWN:
				//dir = DOWN;
				turn(DOWN);
				break;
			case KEY_UP:
				//dir = UP;
				turn(UP);
				break;
			case KEY_LEFT:
				//dir = LEFT;
				turn(LEFT);
				break;
			case KEY_RIGHT:
				//dir = RIGHT;
				turn(RIGHT);
				break;
		}
	}
}

int main()
{
	//两个while(1)的实现需要用到线程
	pthread_t t1;
	pthread_t t2;

	initNcurse();

	initSnake();

	gamePic();

	pthread_create(&t1,NULL,refreshJiemian,NULL);
	pthread_create(&t2,NULL,changeDir,NULL);

	while(1);

	getch();//等待用户输入，如果没这句话程序就退出了，看不到运行结果.
	endwin();//程序退出，调用改函数来恢复shell终端显示，如果没这句话，shell终端字乱码，坏掉
	return 0;
 }
//
// Created by yj186 on 25-7-1.
//
