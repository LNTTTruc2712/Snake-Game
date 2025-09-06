#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

Color darkNavy = {25, 25, 112, 255};

int cellSize = 30;
int cellCount = 28;
int offset = 75;
double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque) {
    for (unsigned int i = 0; i < deque.size(); i++) {
        if (Vector2Equals(deque[i], element)) {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake {
public:
    // first appearance
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    void Draw() {
        for (unsigned int i = 0; i < body.size(); i++) {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float) cellSize,
                                          (float) cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void Update() {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true) {
            addSegment = false;
        } else {
            body.pop_back();
        }
    }

    void Reset() {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food {
public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody) {
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    // destruct
    ~Food() {
        UnloadTexture(texture);
    }

    void Draw() {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody)) {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game {
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    Game() {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw() {
        food.Draw();
        snake.Draw();
    }

    void Update() {
        if (running) {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.body[0], food.position)) {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges() {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1) {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
            GameOver();
        }
    }

    void GameOver() {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail() {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)) {
            GameOver();
        }
    }
};

int main() {
    cout << "Starting the game..." << endl;

    InitWindow(2 * offset + cellSize * cellCount,
               2 * offset + cellSize * cellCount,
               "Snake Game");

    SetTargetFPS(60);
    Game game = Game();

    Font font = LoadFont("../Fonts/static/Roboto-Italic.ttf");
    while (!WindowShouldClose()) {
        BeginDrawing();
        double interval = 0.3;

        if (game.running) {
            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
                game.snake.direction = {0, -1};
            } else if (IsKeyDown(KEY_UP) && game.snake.direction.y == -1) {
                interval = 0.1;
            }

            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
                game.snake.direction = {0, 1};
            } else if (IsKeyDown(KEY_DOWN) && game.snake.direction.y == 1) {
                interval = 0.1;
            }

            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
                game.snake.direction = {-1, 0};
            } else if (IsKeyDown(KEY_LEFT) && game.snake.direction.x == -1) {
                interval = 0.1;
            }

            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
                game.snake.direction = {1, 0};
            } else if (IsKeyDown(KEY_RIGHT) && game.snake.direction.x == 1) {
                interval = 0.1;
            }
        }

        if (eventTriggered(interval)) {
            game.Update();
        }

        if (IsKeyPressed(KEY_P) && game.running) {
            game.paused = !game.paused;
        }

        ClearBackground(darkNavy);
        DrawRectangleGradientV(
                0, 0,
                GetScreenWidth(), GetScreenHeight(),
                SKYBLUE, DARKBLUE
        );

        DrawRectangleLinesEx(Rectangle{float(offset - 5), float(offset - 5),
                                       float(cellSize * cellCount + 10),
                                       float(cellSize * cellCount + 10)}, 5, WHITE);

        DrawTextEx(font, TextFormat("Score: %i", game.score), Vector2{20, 20}, 32, 2, RED);

        const char *missMsg = TextFormat("Miss: %i / 5", game.missCount);
        int fontSize = 32;
        Vector2 textSize = MeasureTextEx(font, missMsg, fontSize, 2);

        float x = GetScreenWidth() / 2 - textSize.x / 2;
        float y = 20;
        DrawTextEx(font, missMsg, Vector2{x, y}, fontSize, 2, RED);

        const char *livesMsg = TextFormat("Lives: %i", game.lives);
        DrawTextEx(font, livesMsg, Vector2{float(GetScreenWidth()) - 200, 20}, 32, 2, BLUE);

        game.Draw();

        if (!game.running && game.lives <= 0) {
            DrawText("GAME OVER - Press ENTER OR SPACE to restart", 100, 200, 30, RED);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                game.snake.Reset();
                game.score = 0;
                game.missCount = 0;
                game.lives = 1;
                game.running = true;
                game.paused = false;
                game.food.Respawn(game.snake.body);
            }
        } else if (game.paused) {
            const char *msg = "PAUSED - Press P to continue";
            int fontSize = 30;
            int textWidth = MeasureText(msg, fontSize);
            int x = GetScreenWidth() / 2 - textWidth / 2;
            int y = GetScreenHeight() / 2 - fontSize / 2;
            DrawText(msg, x, y, fontSize, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;

}
