# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <time.h>
# include <unistd.h>
# include <fcntl.h>
# include <termios.h>
# include <sys/select.h>

# define HEIGHT 20
# define WIDTH 60

struct BODY{
    int x;
    int y;
};
struct SNAKE{
    struct BODY body[HEIGHT*WIDTH];//body[0]就是蛇头
    int size;
}snake;

struct FOOD{
	int x;
	int y;
}food;

int score = 0;//分数显示
int lastx = 0, lasty = 0;
int sleepSecond = 400;

void initSnake(){
	snake.size = 2;  //蛇大小是两节

    snake.body[0].x = WIDTH/2;
    snake.body[0].y = HEIGHT/2;   //初始化蛇头
    snake.body[1].x = WIDTH/2 - 1;
    snake.body[1].y = HEIGHT/2;   //初始化一节蛇身
}

void initFood(){
    srand(time(NULL));//播种随机数种子

    food.x = rand()%WIDTH;//0-59
    food.y = rand()%HEIGHT; //0-19
}

//修改：使用ANSI转义序列定位光标,不能直接使用SetConsoleCursorPosition()函数
void SetCursorPosition(int x, int y) {
    printf("\033[%d;%dH", y+1, x+1);
}

void initUI() {
    for(int i = 0; i < snake.size; i++) {
        SetCursorPosition(snake.body[i].x,snake.body[i].y);
        if(i == 0){
            putchar('@');
        }
        else{
            putchar('#');
        }

        //去除蛇尾，相当于在蛇尾画一个空格
        SetCursorPosition(lastx,lasty);
        putchar(' ');
        //画食物
        SetCursorPosition(food.x, food.y);
        putchar('#');
    }
}

//todo 优化蛇、食物和墙的形状
void initWall(){
    for(int i =0; i <= HEIGHT; i++){
        for(int j =0; j <= WIDTH; j++){
            if(j == WIDTH || j == 0){
                printf("|");
            }
            else if(i == HEIGHT || i == 0){
                printf("_");
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
    }
}

void playGame() {
    char ch = 'd';//初始移动方向是向右
    //判断蛇撞墙
    while (snake.body[0].x > 0 && snake.body[0].x < WIDTH
           && snake.body[0].y > 0 && snake.body[0].y < HEIGHT)
    {
        initUI();//每次都要重新画蛇
        usleep(sleepSecond * 1000);

        //todo 接收用户按键输入，阻塞问题及用户输入的判断尚未解决...
        if (kbhit()) {
            char ch = getchar();
            switch (ch) {
                case 'w':
                    snake.body[0].y--;
                    break;
                case 's':
                    snake.body[0].y++;
                    break;
                case 'a':
                    snake.body[0].x--;
                    break;
                case 'd':
                    snake.body[0].x++;
                    break;
                default:
                    break;
            }//蛇头移动方向
        }
        //存储蛇尾的坐标
        lastx = snake.body[snake.size -1].x;
        lasty = snake.body[snake.size -1].y;

        for(int i =snake.size-1; i > 0; i--){
            snake.body[i].x = snake.body[i-1].x;
            snake.body[i].y = snake.body[i-1].y;
        }//蛇身前一节给后一节赋值

        //蛇头撞身体
        for(int i = 1; i < snake.size; i++){
            if(snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y)
            {
                return;
            }
        }
        //蛇头撞食物
        if(snake.body[0].x == food.x && snake.body[0].y == food.y)
        {
            snake.size++;  //身体增长
            initFood();    //食物消失，初始化新的食物
            score += 10;    //加分
            if (sleepSecond > 50) {
                sleepSecond -= 5;
            }//吃到食物之后加速，且设置最低临界速度
        }
    }
}

//todo 将成绩存入文件，输出排行榜
void showScore() {
    //将光标默认位置移动至不干扰游戏位置
    SetCursorPosition(0,HEIGHT + 2);
    printf("Game Over!!!\n");
    printf("成绩为: %d\n\n\n",score);
}

int main() {
    srand((time(NULL)));

    initSnake();
    initFood();

    initWall();
    initUI();

    playGame();
    showScore();
    //todo 设置多个关卡，让用户选择是否进行下一关
    system("pause");
    return EXIT_SUCCESS;
}


