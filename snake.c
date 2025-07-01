#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define GRID_SIZE 20
#define CELL_SIZE 30
#define MAX_SCORES 5

typedef enum {
    UP, RIGHT, DOWN, LEFT
} Direction;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position *body;
    int length;
    Direction dir;
} Snake;

typedef struct {
    Position pos;
} Food;

typedef struct {
    char name[20];
    int score;
} HighScore;

typedef struct {
    GtkWidget *window;
    GtkWidget *drawing_area;

    Snake snake;
    Food food;

    int score;
    int level;
    int speed;
    bool game_over;
    bool paused;

    HighScore highscores[MAX_SCORES];
    int highscores_count;
} GameState;

void init_game(GameState *game);
void reset_game(GameState *game);
void next_level(GameState *game);
void generate_food(GameState *game);
void update_game(GameState *game);
void draw_game(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);
gboolean game_loop(gpointer data);
void save_highscores(GameState *game);
void load_highscores(GameState *game);
void show_game_over_dialog(GameState *game);

#endif // SNAKE_GAME_H



void init_game(GameState *game) {
    // 初始化蛇
    game->snake.length = 3;
    game->snake.body = malloc(game->snake.length * sizeof(Position));
    for (int i = 0; i < game->snake.length; i++) {
        game->snake.body[i].x = GRID_SIZE / 2 - i;
        game->snake.body[i].y = GRID_SIZE / 2;
    }
    game->snake.dir = RIGHT;

    // 设置初始关卡
    game->level = 1;
    game->speed = 200 - (game->level * 30); // 速度随关卡增加

    // 生成食物
    generate_food(game);

    // 初始化游戏状态
    game->score = 0;
    game->game_over = false;
    game->paused = false;

    // 加载高分记录
    load_highscores(game);
}

void reset_game(GameState *game) {
    free(game->snake.body);
    init_game(game);
}

void next_level(GameState *game) {
    if (game->level < 3) {
        game->level++;
        game->speed = 200 - (game->level * 30);
        free(game->snake.body);

        // 重新初始化蛇
        game->snake.length = 3;
        game->snake.body = malloc(game->snake.length * sizeof(Position));
        for (int i = 0; i < game->snake.length; i++) {
            game->snake.body[i].x = GRID_SIZE / 2 - i;
            game->snake.body[i].y = GRID_SIZE / 2;
        }
        game->snake.dir = RIGHT;

        generate_food(game);
    } else {
        // 通关
        game->game_over = true;
        show_game_over_dialog(game);
    }
}

void generate_food(GameState *game) {
    bool valid;
    do {
        valid = true;
        game->food.pos.x = rand() % GRID_SIZE;
        game->food.pos.y = rand() % GRID_SIZE;

        // 确保食物不会出现在蛇身上
        for (int i = 0; i < game->snake.length; i++) {
            if (game->snake.body[i].x == game->food.pos.x &&
                game->snake.body[i].y == game->food.pos.y) {
                valid = false;
                break;
            }
        }
    } while (!valid);
}

void update_game(GameState *game) {
    if (game->paused || game->game_over) return;

    // 移动蛇
    Position new_head = game->snake.body[0];
    switch (game->snake.dir) {
        case UP:    new_head.y--; break;
        case RIGHT: new_head.x++; break;
        case DOWN:  new_head.y++; break;
        case LEFT:  new_head.x--; break;
    }

    // 检查碰撞
    if (new_head.x < 0 || new_head.x >= GRID_SIZE ||
        new_head.y < 0 || new_head.y >= GRID_SIZE) {
        game->game_over = true;
        show_game_over_dialog(game);
        return;
    }

    for (int i = 0; i < game->snake.length; i++) {
        if (new_head.x == game->snake.body[i].x &&
            new_head.y == game->snake.body[i].y) {
            game->game_over = true;
            show_game_over_dialog(game);
            return;
        }
    }

    // 检查是否吃到食物
    if (new_head.x == game->food.pos.x && new_head.y == game->food.pos.y) {
        // 增加蛇长度
        game->snake.length++;
        game->snake.body = realloc(game->snake.body, game->snake.length * sizeof(Position));

        // 移动身体
        for (int i = game->snake.length - 1; i > 0; i--) {
            game->snake.body[i] = game->snake.body[i - 1];
        }
        game->snake.body[0] = new_head;

        // 增加分数
        game->score += 10 * game->level;

        // 检查是否进入下一关
        if (game->score >= game->level * 50) {
            next_level(game);
        } else {
            generate_food(game);
        }
    } else {
        // 移动身体
        for (int i = game->snake.length - 1; i > 0; i--) {
            game->snake.body[i] = game->snake.body[i - 1];
        }
        game->snake.body[0] = new_head;
    }
}

