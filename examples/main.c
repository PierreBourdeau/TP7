/*******************************************************************************************
*
*   raylib - sample game: snake
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
/*#include <stdio.h>
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define SNAKE_LENGTH   256
#define SQUARE_SIZE     31
#define WALL_NBR 10 //Number of wall generated on the game board
//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Snake {
    Vector2 position;
    Vector2 size;
    Vector2 speed;
    Color color;
    unsigned int lives;
    int counterTail;
    Vector2 snakePosition[SNAKE_LENGTH];
} Snake;

typedef struct FoodOrWall {
    Vector2 position;
    Vector2 size;
    bool active;
    Color color;
    bool isWall; //Added field to specify if an entity is dangerous or not
} FoodOrWall;

typedef struct multiplayers {
    unsigned int nbrOfPlayer;
    Snake* snakes[2];
} MultiP;
//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 1600;
static const int screenHeight = 900;
unsigned int gameFps = 60;
static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static bool menu = true; //Variable that determine the active state of the menu
static FoodOrWall fruit = { 0 };
static FoodOrWall wall[WALL_NBR] = { 0 };
static bool allowMove = false;
static Vector2 offset = { 0 };
static Snake player1[SNAKE_LENGTH] = { 0 };
static Snake player2[SNAKE_LENGTH] = { 0 };
static unsigned int gameMode = 0;
MultiP players = { 0 };
//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)
//------------------------------------------------------------------------------------
// Score storage variables declaratio
//------------------------------------------------------------------------------------
typedef enum {
    STORAGE_POSITION_SCORE = 0,
    STORAGE_POSITION_HISCORE = 1
} StorageData;
static int score; //score of the last run
static int hiscore; //highest score
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)

{
    //Setting default data values of scores to 0 in the file storage.data (1st start of the game)
    //---------------------------------------------------------
    if (LoadStorageValue(STORAGE_POSITION_SCORE) == NULL) {
        SaveStorageValue(STORAGE_POSITION_HISCORE, 0);
        SaveStorageValue(STORAGE_POSITION_SCORE, 0);
    }
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "SNAKE V.2");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(gameFps);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

//Checking for the end of the game and manages the player lives
//---------------------------------------------------------
void EndOfTheGame(Snake *aSnake) {
    aSnake->lives--;
        if (aSnake->lives == 0)
        {
            gameOver = true;
            //Scoring storage (/!\ at the end of each game to minimize file reading)
            SaveStorageValue(STORAGE_POSITION_SCORE, counterTail);
            if (counterTail > hiscore) {
                SaveStorageValue(STORAGE_POSITION_HISCORE, counterTail);
        }
        else
        {   
            aSnake->position = fruit.position; //if the player have other lifes, he restarts on the last fruit present on the map
            aSnake->counterTail--;
        }
}



//Randomized wall generation & wall collision
//---------------------------------------------------------
void WallGeneration(void) {
    //Generate walls if not already active
    if (!wall->active)
    {
        wall->active = true;
        for (int y = 0; y < WALL_NBR; y++) {
            wall[y].position = (Vector2){ GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (screenHeight / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2 };
            for (int k = 0; k < players.nbrOfPlayer; k++) { // loop on nbrOfPlayer
                for (int l = 0; l < players.snakes[k]->counterTail; l++) { // loop on both snakes length
                    if ((wall[y].position.x == players.snakes[k][l].position.x) && (wall[y].position.y == players.snakes[k][l].position.y))
                    {
                        wall[y].position = (Vector2){ GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (screenHeight / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2 };
                        y = y - 1;
                    }
                } 
            }
        }
    }
    // Collision between snake and walls
    for (int i = 0; i < WALL_NBR; i++) {
        for (int j = 0; j < players.nbrOfPlayer; j++) {
            if ((players.snakes[j]->position.x < (wall[i].position.x + wall->size.x) && (players.snakes[j]->position.x + players.snakes[j]->size.x) > wall[i].position.x) &&
                (players.snakes[j]->position.y < (wall[i].position.y + wall->size.y) && (players.snakes[j]->position.y + players.snakes[j]->size.y) > wall[i].position.y))
            {
                EndOfTheGame(players.snakes[j]);
            }
        }
    }
}

//Speed increase difficulty W*I*P
//---------------------------------------------------------
void SpeedIncrease(void) {
    if (gameFps <= 120) {
        gameFps == gameFps + 20;
        SetTargetFPS(gameFps);
    }
}



// Initialize game variables
//---------------------------------------------------------
void InitGame(void)
{
    framesCounter = 0;
    gameOver = false;
    pause = false;
    players.nbrOfPlayer = 2;
    players.snakes[0] = player1;
    if (players.nbrOfPlayer == 2) {
        players.snakes[1] = player2;
    }
    allowMove = false;
    score = LoadStorageValue(STORAGE_POSITION_SCORE);
    hiscore = LoadStorageValue(STORAGE_POSITION_HISCORE);
    offset.x = screenWidth % SQUARE_SIZE;
    offset.y = screenHeight % SQUARE_SIZE;

    for (int i = 0; i < players.nbrOfPlayer; i++) {
        players.snakes[i]->counterTail = 1;
        players.snakes[i]->lives = 3;
        for (int j = 0; j < SNAKE_LENGTH; j++)
        {
            players.snakes[i][j].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
            if (i == 0 && j == 0) // player 1 snake head
            {
                players.snakes[i][j].color = DARKBLUE;
            }
            else if (i == 0) // player 1 snake
            {
                players.snakes[i][j].color = BLUE;
                players.snakes[i][j].position = (Vector2){ offset.x / 2, offset.y / 2 };
                players.snakes[i][j].speed = (Vector2){ SQUARE_SIZE, 0 };
            }
            else if (i == 1 && j == 0) // player 2 snake head
            {
                players.snakes[i][j].color = ORANGE;
            }
            else if (i==1) // player 2 snake
            {
                players.snakes[i][j].color = GOLD;
                players.snakes[i][j].position = (Vector2){ (screenWidth - (offset.x/2)), (screenHeight - (offset.y/2))};
                players.snakes[i][j].speed = (Vector2){-SQUARE_SIZE, 0 };
            }
        }
    }

    for (int n = 0; n < players.nbrOfPlayer; n++) {
        for (int i = 0; i < SNAKE_LENGTH; i++)
        {
            players.snakes[n]->snakePosition[i] = (Vector2){ 0.0f, 0.0f };;
        }
    }

    fruit.size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
    fruit.color = SKYBLUE;
    fruit.active = false;

    if (gameMode != 0) {
        for (int i = 0; i < WALL_NBR; i++) {
            wall[i].isWall = true;
            wall[i].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
            wall[i].color = BEIGE;
            wall[i].active = false;
        }
    }
}


// Update game (one frame)
//---------------------------------------------------------
void UpdateGame(void)
{
    if (!gameOver && !menu)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (pause && IsKeyPressed('E')) {
            menu = true;
        }
        if (!pause)
        {
            // Player 1 control
            if (IsKeyPressed(KEY_RIGHT) && (players.snakes[0][0].speed.x == 0) && allowMove)
            {
                players.snakes[0][0].speed = (Vector2){ SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && (players.snakes[0][0].speed.x == 0) && allowMove)
            {
                players.snakes[0][0].speed = (Vector2){ -SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_UP) && (players.snakes[0][0].speed.y == 0) && allowMove)
            {
                players.snakes[0][0].speed = (Vector2){ 0, -SQUARE_SIZE };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && (players.snakes[0][0].speed.y == 0) && allowMove)
            {
                players.snakes[0][0].speed = (Vector2){ 0, SQUARE_SIZE };
                allowMove = false;
            }
            //Player 2 control
            if (players.snakes[1] != NULL)
            {
                if (IsKeyPressed('G') && (players.snakes[1][0].speed.x == 0) && allowMove)
                {
                    players.snakes[1][0].speed = (Vector2){ SQUARE_SIZE, 0 };
                    allowMove = false;
                }
                if (IsKeyPressed('D') && (players.snakes[1][0].speed.x == 0) && allowMove)
                {
                    players.snakes[1][0].speed = (Vector2){ -SQUARE_SIZE, 0 };
                    allowMove = false;
                }
                if (IsKeyPressed('R') && (players.snakes[1][0].speed.y == 0) && allowMove)
                {
                    players.snakes[1][0].speed = (Vector2){ 0, -SQUARE_SIZE };
                    allowMove = false;
                }
                if (IsKeyPressed('F') && (players.snakes[1][0].speed.y == 0) && allowMove)
                {
                    players.snakes[1][0].speed = (Vector2){ 0, SQUARE_SIZE };
                    allowMove = false;
                }
            }
            // Snake movement
            for (int n = 0; n < players.nbrOfPlayer; n++) 
            {
                for (int i = 0; i < players.snakes[n]->counterTail; i++)
                {
                    players.snakes[n]->snakePosition[i] = players.snakes[n][i].position;
                }
            }

            if ((framesCounter % 5) == 0)
            {
                for (int n = 0; n < players.nbrOfPlayer; n++) {
                    for (int i = 0; i < players.snakes[n]->counterTail; i++)
                    {
                        if (i == 0)
                        {
                            players.snakes[n][0].position.x += players.snakes[n][0].speed.x;
                            players.snakes[n][0].position.y += players.snakes[n][0].speed.y;
                            allowMove = true;
                        }
                        else players.snakes[n][i].position = players.snakes[n]->snakePosition[i - 1];
                    }
                }
            }

            // Wall behaviour
            for (int n = 0; n < players.nbrOfPlayer; n++) {
                if (((players.snakes[n][0].position.x) > (screenWidth - offset.x)) ||
                    ((players.snakes[n][0].position.y) > (screenHeight - offset.y)) ||
                    (players.snakes[n][0].position.x < 0) || (players.snakes[n][0].position.y < 0))
                {
                    EndOfTheGame(players.snakes[n]);
                }
            }

            // Collision with yourself
            for (int n = 0; n < players.nbrOfPlayer; n++)
            {
                for (int i = 1; i < players.snakes[n]->counterTail; i++)
                {
                    if ((players.snakes[n][0].position.x == players.snakes[n][i].position.x) && (players.snakes[n][0].position.y == players.snakes[n][i].position.y)) EndOfTheGame(players.snakes[n]);
                }
                if (players.nbrOfPlayer == 2) {
                    for (int j = 1; j < players.snakes[n]->counterTail; j++)
                    {
                        if ((players.snakes[0][0].position.x == players.snakes[1][j].position.x) && (players.snakes[0][0].position.y == players.snakes[1][j].position.y)) EndOfTheGame(players.snakes[0]);
                        else if ((players.snakes[1][0].position.x == players.snakes[0][j].position.x) && (players.snakes[1][0].position.y == players.snakes[0][j].position.y)) EndOfTheGame(players.snakes[1]);
                    }
                }
            }

            // Fruit position calculation
            if (!fruit.active)
            {
                fruit.active = true;
                fruit.position = (Vector2){ GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (screenHeight / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2 };
                for (int i = 0; i < WALL_NBR; i++) {
                    if ((fruit.position.x == wall[i].position.x) && (fruit.position.y == wall[i].position.y)) {
                        fruit.position = (Vector2){ GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (screenHeight / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2 };
                        i = 0;
                        i = i - 1;
                    }
                }
                for (int n = 0; n < players.nbrOfPlayer; n++)
                {
                    for (int i = 0; i < players.snakes[n]->counterTail; i++)
                    {
                        while ((fruit.position.x == players.snakes[n][i].position.x) && (fruit.position.y == players.snakes[n][i].position.y))
                        {
                            fruit.position = (Vector2){ GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (screenHeight / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2 };
                            i = 0;
                        }
                    }
                }
            }

            // Collision with fruit
            for (int n = 0; n < players.nbrOfPlayer; n++)
            {
                if ((players.snakes[n][0].position.x < (fruit.position.x + fruit.size.x) && (players.snakes[n][0].position.x + players.snakes[n][0].size.x) > fruit.position.x) &&
                    (players.snakes[n][0].position.y < (fruit.position.y + fruit.size.y) && (players.snakes[n][0].position.y + players.snakes[n][0].size.y) > fruit.position.y))
                {
                    players.snakes[n][players.snakes[n]->counterTail].position = players.snakes[n]->snakePosition[players.snakes[n]->counterTail - 1];
                    players.snakes[n]->counterTail += 1;
                    fruit.active = false;
                    //If playing in gameMode 2, generate new wall configuration each 10 fruits eaten
                    if (gameMode == 2 && players.snakes[0]->counterTail % 10 == 0 && players.nbrOfPlayer == 1) {
                        wall->active = false;
                        WallGeneration();
                        SpeedIncrease();
                    }
                }
            }
            //GameMode Features : wall generation
            if (gameMode == 1 || gameMode == 2)
                WallGeneration();

            framesCounter++;
        }
    }
    //GameOver page controls
    else if (!menu && gameOver)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
        else if (IsKeyPressed('E')) {
            InitGame();
            menu = true;
        }
    }
    //Menu controls
    else
    {
        if (IsKeyPressed('1'))
        {
            gameMode = 0;
            InitGame();
            menu = false;
        }
        else if (IsKeyPressed('2'))
        {
            gameMode = 1;
            InitGame();
            menu = false;
        }
        else if (IsKeyPressed('3'))
        {
            gameMode = 2;
            InitGame();
            menu = false;
        }
    }
}

// Draw game (one frame)
//---------------------------------------------------------
void DrawGame(void)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver && !menu)
    {
        // Draw grid lines
        for (int i = 0; i < screenWidth / SQUARE_SIZE + 1; i++)
        {
            DrawLineV((Vector2) { SQUARE_SIZE* i + offset.x / 2, offset.y / 2 }, (Vector2) { SQUARE_SIZE* i + offset.x / 2, screenHeight - offset.y / 2 }, LIGHTGRAY);
        }

        for (int i = 0; i < screenHeight / SQUARE_SIZE + 1; i++)
        {
            DrawLineV((Vector2) { offset.x / 2, SQUARE_SIZE* i + offset.y / 2 }, (Vector2) { screenWidth - offset.x / 2, SQUARE_SIZE* i + offset.y / 2 }, LIGHTGRAY);
        }

        //Drawn walls
        for (int i = 0; i < WALL_NBR; i++) DrawRectangleV(wall[i].position, wall[i].size, wall->color);

        // Draw snake
        for (int n = 0; n < players.nbrOfPlayer; n++) 
        {
            for (int i = 0; i < players.snakes[n]->counterTail; i++) 
            {
                DrawRectangleV(players.snakes[n][i].position, players.snakes[n][i].size, players.snakes[n][i].color);
            }
        }

        // Draw fruit to pick
        DrawRectangleV(fruit.position, fruit.size, fruit.color);

        //Draw score
        DrawText(TextFormat("Score Player 1 : %i", players.snakes[0]->counterTail), 10, (GetScreenHeight() - 20), 20, BLUE);
        if (players.nbrOfPlayer == 2)
        {
            DrawText(TextFormat("Score Player 2 : %i", players.snakes[1]->counterTail), GetScreenWidth() - MeasureText("Score Player 2 : %i", 20) - 10, (GetScreenHeight() - 20), 20, BLUE);
        }
        //Draw pause during game
        if (pause)
        {
            DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
            DrawText("PRESS [E] FOR MENU", screenWidth / 2 - MeasureText("PRESS [E] FOR MENU", 40) / 2, screenHeight / 2, 40, RED);
        }
    }
    //Draw gameOver page
    else if (!menu && gameOver) {
        DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);
        DrawText("PRESS [E] FOR MENU", GetScreenWidth() / 2 - MeasureText("PRESS [E] FOR MENU", 20) / 2, GetScreenHeight() / 2, 20, RED);
    }
    //Draw menu
    else if (menu)
    {
        DrawText("MENU", GetScreenWidth() / 2 - MeasureText("MENU", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);
        DrawText("SELECT DIFFICULTY 1 - 2 - 3", GetScreenWidth() / 2 - MeasureText("SELECT DIFFICULTY 1 - 2 - 3", 20) / 2, GetScreenHeight() / 2, 20, RED);
        DrawText(TextFormat("Last score : %i || Best score : %i", score, hiscore), GetScreenWidth() / 2 - MeasureText("Last score : 10 || Best score : 20", 20) / 2, GetScreenHeight() / 2 + 50, 20, ORANGE);
        DrawText("PRESS [ESC] TO LEAVE", GetScreenWidth() / 2 - MeasureText("PRESS [ESC] TO LEAVE", 20) / 2, GetScreenHeight() - 50, 20, DARKGRAY);
    }
    EndDrawing();
}

// Unload game variables
//---------------------------------------------------------
void UnloadGame(void)
{

}

// Update and Draw (one frame)
//---------------------------------------------------------
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}*/