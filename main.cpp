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
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    int segmentsToAdd = 0;

    void Draw(bool alive = true) {
        for (int i = 0; i < body.size(); i++) {
            int x = offset + body[i].x * cellSize + cellSize / 2;
            int y = offset + body[i].y * cellSize + cellSize / 2;

            if (i == 0) {
                Color headColor = alive ? Color{255, 182, 193, 255} : Color{255, 105, 180, 255};
                DrawCircle(x, y, cellSize / 2, headColor);
                DrawCircleLines(x, y, cellSize / 2, BLACK);

                if (alive) {
                    // mắt
                    DrawCircle(x - 6, y - 5, 6, WHITE);
                    DrawCircle(x - 6, y - 5, 3, BLACK);
                    DrawCircle(x + 6, y - 5, 6, WHITE);
                    DrawCircle(x + 6, y - 5, 3, BLACK);
                    // miệng
                    DrawLine(x - 3, y + 5, x + 3, y + 5, RED);
                } else {
                    DrawText(">", x - 10, y - 10, 20, BLACK);
                    DrawText("<", x, y - 10, 20, BLACK);
                    DrawLine(x - 4, y + 8, x + 4, y + 8, RED);
                }
            } else {
                Color bodyColor = (i % 2 == 0) ? Color{255, 182, 193, 255} : Color{255, 105, 180, 255};
                DrawCircle(x, y, cellSize / 2, bodyColor);
                DrawCircleLines(x, y, cellSize / 2, BLACK);
            }
        }
    }

    int Update(int cellCount, int lives) {
        Vector2 newHead = Vector2Add(body[0], direction);

        // Va chạm tường
        if (newHead.x < 0 || newHead.x >= cellCount ||
            newHead.y < 0 || newHead.y >= cellCount) {
            return (lives <= 1) ? 1 : 2;
        }

        // Cắn thân
        if (ElementInDeque(newHead, body)) {
            return (lives <= 1) ? 1 : 2;
        }

        // Di chuyển bình thường
        body.push_front(newHead);
        if (segmentsToAdd > 0) {
            segmentsToAdd--;
        } else {
            body.pop_back();
        }
        return 0;
    }

    void Reset() {
        body = {Vector2{5, 9}, Vector2{4, 9}, Vector2{3, 9}};
        direction = {1, 0};
        segmentsToAdd = 0;
    }
};

enum class FruitType {
    APPLE,
    BANANA,
    STRAWBERRY,
    CHERRY
};

class Food {
public:
    Vector2 position;
    Texture2D texture;
    bool loaded = false;
    double spawnTime = 0;
    FruitType type;   // loại quả

    Food(deque<Vector2> snakeBody) {
        Respawn(snakeBody);
    }

    ~Food() {
        if (loaded) {
            UnloadTexture(texture);
        }
    }

    void Respawn(deque<Vector2> snakeBody) {
        if (loaded) {
            UnloadTexture(texture);
            loaded = false;
        }

        // random loại quả
        int fruitIndex = GetRandomValue(0, 3);
        type = static_cast<FruitType>(fruitIndex);

        // chọn hình phù hợp
        const char* imagePath = "";
        switch (type) {
            case FruitType::APPLE:      imagePath = "../Pic/apple.png"; break;
            case FruitType::BANANA:     imagePath = "../Pic/banana.png"; break;
            case FruitType::STRAWBERRY: imagePath = "../Pic/strawberry.png"; break;
            case FruitType::CHERRY:     imagePath = "../Pic/cherry.png"; break;
        }

        Image image = LoadImage(imagePath);
        ImageResize(&image, cellSize, cellSize);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        loaded = true;

        position = GenerateRandomPos(snakeBody);
        spawnTime = GetTime();
    }

    int GetScore() {
        switch (type) {
            case FruitType::APPLE: return 1;
            case FruitType::BANANA: return 2;
            case FruitType::STRAWBERRY: return 3;
            case FruitType::CHERRY: return 5;
        }
        return 1;
    }

    void Draw() {
        if (loaded) {
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }
    }

    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {
        Vector2 pos = GenerateRandomCell();
        while (ElementInDeque(pos, snakeBody)) {
            pos = GenerateRandomCell();
        }
        return pos;
    }
};

class Game {
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    bool paused = false;
    int score = 0;
    int missCount = 0;
    int lives = 1;

    void Draw() {
        food.Draw();
        snake.Draw(running);
    }

    void Update() {
        if (running && !paused) {
            int collision = snake.Update(cellCount, lives);

            if (collision == 1) {
                GameOver();
                return;
            } else if (collision == 2) {
                GameOver();
                return;
            }

            // Ăn trái cây
            CheckCollisionWithFood();

            // Check miss
            double foodLifetime = 10.0;
            if (score >= 32) {
                foodLifetime = 6.0;
            } else if (score >= 15) {
                foodLifetime = 8.0;
            }

            if (GetTime() - food.spawnTime >= foodLifetime) {
                missCount++;
                food.Respawn(snake.body);
                if (missCount >= 5) {
                    GameOver();
                }
            }
        }
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.body[0], food.position)) {
            int gained = food.GetScore();
            score += gained;
            snake.segmentsToAdd += gained;
            food.Respawn(snake.body);
        }
    }

    void GameOver() {
        lives--;
        if (lives <= 0) {
            running = false;
        } else {
            running = true;
            paused = false;
        }
    }
};

int main(){
    Snake s = Snake;
    s.Draw();
}