void draw_game(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GameState *game = (GameState *)data;

    // 绘制背景
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_paint(cr);

    // 绘制网格
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_set_line_width(cr, 0.5);
    for (int x = 0; x <= GRID_SIZE; x++) {
        cairo_move_to(cr, x * CELL_SIZE, 0);
        cairo_line_to(cr, x * CELL_SIZE, GRID_SIZE * CELL_SIZE);
    }
    for (int y = 0; y <= GRID_SIZE; y++) {
        cairo_move_to(cr, 0, y * CELL_SIZE);
        cairo_line_to(cr, GRID_SIZE * CELL_SIZE, y * CELL_SIZE);
    }
    cairo_stroke(cr);

    // 绘制食物
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_rectangle(cr,
                  game->food.pos.x * CELL_SIZE,
                  game->food.pos.y * CELL_SIZE,
                  CELL_SIZE, CELL_SIZE);
    cairo_fill(cr);

    // 绘制蛇
    for (int i = 0; i < game->snake.length; i++) {
        if (i == 0) {
            cairo_set_source_rgb(cr, 0, 0.8, 0); // 蛇头
        } else {
            cairo_set_source_rgb(cr, 0, 0.5, 0); // 蛇身
        }
        cairo_rectangle(cr,
                      game->snake.body[i].x * CELL_SIZE,
                      game->snake.body[i].y * CELL_SIZE,
                      CELL_SIZE, CELL_SIZE);
        cairo_fill(cr);
    }

    // 显示游戏信息
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);

    char info[50];
    snprintf(info, sizeof(info), "Level: %d  Score: %d", game->level, game->score);
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, info);

    if (game->paused) {
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_set_font_size(cr, 24);
        cairo_move_to(cr, GRID_SIZE * CELL_SIZE / 2 - 30, GRID_SIZE * CELL_SIZE / 2);
        cairo_show_text(cr, "PAUSED");
    }
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    GameState *game = (GameState *)data;

    switch (event->keyval) {
        case GDK_KEY_Up:
        case GDK_KEY_w:
            if (game->snake.dir != DOWN) game->snake.dir = UP;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_d:
            if (game->snake.dir != LEFT) game->snake.dir = RIGHT;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_s:
            if (game->snake.dir != UP) game->snake.dir = DOWN;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_a:
            if (game->snake.dir != RIGHT) game->snake.dir = LEFT;
            break;
        case GDK_KEY_p:
            game->paused = !game->paused;
            break;
        case GDK_KEY_r:
            if (game->game_over) reset_game(game);
            break;
        case GDK_KEY_q:
            gtk_main_quit();
            break;
    }

    gtk_widget_queue_draw(game->drawing_area);
    return TRUE;
}

gboolean game_loop(gpointer data) {
    GameState *game = (GameState *)data;

    if (!game->paused && !game->game_over) {
        update_game(game);
        gtk_widget_queue_draw(game->drawing_area);
    }

    return TRUE;
}

void save_highscores(GameState *game) {
    FILE *file = fopen("highscores.dat", "wb");
    if (file) {
        fwrite(game->highscores, sizeof(HighScore), game->highscores_count, file);
        fclose(file);
    }
}

void load_highscores(GameState *game) {
    FILE *file = fopen("highscores.dat", "rb");
    if (file) {
        game->highscores_count = fread(game->highscores, sizeof(HighScore), MAX_SCORES, file);
        fclose(file);
    } else {
        game->highscores_count = 0;
    }
}

void show_game_over_dialog(GameState *game) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Game Over",
        GTK_WINDOW(game->window),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Enter your name:");
    GtkWidget *entry = gtk_entry_new();

    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry));

    // 添加到高分榜
    if (game->highscores_count < MAX_SCORES) {
        strncpy(game->highscores[game->highscores_count].name, name, 20);
        game->highscores[game->highscores_count].score = game->score;
        game->highscores_count++;
    } else {
        // 替换最低分
        int min_index = 0;
        for (int i = 1; i < MAX_SCORES; i++) {
            if (game->highscores[i].score < game->highscores[min_index].score) {
                min_index = i;
            }
        }
        if (game->score > game->highscores[min_index].score) {
            strncpy(game->highscores[min_index].name, name, 20);
            game->highscores[min_index].score = game->score;
        }
    }

    // 排序高分榜
    for (int i = 0; i < game->highscores_count - 1; i++) {
        for (int j = 0; j < game->highscores_count - i - 1; j++) {
            if (game->highscores[j].score < game->highscores[j + 1].score) {
                HighScore temp = game->highscores[j];
                game->highscores[j] = game->highscores[j + 1];
                game->highscores[j + 1] = temp;
            }
        }
    }

    save_highscores(game);
    gtk_widget_destroy(dialog);

    // 显示高分榜
    GtkWidget *highscore_dialog = gtk_dialog_new_with_buttons(
        "High Scores",
        GTK_WINDOW(game->window),
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkWidget *hs_content = gtk_dialog_get_content_area(GTK_DIALOG(highscore_dialog));
    GtkWidget *hs_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    for (int i = 0; i < game->highscores_count; i++) {
        char line[50];
        snprintf(line, sizeof(line), "%d. %s - %d", i + 1, game->highscores[i].name, game->highscores[i].score);
        GtkWidget *hs_label = gtk_label_new(line);
        gtk_box_pack_start(GTK_BOX(hs_box), hs_label, FALSE, FALSE, 0);
    }

    gtk_container_add(GTK_CONTAINER(hs_content), hs_box);
    gtk_widget_show_all(highscore_dialog);

    gtk_dialog_run(GTK_DIALOG(highscore_dialog));
    gtk_widget_destroy(highscore_dialog);

    reset_game(game);
}



int main(int argc, char *argv[]) {
    GameState game = {0};

    // 初始化GTK
    gtk_init(&argc, &argv);

    // 创建主窗口
    game.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(game.window), "Snake Game");
    gtk_window_set_default_size(GTK_WINDOW(game.window), GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE);
    g_signal_connect(game.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 创建绘图区域
    game.drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(game.window), game.drawing_area);

    // 初始化游戏
    init_game(&game);

    // 连接信号
    g_signal_connect(game.drawing_area, "draw", G_CALLBACK(draw_game), &game);
    g_signal_connect(game.window, "key-press-event", G_CALLBACK(on_key_press), &game);

    // 显示窗口
    gtk_widget_show_all(game.window);

    // 设置游戏循环
    g_timeout_add(game.speed, game_loop, &game);

    // 进入GTK主循环
    gtk_main();

    // 清理资源
    free(game.snake.body);

    return 0;
}