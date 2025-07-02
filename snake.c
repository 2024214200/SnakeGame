#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gtk/gtk.h>
#include <string.h>

// 统一宏定义
#define HEIGHT 24
#define WIDTH 32
#define CELL_SIZE 20
#define BORDER 5
#define MAX_RECORDS 10
#define MAX_LEVELS 5

// 排行榜记录结构
typedef struct {
    char name[20];
    int score;
    int level;
    time_t timestamp;
} LeaderboardRecord;

// 关卡信息结构
typedef struct {
    int height;
    int width;
    int snake_length;
    int speed;       // 速度单位：微秒
    int obstacle_num;// 障碍物数量
} GameLevel;

// 游戏核心结构
struct BODY{ int x, y; };
struct SNAKE{ struct BODY body[HEIGHT*WIDTH]; int size; };
struct FOOD{ int x, y; int type; }; // type: 0-普通 1-加速 2-减速
struct OBSTACLE{ int x, y; };

struct SNAKE snake;
struct FOOD food;

// 游戏状态
struct GameState {
    int level;
    int score;
    int high_score;
    char direction;
    int sleep_ms;
    gboolean game_over;
    gboolean level_complete;
    LeaderboardRecord leaderboard[MAX_RECORDS];
    int leaderboard_count;
    GameLevel levels[MAX_LEVELS];
    struct OBSTACLE obstacles[HEIGHT*WIDTH];
    int obstacle_count;
};

struct GameState game;
GtkWidget *drawing_area;
GtkWidget *info_bar;
GdkRGBA colors[] = {{0,1,0,1}, {1,0,0,1}, {0.5,0.5,0.5,1},
                    {1,1,0,1}, {0.5,0,0.5,1}, {1,1,1,1}};


// 初始化游戏状态
void initGameState() {
    game.level = 1;
    game.score = 0;
    game.direction = 'd';
    game.sleep_ms = 200000; // 200ms
    game.game_over = FALSE;
    game.level_complete = FALSE;
    game.leaderboard_count = 0;
    game.obstacle_count = 0;

    // 初始化关卡数据
    for (int i = 0; i < MAX_LEVELS; i++) {
        game.levels[i].height = HEIGHT - i;
        game.levels[i].width = WIDTH - i;
        game.levels[i].snake_length = 3 + i;
        game.levels[i].speed = 200000 - i * 20000; // 关卡越高速度越快
        game.levels[i].obstacle_num = i * 3;      // 关卡越高障碍物越多
    }

    // 加载排行榜数据（省略文件操作实现）
}

// 初始化蛇
void initSnake() {
    GameLevel *level = &game.levels[game.level-1];
    snake.size = level->snake_length;
    snake.body[0].x = level->width / 2;
    snake.body[0].y = level->height / 2;

    for (int i = 1; i < snake.size; i++) {
        snake.body[i].x = snake.body[i-1].x - 1;
        snake.body[i].y = snake.body[i-1].y;
    }
}

// 初始化食物（带关卡规则）
void initFood() {
    GameLevel *level = &game.levels[game.level-1];
    int max_x = level->width - 2;
    int max_y = level->height - 2;
    int valid;

    do {
        valid = 1;
        food.x = rand() % max_x + 1;
        food.y = rand() % max_y + 1;
        food.type = rand() % 3; // 随机食物类型

        // 检查与蛇身碰撞
        for (int i = 0; i < snake.size; i++) {
            if (food.x == snake.body[i].x && food.y == snake.body[i].y) {
                valid = 0;
                break;
            }
        }

        // 检查与障碍物碰撞
        for (int i = 0; i < game.obstacle_count; i++) {
            if (food.x == game.obstacles[i].x && food.y == game.obstacles[i].y) {
                valid = 0;
                break;
            }
        }
    } while (!valid);
}

