#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

#define HEIGHT 20
#define WIDTH 60

# include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "贪吃蛇");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show(window);

    gtk_main();
    return 0;
}

struct BODY{
    int x;
    int y;
};
struct SNAKE{
    struct BODY body[HEIGHT*WIDTH];
    int size;
}snake;

struct FOOD{
    int x;
    int y;
}food;

int score = 0;
int lastx = 0, lasty = 0;
int sleepSecond = 200000;

void initSnake(){
    snake.size = 2;
    snake.body[0].x = WIDTH/2;
    snake.body[0].y = HEIGHT/2;
    snake.body[1].x = snake.body[0].x - 1;
    snake.body[1].y = snake.body[0].y/2;
}

void initFood(){
    srand(time(NULL));
    do {
        food.x = rand() % (WIDTH-2) + 1;  // 修正1：限制食物生成在有效区域
        food.y = rand() % (HEIGHT-2) + 1;
    } while (food.x == snake.body[0].x && food.y == snake.body[0].y); // 简单碰撞检测
}

void SetCursorPosition(int x, int y) {
    printf("\033[%d;%dH", y+1, x+1);
}
void initUI() {
    // 先清除旧蛇尾
    SetCursorPosition(lastx, lasty);
    putchar(' ');

    // 再绘制新蛇身
    for(int i = 0; i < snake.size; i++) {
        SetCursorPosition(snake.body[i].x, snake.body[i].y);
        if(i == 0) putchar('@');
        else putchar('●');
    }

    // 绘制食物
    SetCursorPosition(food.x, food.y);
    putchar('●');
}

void initWall(){
    for(int i = 0; i <= HEIGHT; i++){
        for(int j = 0; j <= WIDTH; j++){
            if(j == 0 || j == WIDTH || i == 0 || i == HEIGHT){
                printf("●");
            }
            else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

int kbhit(void) {
    fd_set set;
    struct timeval timeout = {0, 0};
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    return select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) == 1;
}

char getch(void) {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

void playGame() {
    char dir = 'd';
    while (snake.body[0].x >= 1 && snake.body[0].x < WIDTH-1 &&
           snake.body[0].y >= 1 && snake.body[0].y < HEIGHT-1) {

        lastx = snake.body[snake.size-1].x;
        lasty = snake.body[snake.size-1].y;

        if(kbhit()) {
            char new_dir = getch();
            // 防止180度急转弯
            if((new_dir == 'w' && dir != 's') ||
               (new_dir == 's' && dir != 'w') ||
               (new_dir == 'a' && dir != 'd') ||
               (new_dir == 'd' && dir != 'a')) {
                dir = new_dir;
            }
        }

        // 更新蛇头位置
        switch(dir) {
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
        }

        // 更新蛇身位置
        for(int i = snake.size-1; i > 0; i--) {
            snake.body[i].x = snake.body[i-1].x;
            snake.body[i].y = snake.body[i-1].y;
        }
        // 自撞检测
        for(int i = 1; i < snake.size; i++){
            if(snake.body[0].x == snake.body[i].x &&
               snake.body[0].y == snake.body[i].y)
                return;
        }

        // 吃食物检测
        if(snake.body[0].x == food.x && snake.body[0].y == food.y){
            snake.size++;
            initFood();
            score += 10;
            if(sleepSecond > 50000) sleepSecond -= 10000;
        }

        printf("\033[2J");
        initWall();
        initUI();
        usleep(sleepSecond);
    }
}

void showScore() {
    SetCursorPosition(0, HEIGHT+2);
    printf("Game Over!!!\n成绩为: %d\n\n", score);
}

int main() {
    srand(time(NULL));

    initSnake();
    initFood();

    initWall();
    initUI();

    playGame();
    showScore();
    return 0;
}