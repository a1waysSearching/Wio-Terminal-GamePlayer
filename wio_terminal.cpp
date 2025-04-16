#include <TFT_eSPI.h>
#include "LIS3DHTR.h"
LIS3DHTR<TwoWire> lis;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite screenBuffer = TFT_eSprite(&tft);

enum GameStatus {
    START_SCREEN,
    PLAYING,
    GAME_OVER
};

struct GameState {
    float ballX;
    float ballY;
    float ballVx;
    float ballVy;
    int paddleX;
    int score;
    GameStatus status;
    unsigned long gameOverTime;
    int rewardX;
    int rewardWidth;
    int rewardHeight;
};

GameState gameState = {160.0, 120.0, 0.0, 0.0, 140, 0, START_SCREEN, 0, 0, 40, 1};

const int screenWidth = 320;
const int screenHeight = 240;
const int ballRadius = 5;
const int paddleWidth = 60;
const int paddleHeight = 10;
const int paddleY = screenHeight - 20;
const float initialSpeed = 3;
const float speedIncrement = 0.05;
const int paddleSpeed = 5;
const unsigned long frameInterval = 16;
const unsigned long gameOverDelay = 2000;
const unsigned long debounceInterval = 10;
const float minAngle = 30.0 * PI / 180.0;

void resetGame();
void updateGameLogic();
void handleButtons();
void renderDisplay();
void generateReward();

void setup() {
    Serial.begin(115200);
    delay(1000);
    while (!Serial);

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    screenBuffer.createSprite(screenWidth, screenHeight);
    screenBuffer.fillSprite(TFT_BLACK);

    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);

    lis.begin(Wire1);
    if (!lis) {
        Serial.println("ERROR");
        while(1);
    }
    lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
    lis.setFullScaleRange(LIS3DHTR_RANGE_2G);

    generateReward();
}

float x_values, y_values, z_values;
void loop() {
    static unsigned long lastUpdate = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastUpdate >= frameInterval) {
        handleButtons();
        updateGameLogic();
        renderDisplay();
        lastUpdate = currentTime;
    }

    x_values = lis.getAccelerationX();
    y_values = lis.getAccelerationY();
    z_values = lis.getAccelerationZ();
}

void handleButtons() {
    static unsigned long lastDebounce = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastDebounce >= debounceInterval) {
        if (gameState.status == START_SCREEN) {
            if (digitalRead(WIO_5S_UP) == LOW) {
                gameState.status = PLAYING;
                resetGame();
            }
        } else if (gameState.status == PLAYING) {
            if (digitalRead(WIO_5S_LEFT) == LOW && gameState.paddleX > 0) {
                gameState.paddleX -= paddleSpeed;
            } else if (digitalRead(WIO_5S_RIGHT) == LOW && gameState.paddleX < screenWidth - paddleWidth) {
                gameState.paddleX += paddleSpeed;
            }
            if (y_values > 0 && gameState.paddleX > 0) {
                gameState.paddleX -= y_values * 10;
            } else if (y_values < 0 && gameState.paddleX < screenWidth - paddleWidth) {
                gameState.paddleX += (0 - y_values) * 10;
            }
        } else if (gameState.status == GAME_OVER) {
            if (digitalRead(WIO_5S_DOWN) == LOW) {
                gameState.status = START_SCREEN;
            }
        }
        lastDebounce = currentTime;
    }
}

void updateGameLogic() {
    if (gameState.status == PLAYING) {
        gameState.ballX += gameState.ballVx;
        gameState.ballY += gameState.ballVy;

        if (gameState.ballX - ballRadius < 0 || gameState.ballX + ballRadius > screenWidth) {
            gameState.ballVx = -gameState.ballVx;
            gameState.ballX = constrain(gameState.ballX, ballRadius, screenWidth - ballRadius);
        }
        if (gameState.ballY - ballRadius < 0) {
            if (gameState.ballX >= gameState.rewardX && gameState.ballX <= gameState.rewardX + gameState.rewardWidth) {
                gameState.score += 2;
                generateReward();
            } else {
                gameState.score += 1;
            }
            gameState.ballVy = -gameState.ballVy;
            gameState.ballY = ballRadius;
            gameState.ballVx *= (1.0 + speedIncrement);
            gameState.ballVy *= (1.0 + speedIncrement);
        }
        if (gameState.ballY + ballRadius > screenHeight) {
            gameState.status = GAME_OVER;
            gameState.gameOverTime = millis();
        }

        if (gameState.ballY + ballRadius > paddleY && 
            gameState.ballY - ballRadius < paddleY + paddleHeight &&
            gameState.ballX > gameState.paddleX && 
            gameState.ballX < gameState.paddleX + paddleWidth) {
            float relativeIntersectX = gameState.ballX - (gameState.paddleX + paddleWidth / 2.0);
            float normalizedIntersectX = relativeIntersectX / (paddleWidth / 2.0);
            float bounceAngle = normalizedIntersectX * (PI / 3.0);
            if (abs(bounceAngle) < minAngle) {
                bounceAngle = (bounceAngle > 0 ? 1 : -1) * minAngle;
            }
            float speed = sqrt(gameState.ballVx * gameState.ballVx + gameState.ballVy * gameState.ballVy);
            gameState.ballVx = speed * sin(bounceAngle);
            gameState.ballVy = -speed * cos(bounceAngle);
            gameState.ballY = paddleY - ballRadius;
        }
    }
}

void renderDisplay() {
    screenBuffer.fillSprite(TFT_BLACK);

    if (gameState.status == START_SCREEN) {
        screenBuffer.setTextColor(TFT_GREEN);
        screenBuffer.setTextSize(3);
        screenBuffer.drawString("Bounce Ball", 60, 80);
        screenBuffer.setTextSize(2);
        screenBuffer.drawString("Press UP to Start", 60, 140);
    } else if (gameState.status == PLAYING) {
        screenBuffer.fillCircle((int)gameState.ballX, (int)gameState.ballY, ballRadius, TFT_WHITE);
        screenBuffer.fillRoundRect(gameState.paddleX, paddleY, paddleWidth, paddleHeight, 5, TFT_BLUE);
        screenBuffer.setTextColor(TFT_YELLOW, TFT_BLACK);
        screenBuffer.setTextSize(2);
        screenBuffer.drawString("Score: " + String(gameState.score), 10, 10);
        screenBuffer.fillRect(gameState.rewardX, 0, gameState.rewardWidth, gameState.rewardHeight, TFT_RED);
    } else if (gameState.status == GAME_OVER) {
        screenBuffer.setTextColor(TFT_RED);
        screenBuffer.setTextSize(3);
        screenBuffer.drawString("Game Over", 80, 80);
        screenBuffer.drawString("Score: " + String(gameState.score), 80, 120);
        screenBuffer.setTextSize(2);
        screenBuffer.drawString("Press DOWN to Restart", 35, 160);
    }

    screenBuffer.pushSprite(0, 0);
}

void resetGame() {
    gameState.ballX = screenWidth / 2;
    gameState.ballY = screenHeight / 2;
    gameState.ballVx = (random(0, 2) == 0 ? 1 : -1) * initialSpeed;
    gameState.ballVy = (random(0, 2) == 0 ? 1 : -1) * initialSpeed;
    gameState.paddleX = (screenWidth - paddleWidth) / 2;
    gameState.score = 0;
    gameState.gameOverTime = 0;
    generateReward();
}

void generateReward() {
    gameState.rewardX = random(0, screenWidth - gameState.rewardWidth);
}