// 初始化障碍物（关卡功能）
void initObstacles() {
    GameLevel *level = &game.levels[game.level-1];
    game.obstacle_count = level->obstacle_num;
    int max_x = level->width - 2;
    int max_y = level->height - 2;
    int valid;

    for (int i = 0; i < game.obstacle_count; i++) {
        do {
            valid = 1;
            game.obstacles[i].x = rand() % max_x + 1;
            game.obstacles[i].y = rand() % max_y + 1;

            // 检查与蛇身初始位置碰撞
            for (int j = 0; j < snake.size; j++) {
                if (game.obstacles[i].x == snake.body[j].x &&
                    game.obstacles[i].y == snake.body[j].y) {
                    valid = 0;
                    break;
                }
            }
        } while (!valid);
    }
}

// 绘制游戏元素
gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int win_width = gtk_widget_get_allocated_width(widget);
    int win_height = gtk_widget_get_allocated_height(widget);
    GameLevel *level = &game.levels[game.level-1];

    // 绘制背景
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    // 绘制墙壁
    for (int i = 0; i <= level->height; i++) {
        for (int j = 0; j <= level->width; j++) {
            if (j == 0 || j == level->width || i == 0 || i == level->height) {
                cairo_rectangle(cr, j*CELL_SIZE, i*CELL_SIZE,
                               CELL_SIZE, CELL_SIZE);
                gdk_cairo_set_source_rgba(cr, &colors[2]); // 灰色墙壁
                cairo_fill(cr);
            }
        }
    }

    // 绘制障碍物
    for (int i = 0; i < game.obstacle_count; i++) {
        cairo_rectangle(cr,
            game.obstacles[i].x*CELL_SIZE,
            game.obstacles[i].y*CELL_SIZE,
            CELL_SIZE, CELL_SIZE);
        gdk_cairo_set_source_rgba(cr, &colors[4]); // 紫色障碍物
        cairo_fill(cr);
    }

    // 绘制蛇
    for (int i = 0; i < snake.size; i++) {
        cairo_rectangle(cr,
            snake.body[i].x*CELL_SIZE+BORDER,
            snake.body[i].y*CELL_SIZE+BORDER,
            CELL_SIZE-2*BORDER, CELL_SIZE-2*BORDER);
        gdk_cairo_set_source_rgba(cr, (i==0) ? &colors[0] : &colors[0]); // 绿色蛇身
        cairo_fill(cr);
    }

    // 绘制食物
    cairo_rectangle(cr,
        food.x*CELL_SIZE+BORDER,
        food.y*CELL_SIZE+BORDER,
        CELL_SIZE-2*BORDER, CELL_SIZE-2*BORDER);
    gdk_cairo_set_source_rgba(cr,
        (food.type==0) ? &colors[1] : // 红色普通食物
        (food.type==1) ? &colors[3] : // 黄色加速食物
        &colors[3]);                  // 黄色减速食物
    cairo_fill(cr);

    // 绘制信息栏
    cairo_set_source_rgba(cr, &colors[5]); // 白色文本
    cairo_select_font_face(cr, "Sans",
                          CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);

    // 绘制分数和关卡
    char info[100];
    sprintf(info, "分数: %d | 关卡: %d | 速度: %dms",
            game.score, game.level, game.sleep_ms/1000);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, info, &extents);
    cairo_move_to(cr, level->width*CELL_SIZE + 20, 30);
    cairo_show_text(cr, info);

    // 游戏结束提示
    if (game.game_over) {
        cairo_set_font_size(cr, 32);
        cairo_text_extents(cr, "GAME OVER", &extents);
        cairo_move_to(cr, (win_width-extents.width)/2, win_height/2-30);
        cairo_show_text(cr, "GAME OVER");

        sprintf(info, "最终分数: %d", game.score);
        cairo_text_extents(cr, info, &extents);
        cairo_move_to(cr, (win_width-extents.width)/2, win_height/2+20);
        cairo_show_text(cr, info);
    }

    // 关卡完成提示
    if (game.level_complete) {
        cairo_set_font_size(cr, 32);
        cairo_text_extents(cr, "关卡完成!", &extents);
        cairo_move_to(cr, (win_width-extents.width)/2, win_height/2-30);
        cairo_show_text(cr, "关卡完成!");

        sprintf(info, "进入关卡 %d", game.level+1);
        cairo_text_extents(cr, info, &extents);
        cairo_move_to(cr, (win_width-extents.width)/2, win_height/2+20);
        cairo_show_text(cr, info);
    }

    return FALSE;
}

