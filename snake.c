void render(GameState *game, Snake *snake) {
    system("clear"); // 清屏

    // 绘制游戏边界
    for (int i = 0; i < game->width + 2; i++) printf("#");
    printf("\n");

    for (int y = 0; y < game->height; y++) {
        printf("#"); // 左边界
        for (int x = 0; x < game->width; x++) {
            if (is_snake_body(snake, x, y)) {
                printf("O"); // 蛇身
            } else if (x == game->food.x && y == game->food.y) {
                printf("*"); // 食物
            } else if (game->obstacles && game->obstacles[y][x]) {
                printf("X"); // 障碍物
            } else {
                printf(" ");
            }
        }
        printf("#\n"); // 右边界
    }

    // 绘制游戏边界
    for (int i = 0; i < game->width + 2; i++) printf("#");
    printf("\n");

    // 显示游戏信息
    printf("Level: %d | Score: %d | Speed: %d\n", game->level, game->score, game->speed);
    if (game->paused) printf("PAUSED - Press P to continue\n");
    if (game->game_over) printf("GAME OVER! Press Q to quit\n");
}