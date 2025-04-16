# Wio-Terminal-GamePlayer
本项目基于Seeed Studio的Wio Terminal开发了一个简单的弹球游戏。Wio Terminal是一款基于ATSAMD51的微控制器开发板，集成了2.4英寸LCD屏幕、加速度计、按钮和多种接口，非常适合开发交互式应用和游戏。

# Wio Terminal 弹球游戏开发文档

## 1. 项目概述

本项目基于Seeed Studio的Wio Terminal开发了一个简单的弹球游戏。Wio Terminal是一款基于ATSAMD51的微控制器开发板，集成了2.4英寸LCD屏幕、加速度计、按钮和多种接口，非常适合开发交互式应用和游戏。

## 2. 硬件平台

- **开发板**：[Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)
- **处理器**：ARM Cortex-M4F (ATSAMD51)
- **显示屏**：2.4英寸LCD (320×240分辨率)
- **传感器**：LIS3DHTR加速度计
- **输入设备**：5向操纵杆

## 3. 开发环境

- **IDE**：Arduino IDE
- **主要库**：
  - TFT_eSPI：用于LCD显示控制
  - LIS3DHTR：用于加速度计数据读取

## 4. 游戏设计

### 4.1 游戏规则

- 玩家控制一个挡板在屏幕底部左右移动
- 球在屏幕中弹跳，必须防止球落到屏幕底部
- 当球碰到屏幕顶部时，玩家得分
- 屏幕顶部有特殊奖励区域，球碰到该区域可获得双倍分数
- 球每次反弹后速度略微增加，增加游戏难度
- 当球落到屏幕底部时，游戏结束

### 4.2 游戏状态

游戏有三种状态：
1. **开始界面**：显示游戏标题和开始提示
2. **游戏中**：显示球、挡板、分数和奖励区域
3. **游戏结束**：显示最终分数和重新开始提示

## 5. 代码结构

### 5.1 主要组件

```
wio_terminal.cpp
├── 游戏状态定义 (GameStatus枚举和GameState结构体)
├── 全局常量 (屏幕尺寸、球半径、挡板尺寸等)
├── 主要函数
│   ├── setup()：初始化设备和游戏
│   ├── loop()：主循环，控制游戏帧率
│   ├── handleButtons()：处理按钮和加速度计输入
│   ├── updateGameLogic()：更新游戏状态和物理模拟
│   ├── renderDisplay()：渲染游戏画面
│   ├── resetGame()：重置游戏状态
│   └── generateReward()：生成奖励区域
```

### 5.2 关键数据结构

```cpp
enum GameStatus {
    START_SCREEN,
    PLAYING,
    GAME_OVER
};

struct GameState {
    float ballX;        // 球的X坐标
    float ballY;        // 球的Y坐标
    float ballVx;       // 球的X方向速度
    float ballVy;       // 球的Y方向速度
    int paddleX;        // 挡板的X坐标
    int score;          // 当前分数
    GameStatus status;  // 游戏状态
    unsigned long gameOverTime;  // 游戏结束时间戳
    int rewardX;        // 奖励区域X坐标
    int rewardWidth;    // 奖励区域宽度
    int rewardHeight;   // 奖励区域高度
};
```

## 6. 功能实现

### 6.1 输入控制

游戏支持两种输入方式：
1. **按钮控制**：使用Wio Terminal的5向操纵杆
   - 上键：开始游戏
   - 左右键：移动挡板
   - 下键：游戏结束后重新开始

2. **加速度计控制**：倾斜设备来移动挡板
   ```cpp
   if (y_values > 0 && gameState.paddleX > 0) {
       gameState.paddleX -= y_values * 10;
   } else if (y_values < 0 && gameState.paddleX < screenWidth - paddleWidth) {
       gameState.paddleX += (0 - y_values) * 10;
   }
   ```

### 6.2 物理模拟

1. **球的移动**：
   ```cpp
   gameState.ballX += gameState.ballVx;
   gameState.ballY += gameState.ballVy;
   ```

2. **边界碰撞检测**：
   ```cpp
   if (gameState.ballX - ballRadius < 0 || gameState.ballX + ballRadius > screenWidth) {
       gameState.ballVx = -gameState.ballVx;
       gameState.ballX = constrain(gameState.ballX, ballRadius, screenWidth - ballRadius);
   }
   ```

3. **挡板碰撞检测与反弹角度计算**：
   ```cpp
   if (gameState.ballY + ballRadius > paddleY && 
       gameState.ballY - ballRadius < paddleY + paddleHeight &&
       gameState.ballX > gameState.paddleX && 
       gameState.ballX < gameState.paddleX + paddleWidth) {
       
       float relativeIntersectX = gameState.ballX - (gameState.paddleX + paddleWidth / 2.0);
       float normalizedIntersectX = relativeIntersectX / (paddleWidth / 2.0);
       float bounceAngle = normalizedIntersectX * (PI / 3.0);
       
       // 确保反弹角度不会太小
       if (abs(bounceAngle) < minAngle) {
           bounceAngle = (bounceAngle > 0 ? 1 : -1) * minAngle;
       }
       
       float speed = sqrt(gameState.ballVx * gameState.ballVx + gameState.ballVy * gameState.ballVy);
       gameState.ballVx = speed * sin(bounceAngle);
       gameState.ballVy = -speed * cos(bounceAngle);
       gameState.ballY = paddleY - ballRadius;
   }
   ```

### 6.3 图形渲染

使用TFT_eSprite创建屏幕缓冲区，避免屏幕闪烁：

```cpp
screenBuffer.createSprite(screenWidth, screenHeight);
// 在缓冲区中绘制所有元素
screenBuffer.pushSprite(0, 0);  // 一次性将缓冲区内容推送到屏幕
```

## 7. 优化技巧

1. **帧率控制**：使用时间间隔控制游戏更新频率，确保游戏在不同设备上运行速度一致
   ```cpp
   if (currentTime - lastUpdate >= frameInterval) {
       // 更新游戏逻辑和渲染
       lastUpdate = currentTime;
   }
   ```

2. **按键去抖动**：防止按键抖动导致的误触发
   ```cpp
   if (currentTime - lastDebounce >= debounceInterval) {
       // 处理按键输入
       lastDebounce = currentTime;
   }
   ```

3. **碰撞检测优化**：使用简化的AABB（轴对齐边界框）碰撞检测算法

4. **内存优化**：使用sprite缓冲区减少屏幕刷新次数，降低闪烁

## 8. 扩展功能建议

1. **多关卡系统**：随着分数增加，增加游戏难度或改变游戏规则
2. **音效支持**：添加碰撞、得分和游戏结束的音效
3. **高分记录**：使用EEPROM存储最高分数
4. **多球模式**：同时控制多个球增加游戏难度
5. **障碍物**：添加静态或移动的障碍物
6. **道具系统**：添加可收集的道具，如扩大挡板、减慢球速等

## 9. 参考资源

- [Wio Terminal官方文档](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/)
- [TFT_eSPI库文档](https://github.com/Bodmer/TFT_eSPI)
- [LIS3DHTR库文档](https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR)

## 10. 总结

本项目展示了如何利用Wio Terminal的显示屏、按钮和加速度计开发一个简单而有趣的弹球游戏。通过合理的代码结构和优化技巧，实现了流畅的游戏体验。该项目可作为Wio Terminal游戏开发的入门示例，也可以作为更复杂游戏的基础进行扩展。