// 处理键盘输入
gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (game.game_over || game.level_complete) return TRUE;

    switch (event->keyval) {
        case GDK_KEY_w: case GDK_KEY_Up:
            if (game.direction != 's') game.direction = 'w';
            break;
        case GDK_KEY_s: case GDK_KEY_Down:
            if (game.direction != 'w') game.direction = 's';
            break;
        case GDK_KEY_a: case GDK_KEY_Left:
            if (game.direction != 'd') game.direction = 'a';
            break;
        case GDK_KEY_d: case GDK_KEY_Right:
            if (game.direction != 'a') game.direction = 'd';
            break;
        case GDK_KEY_Escape:
            gtk_main_quit();
            break;
    }
    return TRUE;
}

// 游戏主循环
gboolean game_loop(gpointer data) {
    if (game.game_over || game.level_complete) return FALSE;

    GameLevel *level = &game.levels[game.level-1];
    int new_x = snake.body[0].x;
    int new_y = snake.body[0].y;

    // 更新蛇头位置
    switch (game.direction) {
        case 'w': new_y--; break;
        case 's': new_y++; break;
        case 'a': new_x--; break;
        case 'd': new_x++; break;
    }

    // 保存蛇尾位置
    int tail_x = snake.body[snake.size-1].x;
    int tail_y = snake.body[snake.size-1].y;

    // 移动蛇身
    for (int i = snake.size-1; i > 0; i--) {
        snake.body[i].x = snake.body[i-1].x;
        snake.body[i].y = snake.body[i-1].y;
    }
    snake.body[0].x = new_x;
    snake.body[0].y = new_y;

    // 碰撞检测
    // 1. 墙壁碰撞
    if (new_x <= 0 || new_x >= level->width ||
        new_y <= 0 || new_y >= level->height) {
        game.game_over = TRUE;
        gtk_widget_queue_draw(drawing_area);
        return FALSE;
    }

    // 2. 自碰撞
    for (int i = 1; i < snake.size; i++) {
        if (snake.body[0].x == snake.body[i].x &&
            snake.body[0].y == snake.body[i].y) {
            game.game_over = TRUE;
            gtk_widget_queue_draw(drawing_area);
            return FALSE;
        }
    }

        // 3. 障碍物碰撞
    for (int i = 0; i < game.obstacle_count; i++) {
        if (snake.body[0].x == game.obstacles[i].x &&
            snake.body[0].y == game.obstacles[i].y) {
            game.game_over = TRUE;
            gtk_widget_queue_draw(drawing_area);
            return FALSE;
        }
    }

    // 食物检测
    if (snake.body[0].x == food.x && snake.body[0].y == food.y) {
        // 增加蛇长度
        snake.size++;
        if (snake.size > level->width * level->height) {
            // 蛇太长则通关
            game.level_complete = TRUE;
            game.score += 100; // 通关额外加分
            gtk_widget_queue_draw(drawing_area);
            return FALSE;
        }

        // 根据食物类型生效
        if (food.type == 1) { // 加速食物
            if (game.sleep_ms > 50000) game.sleep_ms -= 20000;
        } else if (food.type == 2) { // 减速食物
            if (game.sleep_ms < 200000) game.sleep_ms += 20000;
        }

        // 加分
        game.score += 10 + food.type * 5;
        if (game.score > game.high_score) {
            game.high_score = game.score;
        }

        // 生成新食物
        initFood();
    }

    // 更新显示
    gtk_widget_queue_draw(drawing_area);
    return TRUE;
}

