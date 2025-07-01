#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gtk/gtk.h>

#define HEIGHT 24
#define WIDTH 32
#define CELL_SIZE 20  // 每个格子的像素大小
#define BORDER 5      // 边框宽度

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
int sleepSecond = 200000;
char direction = 'd';
gboolean game_over = FALSE;

GtkWidget *drawing_area;
GdkRGBA snake_color = {0.0, 1.0, 0.0, 1.0};  // 绿色
GdkRGBA food_color = {1.0, 0.0, 0.0, 1.0};   // 红色
GdkRGBA wall_color = {0.5, 0.5, 0.5, 1.0};   // 灰色
GdkRGBA text_color = {1.0, 1.0, 1.0, 1.0};   // 白色文本

void initSnake(){
    snake.size = 2;
    snake.body[0].x = WIDTH/2;
    snake.body[0].y = HEIGHT/2;
    snake.body[1].x = snake.body[0].x - 1;
    snake.body[1].y = snake.body[0].y;
}

void initFood(){
    srand(time(NULL));
    do {
        int max_distance = 10; // 最大允许距离
        int dx, dy;
        do {
            food.x = rand() % (WIDTH-2) + 1;
            food.y = rand() % (HEIGHT-2) + 1;

            dx = food.x - snake.body[0].x;
            dy = food.y - snake.body[0].y;
        } while (abs(dx) > max_distance || abs(dy) > max_distance);

    } while (food.x == snake.body[0].x && food.y == snake.body[0].y);
}

void draw_cell(cairo_t *cr, int x, int y, GdkRGBA *color) {
    gdk_cairo_set_source_rgba(cr, color);
    cairo_rectangle(cr,
                   (x * CELL_SIZE) + BORDER,
                   (y * CELL_SIZE) + BORDER,
                   CELL_SIZE - 2*BORDER,
                   CELL_SIZE - 2*BORDER);
    cairo_fill(cr);
}

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    // 绘制背景
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    // 绘制墙壁
    gdk_cairo_set_source_rgba(cr, &wall_color);
    for(int i = 0; i <= HEIGHT; i++) {
        for(int j = 0; j <= WIDTH; j++) {
            if(j == 0 || j == WIDTH || i == 0 || i == HEIGHT) {
                cairo_rectangle(cr,
                              j * CELL_SIZE,
                              i * CELL_SIZE,
                              CELL_SIZE,
                              CELL_SIZE);
                cairo_fill(cr);
            }
        }
    }

    // 绘制蛇
    for(int i = 0; i < snake.size; i++) {
        draw_cell(cr, snake.body[i].x, snake.body[i].y,
                i == 0 ? &snake_color : &snake_color);
    }

    // 绘制食物
    draw_cell(cr, food.x, food.y, &food_color);

    if(game_over) {
        // 设置字体
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                              CAIRO_FONT_WEIGHT_NORMAL); // 修改为普通字体粗细
        cairo_set_font_size(cr, 24); // 修改字体大小
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);

        // 计算文本位置（右侧区域）
        int text_x = WIDTH * CELL_SIZE + 20;
        int text_y = height/2 - 20;

        // 绘制"GAME OVER"
        cairo_text_extents_t extents;
        char text[] = "GAME OVER";
        cairo_text_extents(cr, text, &extents);
        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, text);

        // 绘制分数
        char score_text[64];
        snprintf(score_text, sizeof(score_text), "Score: %d", score);
        cairo_text_extents(cr, score_text, &extents);
        text_y += extents.height * 1.5;
        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, score_text);
    }

    return FALSE;
}

gboolean game_loop(gpointer data) {
    if(game_over) return FALSE;

    // 更新蛇的位置
    for(int i = snake.size-1; i > 0; i--) {
        snake.body[i].x = snake.body[i-1].x;
        snake.body[i].y = snake.body[i-1].y;
    }

    switch(direction) {
        case 'w': snake.body[0].y--; break;
        case 's': snake.body[0].y++; break;
        case 'a': snake.body[0].x--; break;
        case 'd': snake.body[0].x++; break;
    }

    // 碰撞检测（墙壁）
    if(snake.body[0].x <= 0 || snake.body[0].x >= WIDTH-1 ||
       snake.body[0].y <= 0 || snake.body[0].y >= HEIGHT-1) {
        game_over = TRUE;
        return FALSE;
    }

    // 自撞检测
    for(int i = 1; i < snake.size; i++) {
        if(snake.body[0].x == snake.body[i].x &&
           snake.body[0].y == snake.body[i].y) {
            game_over = TRUE;
            return FALSE;
        }
    }

    // 吃食物检测
    if(snake.body[0].x == food.x && snake.body[0].y == food.y) {
        snake.size++;
        initFood();
        score += 10;
        if(sleepSecond > 50000) sleepSecond -= 10000;
    }

    gtk_widget_queue_draw(drawing_area);
    return TRUE;
}

static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if(game_over) return TRUE;

    switch(event->keyval) {
        case GDK_KEY_w:
        case GDK_KEY_Up:
            if(direction != 's') direction = 'w';
            break;
        case GDK_KEY_s:
        case GDK_KEY_Down:
            if(direction != 'w') direction = 's';
            break;
        case GDK_KEY_a:
        case GDK_KEY_Left:
            if(direction != 'd') direction = 'a';
            break;
        case GDK_KEY_d:
        case GDK_KEY_Right:
            if(direction != 'a') direction = 'd';
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;

    gtk_init(&argc, &argv);

    int window_width = (WIDTH + 10) * CELL_SIZE;  // 包含右侧信息栏空间
    int window_height = HEIGHT * CELL_SIZE;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "贪吃蛇");
    gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height);

    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_callback), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), NULL);

    gtk_widget_show_all(window);

    srand(time(NULL));
    initSnake();
    initFood();

    g_timeout_add(sleepSecond/1000, game_loop, NULL);

    gtk_main();
    return 0;
}