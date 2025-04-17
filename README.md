# 项目概述和目标

![image](https://files.seeedstudio.com/wiki/Wio-Terminal/img/Wio-Terminal-Wiki.jpg)

本项目基于Seeed Studio的Wio Terminal开发了一个交互式弹球游戏。通过这个项目，我们将展示如何利用Wio Terminal的LCD显示屏、按钮和加速度计传感器创建一个简单而有趣的游戏应用。

![image](https://files.seeedstudio.com/wiki/Wio-Terminal/img/Wio-Terminal-Hardware-Overview.png)

Wio 终端是一款基于 SAMD51 的微控制器，配备Realtek RTL8720DN 芯片，支持无线连接，兼容 Arduino 和 MicroPython。目前，无线连接仅由 Arduino 支持。它运行频率为120MHz（最高可达 200MHz），配备4MB外部闪存和192KB内存。它支持蓝牙和 Wi-Fi，为物联网项目提供可靠的连接。Wio 终端本身配备2.4 英寸 LCD 屏幕、板载惯性测量单元 (LIS3DHTR)、麦克风、蜂鸣器、microSD 卡槽、光传感器和红外发射器 (IR 940nm)。此外，它还配备两个用于Grove 生态系统的多功能 Grove 端口，以及 40 个兼容 Raspberry Pi 的 GPIO 引脚，可用于连接更多扩展组件。


**项目目标：**
- 开发一个功能完整的弹球游戏
- 实现多种输入控制方式（按键和加速度计）
- 展示Wio Terminal的图形显示和传感器集成能力
- 提供一个可扩展的游戏开发框架

# 材料清单与环境设置

![image](https://media-cdn.seeedstudio.com/media/wysiwyg/wtpinout2.png)

## 硬件需求
- Wio Terminal（[购买链接](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)）
- USB-C数据线（用于编程和供电）
- 电脑（用于开发和上传代码）

## 软件环境
1. **Arduino IDE**
   - 下载并安装最新版本的[Arduino IDE](https://www.arduino.cc/en/software)
   - 添加Wio Terminal开发板支持：
     - 打开Arduino IDE，进入"文件 > 首选项"
     - 在"附加开发板管理器网址"中添加：`https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json`
     - 进入"工具 > 开发板 > 开发板管理器"，搜索并安装"Seeed SAMD Boards"

2. **必要库文件**
   - TFT_eSPI：用于LCD显示控制
   - LIS3DHTR：用于加速度计数据读取
   
   安装方法：在Arduino IDE中，进入"工具 > 管理库"，搜索并安装上述库

## 硬件连接
Wio Terminal是一个集成设备，无需额外连接。只需通过USB-C线缆连接到电脑即可进行编程。

# 详细步骤说明

## 1. 基础设置
1. 连接Wio Terminal到电脑
2. 在Arduino IDE中选择正确的开发板和端口：
   - 开发板：`Tools > Board > Seeed SAMD > Wio Terminal`
   - 端口：选择Wio Terminal连接的COM端口

## 2. 游戏玩法
1. 在开始界面，按下Wio Terminal的上键开始游戏
2. 使用左右键或倾斜设备控制挡板移动
3. 防止球落到屏幕底部
4. 当球碰到屏幕顶部时得分，碰到红色奖励区域可获得双倍分数
5. 游戏结束后，按下键重新开始

## 3. 游戏架构设计
1. 定义游戏状态和数据结构
2. 实现游戏初始化函数
3. 设计游戏循环和逻辑更新
4. 实现输入处理和显示渲染

## 4. 编译与上传
1. 点击Arduino IDE中的"验证"按钮检查代码
2. 点击"上传"按钮将代码上传到Wio Terminal
3. 上传完成后，游戏将自动运行

## 5. 测试与调试

![image](https://github.com/a1waysSearching/Wio-Terminal-GamePlayer/blob/main/7-2920250417_092216.gif)

使用IMU控制

![image](https://github.com/a1waysSearching/Wio-Terminal-GamePlayer/blob/main/7-2920250417_092310.gif)

使用按键控制

1. 使用Serial监视器查看调试信息（如需要）
2. 测试各种游戏功能和交互
3. 根据需要调整参数和逻辑

# 代码解析

## 核心数据结构

```cpp
// 游戏状态枚举
enum GameStatus {
    START_SCREEN,  // 开始界面
    PLAYING,       // 游戏进行中
    GAME_OVER      // 游戏结束
};

// 游戏状态结构体
struct GameState {
    float ballX, ballY;         // 球的位置
    float ballVx, ballVy;       // 球的速度
    int paddleX;                // 挡板位置
    int score;                  // 分数
    GameStatus status;          // 当前游戏状态
    unsigned long gameOverTime; // 游戏结束时间戳
    int rewardX;                // 奖励区域位置
    int rewardWidth, rewardHeight; // 奖励区域尺寸
};
```

## 关键函数解析

### 1. 游戏初始化
```cpp
void setup() {
    // 初始化串口通信
    Serial.begin(115200);
    
    // 初始化显示屏
    tft.begin();
    tft.setRotation(3);  // 横屏模式
    
    // 创建屏幕缓冲区
    screenBuffer.createSprite(screenWidth, screenHeight);
    
    // 设置按钮输入
    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);
    
    // 初始化加速度计
    lis.begin(Wire1);
    lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
    lis.setFullScaleRange(LIS3DHTR_RANGE_2G);
    
    // 生成初始奖励区域
    generateReward();
}
```

### 2. 主循环
```cpp
void loop() {
    // 帧率控制
    static unsigned long lastUpdate = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastUpdate >= frameInterval) {
        // 处理输入
        handleButtons();
        
        // 更新游戏逻辑
        updateGameLogic();
        
        // 渲染画面
        renderDisplay();
        
        lastUpdate = currentTime;
    }
    
    // 读取加速度计数据
    x_values = lis.getAccelerationX();
    y_values = lis.getAccelerationY();
    z_values = lis.getAccelerationZ();
}
```

### 3. 物理模拟与碰撞检测
```cpp
// 球与挡板碰撞处理
if (gameState.ballY + ballRadius > paddleY && 
    gameState.ballY - ballRadius < paddleY + paddleHeight &&
    gameState.ballX > gameState.paddleX && 
    gameState.ballX < gameState.paddleX + paddleWidth) {
    
    // 计算碰撞点相对挡板中心的位置
    float relativeIntersectX = gameState.ballX - (gameState.paddleX + paddleWidth / 2.0);
    
    // 归一化到[-1,1]范围
    float normalizedIntersectX = relativeIntersectX / (paddleWidth / 2.0);
    
    // 计算反弹角度（最大60度）
    float bounceAngle = normalizedIntersectX * (PI / 3.0);
    
    // 确保最小反弹角度
    if (abs(bounceAngle) < minAngle) {
        bounceAngle = (bounceAngle > 0 ? 1 : -1) * minAngle;
    }
    
    // 保持速度大小，改变方向
    float speed = sqrt(gameState.ballVx * gameState.ballVx + gameState.ballVy * gameState.ballVy);
    gameState.ballVx = speed * sin(bounceAngle);
    gameState.ballVy = -speed * cos(bounceAngle);
    
    // 调整球位置，避免多次碰撞
    gameState.ballY = paddleY - ballRadius;
}
```

# 扩展思路

1. **多级难度**
   - 添加难度选择界面
   - 根据难度调整球速、挡板大小等参数

2. **多人模式**
   - 添加第二个挡板在屏幕顶部
   - 利用顶部三个按键控制第二个人的挡板
   - 实现双人对战模式

3. **障碍物系统**
   - 在游戏区域添加静态或移动的障碍物
   - 设计不同关卡的障碍物布局

4. **道具系统**
   - 添加可收集的道具（如扩大挡板、减慢球速等）
   - 实现道具效果的计时和显示

5. **声音效果**
   - 添加碰撞、得分和游戏结束的音效
   - 使用Wio Terminal的蜂鸣器或外接扬声器

6. **高分记录**
   - 使用EEPROM存储最高分数
   - 添加高分榜显示

7. **图形优化**
   - 添加游戏背景和动画效果
   - 优化UI设计和视觉反馈

# 参考资源

1. **官方文档**
   - [Wio Terminal官方Wiki](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/)
   - [TFT_eSPI库文档](https://github.com/Bodmer/TFT_eSPI)
   - [LIS3DHTR库文档](https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR)

2. **教程与示例**
   - [Seeed Studio Arduino教程](https://wiki.seeedstudio.com/Wio-Terminal-LCD-Overview/)

3. **社区资源**
   - [Seeed Studio论坛](https://forum.seeedstudio.com/)
   - [Arduino论坛](https://forum.arduino.cc/)

4. **进阶学习**
   - [游戏物理基础](https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics)
   - [Arduino游戏编程技巧](https://create.arduino.cc/projecthub/projects/tags/game)