// 处理关卡完成
void handleLevelComplete() {
    if (game.level < MAX_LEVELS) {
        game.level++;
        // 重置游戏状态
        game.game_over = FALSE;
        game.level_complete = FALSE;
        game.sleep_ms = game.levels[game.level-1].speed;

        // 初始化新关卡
        initSnake();
        initFood();
        initObstacles();

        // 重新启动游戏循环
        g_timeout_add(game.sleep_ms/1000, game_loop, NULL);
    } else {
        game.game_over = TRUE;
        // 所有关卡完成，显示最终成绩
        gtk_widget_queue_draw(drawing_area);
    }
}

// 显示排行榜窗口（完整实现）
void showLeaderboard() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "贪吃蛇排行榜");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 450);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *title = gtk_label_new("历史最高分排行榜");
    PangoAttrList *attr = pango_attr_list_new();
    PangoAttribute *size_attr = pango_attr_size_new(PANGO_SCALE_LARGE);
    pango_attr_list_insert(attr, size_attr);
    gtk_label_set_attributes(GTK_LABEL(title), attr);
    pango_attr_list_unref(attr);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 10);

    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

    GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING,
                                           G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
    GtkTreeView *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(tree_view));

    // 添加列
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(tree_view,
                               gtk_tree_view_column_new_with_attributes(
                                   "排名", renderer, "text", 0, NULL));
    gtk_tree_view_append_column(tree_view,
                               gtk_tree_view_column_new_with_attributes(
                                   "玩家", renderer, "text", 1, NULL));
    gtk_tree_view_append_column(tree_view,
                               gtk_tree_view_column_new_with_attributes(
                                   "分数", renderer, "text", 2, NULL));
    gtk_tree_view_append_column(tree_view,
                               gtk_tree_view_column_new_with_attributes(
                                   "关卡", renderer, "text", 3, NULL));
    gtk_tree_view_append_column(tree_view,
                               gtk_tree_view_column_new_with_attributes(
                                   "时间", renderer, "text", 4, NULL));

    // 从文件加载排行榜（定义在全局作用域）
    void loadLeaderboard() {
        FILE *file = fopen("snake_leaderboard.dat", "rb");
        if (file) {
            fread(&game.leaderboard_count, sizeof(int), 1, file);
            if (game.leaderboard_count > MAX_RECORDS) {
                game.leaderboard_count = MAX_RECORDS;
            }
            fread(game.leaderboard, sizeof(LeaderboardRecord),
                  game.leaderboard_count, file);
            fclose(file);
        }
    }

    // 保存排行榜到文件（定义在全局作用域）
    void saveLeaderboard() {
        FILE *file = fopen("snake_leaderboard.dat", "wb");
        if (file) {
            fwrite(&game.leaderboard_count, sizeof(int), 1, file);
            fwrite(game.leaderboard, sizeof(LeaderboardRecord),
                  game.leaderboard_count, file);
            fclose(file);
        }
    }
    // 加载排行榜数据（示例数据，实际应从文件读取）
    GtkTreeIter iter;
    for (int i = 0; i < game.leaderboard_count && i < 10; i++) {
        char time_str[30];
        struct tm *tm_info = localtime(&game.leaderboard[i].timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, i+1,
                          1, game.leaderboard[i].name,
                          2, game.leaderboard[i].score,
                          3, game.leaderboard[i].level,
                          4, time_str,
                          -1);
    }

    GtkWidget *close_btn = gtk_button_new_with_label("关闭");
    g_signal_connect(close_btn, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    gtk_box_pack_start(GTK_BOX(vbox), close_btn, FALSE, FALSE, 10);

    gtk_widget_show_all(window);
}

// 显示关卡选择界面（完整实现）
void showLevelSelection() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "选择游戏关卡");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *title = gtk_label_new("请选择游戏关卡");
    PangoAttrList *attr = pango_attr_list_new();
    PangoAttribute *size_attr = pango_attr_size_new(PANGO_SCALE_LARGE);
    pango_attr_list_insert(attr, size_attr);
    gtk_label_set_attributes(GTK_LABEL(title), attr);
    pango_attr_list_unref(attr);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 10);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    // 创建关卡按钮
    for (int i = 0; i < MAX_LEVELS; i++) {
        char label[20];
        sprintf(label, "关卡 %d", i+1);

        GtkWidget *btn = gtk_button_new_with_label(label);
        g_signal_connect(btn, "clicked", G_CALLBACK(gtk_widget_destroy), window);
        g_signal_connect(btn, "clicked", G_CALLBACK((GCallback)[](GtkWidget* b, gpointer data) {
            int level = GPOINTER_TO_INT(data);
            game.level = level;
            gtk_main_quit();
        }), GINT_TO_POINTER(i+1));

        int col = i % 5;
        int row = i / 5;
        gtk_grid_attach(GTK_GRID(grid), btn, col, row, 1, 1);
    }

    gtk_widget_show_all(window);
}

// 主函数
int main(int argc, char *argv[]) {
    // 初始化GTK
    gtk_init(&argc, &argv);

    // 初始化游戏状态
    initGameState();

    // 创建主窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "贪吃蛇大冒险");
    gtk_window_set_default_size(GTK_WINDOW(window),
                              (WIDTH+5)*CELL_SIZE, HEIGHT*CELL_SIZE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // 创建绘图区域
    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // 连接信号
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_callback), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), NULL);

    // 显示主菜单（简化实现，实际应使用独立菜单界面）
    GtkWidget *menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(drawing_area), menu_box);
    gtk_box_set_center_widget(GTK_BOX(menu_box), menu_box);

    GtkWidget *title = gtk_label_new("贪吃蛇大冒险");
    PangoAttrList *attr = pango_attr_list_new();
    PangoAttribute *size_attr = pango_attr_size_new(PANGO_SCALE_LARGE);
    pango_attr_list_insert(attr, size_attr);
    gtk_label_set_attributes(GTK_LABEL(title), attr);
    pango_attr_list_unref(attr);
    gtk_box_pack_start(GTK_BOX(menu_box), title, FALSE, FALSE, 20);

    GtkWidget *start_btn = gtk_button_new_with_label("开始游戏");
    g_signal_connect(start_btn, "clicked", G_CALLBACK((GCallback)[](GtkWidget* b, gpointer data) {
        showLevelSelection();
    }), NULL);
    gtk_box_pack_start(GTK_BOX(menu_box), start_btn, FALSE, FALSE, 10);

    GtkWidget *leaderboard_btn = gtk_button_new_with_label("查看排行榜");
    g_signal_connect(leaderboard_btn, "clicked", G_CALLBACK(showLeaderboard), NULL);
    gtk_box_pack_start(GTK_BOX(menu_box), leaderboard_btn, FALSE, FALSE, 10);

    GtkWidget *quit_btn = gtk_button_new_with_label("退出游戏");
    g_signal_connect(quit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(menu_box), quit_btn, FALSE, FALSE, 10);

    gtk_widget_show_all(window);

    // 进入GTK主循环
    gtk_main();

    // 保存排行榜数据（程序退出时）
    // 保存排行榜到文件

    return 0;
}
void saveLeaderboard() {
    FILE *file = fopen("snake_leaderboard.dat", "wb");
    if (file) {
        fwrite(&game.leaderboard_count, sizeof(int), 1, file);
        fwrite(game.leaderboard, sizeof(LeaderboardRecord),
               game.leaderboard_count, file);
        fclose(file);
    }
}

// 从文件加载排行榜
void loadLeaderboard() {
    FILE *file = fopen("snake_leaderboard.dat", "rb");
    if (file) {
        fread(&game.leaderboard_count, sizeof(int), 1, file);
        if (game.leaderboard_count > MAX_RECORDS) {
            game.leaderboard_count = MAX_RECORDS;
        }
        fread(game.leaderboard, sizeof(LeaderboardRecord),
              game.leaderboard_count, file);
        fclose(file);
    }
}
// saveLeaderboard(); // 实现文件保存功能
