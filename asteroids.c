#include <stdio.h>
#include "raylib.h"
#include <math.h>
#define WIDTH 1200
#define HEIGHT 800
#define MAX_ROCKS 12
#define MAX_LASERS 20
#define MAX_POINTS 10
#define MAX_ENEMY_BULLETS 25
#define MAX_ENEMIES 3

typedef enum GameState{ 
    STATE_GAMEPLAY, 
    STATE_GAMEOVER 
}GameState;
typedef enum{
     Menu,
     GAME,
     STATE_HIGHSCORE,
     HOWTOPLAY,
     ABOUTUS
    }GameScreen;
typedef struct Background{
    Vector2 position,velocity;
    float baseRadius;
    int pointsCount;
    float pointOffsets[MAX_POINTS];
    float rotation, rotationSpeed;
    int sizeClass;
    float depth, depthSpeed;
    bool active;
}Background;

typedef struct Player{
    Vector2 position, velocity;
    float rotation, acceleration, friction;
    int lives;
    float invulnTimer;
}Player;

typedef struct Laser{
    Vector2 position, velocity;
    float lifetime;
    float rotation;
    bool active;
}Laser;

typedef struct EnemyShip{
    Vector2 position;
    Vector2 velocity;
    float shootTimer;
    float spawnTimer;
    bool active;
}EnemyShip;

typedef struct EnemyBullet{
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    bool active;
}EnemyBullet;

int Highscore(void)
{
    int loadscore=0;
    FILE* file=fopen("highscore.txt","r");
    if(file!=NULL){
        fscanf(file,"%d",&loadscore);
        fclose(file);
    }
    return loadscore;
}
void Savehighscore(int score)
{
    FILE* file=fopen("highscore.txt","w");
    if(file!=NULL){
        fprintf(file,"%d",score);
        fclose(file);
    }
}
void SpawnRock(Background rocks[], Vector2 pos, int size, int screenWidth, int screenHeight)
{
    for (int i=0;i<MAX_ROCKS;i++)
    {
        if (!rocks[i].active)
        {
            rocks[i].position = pos;
            float angle =(float)GetRandomValue(0,360) * DEG2RAD;
            float speedModifier =(size==3)?1.0:(size==2)?1.4:2.0;
            rocks[i].velocity =(Vector2){cosf(angle) * speedModifier,sinf(angle) * speedModifier};
            rocks[i].sizeClass =size;
            rocks[i].baseRadius =(size==3)? 50.0:(size==2)? 25.0:12.0;
            rocks[i].pointsCount =MAX_POINTS;
            rocks[i].rotation =(float)GetRandomValue(0,360);
            rocks[i].rotationSpeed =(float)GetRandomValue(-20,20)/30.0;
            rocks[i].depth =(float)GetRandomValue(50,130)/100.0;
            rocks[i].depthSpeed =((float)GetRandomValue(2,6)/1000.0)*(GetRandomValue(0,1)?1:-1);
            rocks[i].active =true;
            for (int p=0;p<MAX_POINTS;p++){
                rocks[i].pointOffsets[p] =(float)GetRandomValue(-50,50)/100.0;
            }
            break;
        }
    }
}

void ResetAsteroids(Background rocks[],int count,int screenWidth,int screenHeight,Vector2 playerPos)
{
    for (int i=0;i<MAX_ROCKS;i++){
        rocks[i].active=false;
    }

    int maxSpawn =(count>MAX_ROCKS) ? MAX_ROCKS:count;

    for (int i=0;i<maxSpawn;i++)
    {
        rocks[i].position =(Vector2){(float)GetRandomValue(0,screenWidth),(float)GetRandomValue(0,screenHeight)};
        while (CheckCollisionCircles(playerPos,120,rocks[i].position,45))
        {
        rocks[i].position =(Vector2){(float)GetRandomValue(0,screenWidth),(float)GetRandomValue(0,screenHeight)};
        }
        rocks[i].velocity =(Vector2){(float)GetRandomValue(-100, 100)/60.0,(float)GetRandomValue(-100,100)/60.0};

        if (rocks[i].velocity.x==0 && rocks[i].velocity.y==0){
         rocks[i].velocity =(Vector2){1.0,-1.0};
        }
        rocks[i].sizeClass =3;
        rocks[i].baseRadius =65.0;
        rocks[i].pointsCount =MAX_POINTS;
        rocks[i].rotation =(float)GetRandomValue(0,360);
        rocks[i].rotationSpeed =(float)GetRandomValue(-40,40)/30.0;
        rocks[i].depth =(float)GetRandomValue(50,130)/100.0;
        rocks[i].depthSpeed =((float)GetRandomValue(2,6)/1000.0)*(GetRandomValue(0,1)?1:-1);
        rocks[i].active =true;
        for (int p=0;p<MAX_POINTS;p++){
            rocks[i].pointOffsets[p] =(float)GetRandomValue(-30,30)/70.0;
        }

    }

}
void RespawnAsteroids(Background rocks[],int screenWidth,int screenHeight,Vector2 playerpos)
{
    bool asteroidleft=false;
    for(int i=0;i<MAX_ROCKS;i++){
        if(rocks[i].active){
            asteroidleft=true;
            break;
        }

    }
    if(!asteroidleft){
        ResetAsteroids(rocks,8,screenWidth,screenHeight,playerpos);
    }

}
float RandomSpawnDelay(void) {
    return 12.0+(float)GetRandomValue(0,80)/10.0;
}

void ResetEnemies(EnemyShip e[], EnemyBullet b[]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        e[i].active = false;
        e[i].spawnTimer = 3.0f + i * 4.0f + (float)GetRandomValue(0, 30) / 10.0f;
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        b[i].active = false;
    }
}

void DrawEnemyShip(Vector2 p) {
    DrawCircleSectorLines((Vector2){p.x, p.y - 4}, 20, 180, 360, 24, WHITE);
    DrawEllipseLines((int)p.x, (int)p.y, 46, 13, WHITE);
    DrawEllipseLines((int)p.x, (int)p.y + 9, 24, 7, WHITE);
    for (int i = -2; i <= 2; i++)
        DrawCircle((int)(p.x + i * 16), (int)(p.y + 3), 2.5f, WHITE);
    DrawLine((int)p.x - 14, (int)p.y + 15, (int)p.x - 22, (int)p.y + 24, WHITE);
    DrawLine((int)p.x + 14, (int)p.y + 15, (int)p.x + 22, (int)p.y + 24, WHITE);
}

        int main()
        {
            const int screenWidth =WIDTH,screenHeight = HEIGHT;
            InitWindow(WIDTH,HEIGHT,"ASTEROIDS");
            InitAudioDevice();
            Sound crashSound = LoadSound("resources/crash.wav");
            Sound lasersound= LoadSound("resources/laser.mp3");
            Music bgMusic = LoadMusicStream("resources/background.mp3");
            Texture2D mehediPhoto = LoadTexture("resources/Tahmid.png");
Texture2D hridoyPhoto = LoadTexture("resources/Hridoy.png");
            
            PlayMusicStream(bgMusic);
                SetMusicVolume(bgMusic,0.8);
                SetTargetFPS(60);
            GameScreen currentscreen =Menu;
                Rectangle playbutton ={WIDTH/2.0-210,400,320,80};
                Rectangle highscorebutton ={WIDTH/2.0-210,500,320,80};
                Rectangle howtoplaybutton ={WIDTH/2.0-210,600,320,80}; 
                Rectangle backbutton ={WIDTH/2.0-150,680,300,60};
                Rectangle aboutbutton = {WIDTH / 2.0 - 210, 690, 320, 80};
                GameState currentState =STATE_GAMEPLAY;

        int score=0;
        float difficultytimer=0.0;
        int highscore=Highscore();
        bool isnewhighscore=false;
        EnemyShip enemies[MAX_ENEMIES]={0};
        EnemyBullet enemyBullets[MAX_ENEMY_BULLETS] = {0};
        ResetEnemies(enemies, enemyBullets);

        Player player = {0};
            player.position =(Vector2){(float)screenWidth/2,(float)screenHeight/1.5};
            player.velocity =(Vector2){0, 0};
            player.rotation =0.0;
            player.acceleration =0.07;
            player.friction =0.99;
            player.lives =3;
            player.invulnTimer =1.0;
            Background rocks[MAX_ROCKS] ={0};
            Laser lasers[MAX_LASERS] ={0};
        ResetAsteroids(rocks,8,screenWidth,screenHeight,player.position);
        while (!WindowShouldClose())
            {
                float dt =GetFrameTime();
                UpdateMusicStream(bgMusic);
                if(currentscreen==GAME && currentState==STATE_GAMEPLAY){
                    difficultytimer+=dt;
                }
                float speedmultiplier=1.0+(difficultytimer*0.004);
                if(speedmultiplier>3.0){
                    speedmultiplier=3.0;
                }
        for (int i=0;i<MAX_ROCKS;i++)
        {
            if(rocks[i].active)
            {
                rocks[i].position.x+=rocks[i].velocity.x*speedmultiplier;
                rocks[i].position.y+=rocks[i].velocity.y*speedmultiplier;
                rocks[i].rotation+=rocks[i].rotationSpeed;
                rocks[i].depth+=rocks[i].depthSpeed;
                if(rocks[i].depth>1.3 || rocks[i].depth<0.5)
                    rocks[i].depthSpeed=-rocks[i].depthSpeed;
            if(rocks[i].position.x>screenWidth+rocks[i].baseRadius)
                    rocks[i].position.x=-rocks[i].baseRadius;
            else if (rocks[i].position.x<-rocks[i].baseRadius)
                    rocks[i].position.x=screenWidth+rocks[i].baseRadius;

            if (rocks[i].position.y>screenHeight+rocks[i].baseRadius)
                    rocks[i].position.y=-rocks[i].baseRadius;
            else if (rocks[i].position.y<-rocks[i].baseRadius)
                    rocks[i].position.y=screenHeight+rocks[i].baseRadius;
            }
        }
        if (currentscreen==Menu)
        {
            
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, playbutton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                currentscreen =GAME;
                currentState =STATE_GAMEPLAY;
                difficultytimer=0.0;
                player.position =(Vector2){(float)screenWidth/2,(float)screenHeight/1.5};
                player.velocity =(Vector2){0,0};
                player.rotation =0.0;
                player.lives =3;
                player.invulnTimer =1.0;
                score =0;
                isnewhighscore=false;
                    for (int i=0;i<MAX_LASERS;i++){
                        lasers[i].active =false;
                    }
                    ResetEnemies(enemies,enemyBullets);
                        ResetAsteroids(rocks,8,screenWidth,screenHeight,player.position);
                    
            }
            else if(CheckCollisionPointRec(mouse,highscorebutton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                currentscreen=STATE_HIGHSCORE;
            }
            else if (CheckCollisionPointRec(mouse,howtoplaybutton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                currentscreen=HOWTOPLAY;
            }
            else if (CheckCollisionPointRec(mouse, aboutbutton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            currentscreen=ABOUTUS;
    }
        }
        else if(currentscreen==STATE_HIGHSCORE)
        {
            if(IsKeyPressed(KEY_B)||IsKeyPressed(KEY_ESCAPE)){
                currentscreen=Menu;
            }
        }
        else if(currentscreen==HOWTOPLAY) 
        { 
            if(IsKeyPressed(KEY_B)||IsKeyPressed(KEY_ESCAPE)) 
                currentscreen=Menu; 

            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(),backbutton)) 
                currentscreen=Menu; 
        }
        else if (currentscreen == ABOUTUS){
        if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)){
        currentscreen = Menu;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), backbutton)) {
        currentscreen = Menu;
        }
        }
        else if(currentscreen==GAME)
        {
            switch(currentState)
            {
                case STATE_GAMEPLAY:
                    if (player.invulnTimer>0.0)
                        player.invulnTimer-=dt;

                    if(IsKeyDown(KEY_LEFT))
                        player.rotation-=240.0*dt;

                    if(IsKeyDown(KEY_RIGHT))
                        player.rotation+=240.0*dt;

                    if(IsKeyDown(KEY_UP))
                    {
                        player.velocity.x+=sinf(player.rotation*DEG2RAD)*player.acceleration;
                        player.velocity.y-=cosf(player.rotation*DEG2RAD)*player.acceleration;
                    }
                    if(IsKeyDown(KEY_A))
                    {
                        player.velocity.x-=cosf(player.rotation*DEG2RAD)*player.acceleration;
                        player.velocity.y-=sinf(player.rotation*DEG2RAD)*player.acceleration;
                    }

                    if(IsKeyDown(KEY_D))
                    {
                        player.velocity.x+=cosf(player.rotation*DEG2RAD)*player.acceleration;
                        player.velocity.y+=sinf(player.rotation*DEG2RAD)*player.acceleration;
                    }

                    player.velocity.x*=player.friction;
                    player.velocity.y*=player.friction;

                    player.position.x+= player.velocity.x;
                    player.position.y+= player.velocity.y;

                    if(player.position.x>screenWidth)
                        player.position.x =0;
                    else if(player.position.x<0)
                        player.position.x =(float)screenWidth;

                    if(player.position.y>screenHeight)
                        player.position.y =0;
                    else if(player.position.y<0)
                        player.position.y =(float)screenHeight;

                    if(IsKeyPressed(KEY_SPACE))
                   
                    {
                        for (int i=0;i<MAX_LASERS;i++)
                        {
                            if (!lasers[i].active)
                            {
                                lasers[i].position.x =player.position.x+sinf(player.rotation*DEG2RAD);
                                lasers[i].position.y =player.position.y-cosf(player.rotation*DEG2RAD);
                                float angle =player.rotation*DEG2RAD;
                                float laserSpeed =10.0;
                                lasers[i].velocity.x =sinf(angle)*laserSpeed;
                                lasers[i].velocity.y =-cosf(angle)*laserSpeed;
                                lasers[i].rotation=angle;
                                lasers[i].lifetime =1.0;
                                lasers[i].active =true;
                                PlaySound(lasersound);
                                break;
                            }
                        }
                    }
                    for(int i=0;i<MAX_LASERS;i++)
                    {
                        if(lasers[i].active)
                        {
                            lasers[i].position.x+=lasers[i].velocity.x;
                            lasers[i].position.y+=lasers[i].velocity.y;
                            lasers[i].lifetime-=dt;

                            if(lasers[i].lifetime<=0.0)
                                lasers[i].active =false;
                            if(lasers[i].position.x>screenWidth)
                                lasers[i].position.x =0;
                            else if(lasers[i].position.x<0)
                                lasers[i].position.x =(float)screenWidth;
                            if(lasers[i].position.y>screenHeight)
                                lasers[i].position.y =0;
                            else if(lasers[i].position.y<0)
                                lasers[i].position.y =(float)screenHeight;
                        }
                    }
                    for(int m=0;m<MAX_LASERS;m++)
                    {
                        if(!lasers[m].active)
                            continue;
                    
                        for(int j=0;j<MAX_ROCKS;j++)
                        {
                            if(!rocks[j].active)
                                continue;
                               
                            if(CheckCollisionCircles(lasers[m].position,2.0,rocks[j].position,rocks[j].baseRadius*rocks[j].depth))
                            {
                                lasers[m].active =false;
                                PlaySound(crashSound);
                                if(rocks[j].sizeClass==3)
                                    score+=20;
                                else if(rocks[j].sizeClass==2)
                                    score+=50;
                                else if(rocks[j].sizeClass==1)
                                    score+=90;
                                if (rocks[j].sizeClass>1)
                                {
                                    int nextSize =rocks[j].sizeClass-1;
                                    SpawnRock(rocks,rocks[j].position,nextSize,screenWidth,screenHeight);
                                    SpawnRock(rocks,rocks[j].position,nextSize,screenWidth,screenHeight);
                                }
                                rocks[j].active = false;
                                break;
                            }
                        }
                    }
                    if(player.invulnTimer<=0.0)
                    {
                        for(int i=0;i<MAX_ROCKS;i++)
                        {
                            if(!rocks[i].active)
                                continue;
                            if(CheckCollisionCircles(player.position,8.0,rocks[i].position,rocks[i].baseRadius*rocks[i].depth))
                            {
                                int hit =rocks[i].sizeClass;
                                Vector2 hitPos =rocks[i].position;
                                rocks[i].active =false;
                                if(hit==3)
                                    score+=20;
                                else if(hit==2)
                                    score+=50;
                                else if(hit==1)
                                    score+=90;
                                if(hit>1)
                                {
                                    int nextSize =hit-1;
                                    SpawnRock(rocks,hitPos,nextSize,screenWidth,screenHeight);
                                    SpawnRock(rocks,hitPos,nextSize,screenWidth,screenHeight);
                                }
                                player.lives--;
                                player.invulnTimer =1.0;
                                PlaySound(crashSound);
                                player.position =(Vector2){(float)screenWidth/2,(float)screenHeight/1.5};
                                player.velocity =(Vector2){0,0};
                                if(player.lives==0){
                                    currentState =STATE_GAMEOVER;
                                    if(score>highscore){
                                        isnewhighscore=true;
                                        highscore=score;
                                        Savehighscore(highscore);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    for(int e=0;e<MAX_ENEMIES;e++){
                        EnemyShip *en=&enemies[e];
                        if(!en->active){
                            en->spawnTimer-=dt;
                            if(en->spawnTimer<=0.0) {
                                bool fromLeft=GetRandomValue(0, 1)==0;
                                en->position=(Vector2){fromLeft ? -60.0 : screenWidth+60.0,(float)GetRandomValue(80,screenHeight-80)};
                                en->velocity=(Vector2){fromLeft ? 2.0 : -2.0, 0};
                                en->shootTimer=1.0;
                                en->active=true;
                            }
                            continue;
                        }
                        
                        en->position.x+=en->velocity.x;
                        en->position.y+=en->velocity.y;
                        
                        if(GetRandomValue(0,80)==0){
                            en->velocity.y=(float)GetRandomValue(-1,1)*1.2;
                        }
                        if(en->position.y<40){
                            en->position.y=40;
                        }
                        if(en->position.y>screenHeight-40){
                        en->position.y=screenHeight-40;
                        } 
                            
                        en->shootTimer-=dt;
                        if(en->shootTimer<=0.0 && en->position.x>0 && en->position.x<screenWidth){
                            for (int i=0;i<MAX_ENEMY_BULLETS;i++){
                                if(!enemyBullets[i].active){
                                    float a=atan2f(player.position.y-en->position.y,player.position.x-en->position.x)+(float)GetRandomValue(-12,12)*DEG2RAD;
                                    enemyBullets[i].position=(Vector2){en->position.x,en->position.y+12};
                                    enemyBullets[i].velocity=(Vector2){cosf(a)*4.0,sinf(a)*4.0};
                                    enemyBullets[i].lifetime=3.0;
                                    enemyBullets[i].active=true;
                                    break;
                                }
                            }
                            en->shootTimer=1.5;
                        }
                        
                        if((en->velocity.x>0 && en->position.x>screenWidth+70) || (en->velocity.x<0 && en->position.x<-70)){
                            en->active=false;
                            en->spawnTimer=RandomSpawnDelay();
                        }
                        
                        for (int m = 0; m < MAX_LASERS; m++) {
                            if (lasers[m].active && CheckCollisionCircles(lasers[m].position, 2.0f, en->position, 38.0f)) {
                                lasers[m].active = false;
                                en->active = false;
                                en->spawnTimer = RandomSpawnDelay();
                                score += 50;
                                PlaySound(crashSound);
                                break;
                            }
                        }
                    }

                    for(int i=0;i<MAX_ENEMY_BULLETS;i++){
                        EnemyBullet *b=&enemyBullets[i];
                        if(!b->active)
                            continue;
                        b->position.x+=b->velocity.x;
                        b->position.y+=b->velocity.y;
                        b->lifetime-=dt;
                        if(b->lifetime<=0.0 || b->position.x< 0 || b->position.x > screenWidth || b->position.y < 0 || b->position.y > screenHeight) {
                            b->active = false;
                        } 
                        else if (player.invulnTimer <= 0.0f && CheckCollisionCircles(player.position, 8.0f, b->position, 3.0f)) {
                            b->active = false;
                            player.lives--;
                            player.invulnTimer=1.0;
                            PlaySound(crashSound);
                            player.position = (Vector2){(float)screenWidth / 2, (float)screenHeight / 1.5};
                            player.velocity = (Vector2){0, 0};
                            if (player.lives == 0){
                                currentState = STATE_GAMEOVER;
                                if(score>highscore){
                                    isnewhighscore=true;
                                    highscore=score;
                                    Savehighscore(highscore);
                                }
                            }
                        }
                    }

                    RespawnAsteroids(rocks,screenWidth,screenHeight,player.position);
                    if(IsKeyPressed(KEY_B))
                        currentscreen =Menu;
                    break;
                case STATE_GAMEOVER:
                    if(IsKeyPressed(KEY_ENTER))
                    {
                        difficultytimer=0.0;
                       player.position =(Vector2){(float)screenWidth/2,(float)screenHeight/1.5};
                        player.velocity =(Vector2){0,0};
                        player.rotation =0.0;
                        player.lives = 3;
                        player.invulnTimer =1.0;
                        score = 0;
                        isnewhighscore=false;
                        for(int k=0;k<MAX_LASERS;k++){
                            lasers[k].active = false;
                        }
                        ResetEnemies(enemies,enemyBullets);
                        ResetAsteroids(rocks,8,screenWidth,screenHeight,player.position);
                        currentState = STATE_GAMEPLAY;
                    }
                    
                    if(IsKeyPressed(KEY_B))
                        currentscreen = Menu;
                    break;
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);
        
        if(currentscreen==Menu)
        {
            DrawText("ASTEROIDS",WIDTH/2-MeasureText("ASTEROIDS",70)/2,100,90, WHITE);
            
            for(int i=0;i<MAX_ROCKS;i++)
            {
                if(rocks[i].active)
                {
                    Vector2 points[MAX_POINTS];
                    float angleStep =360.0/rocks[i].pointsCount;
                    for(int p =0;p<rocks[i].pointsCount;p++)
                    {
                        float angle =(rocks[i].rotation+(p*angleStep))*DEG2RAD;
                        float radius =rocks[i].baseRadius*rocks[i].depth*(1.0 + rocks[i].pointOffsets[p]);
                        points[p].x =rocks[i].position.x+cosf(angle)*radius;
                        points[p].y =rocks[i].position.y+sinf(angle)*radius;
                    }
                    for(int p =0;p<rocks[i].pointsCount;p++){
                        DrawLineV(points[p],points[(p+1)%rocks[i].pointsCount], WHITE);
                    }
                }
            }
            DrawRectangleLinesEx(playbutton,2,WHITE);
            DrawText("PLAY GAME",playbutton.x+playbutton.width/2-MeasureText("PLAY GAME",30)/2,playbutton.y+25,30,WHITE);
            DrawRectangleLinesEx(highscorebutton,3,WHITE);
            DrawText("HIGH SCORES",highscorebutton.x+highscorebutton.width/2-MeasureText("HIGH SCORES",30)/2,highscorebutton.y+25,30,WHITE);
            DrawRectangleLinesEx(howtoplaybutton,3,WHITE);
            DrawText("HOW TO PLAY",howtoplaybutton.x+howtoplaybutton.width/2-MeasureText("HOW TO PLAY",30)/2,howtoplaybutton.y+25,30,WHITE);
            DrawRectangleLinesEx(aboutbutton, 3, WHITE);
            DrawText("ABOUT US", aboutbutton.x + aboutbutton.width / 2 - MeasureText("ABOUT US", 30) / 2, aboutbutton.y + 25, 30, WHITE);
        }
        else if(currentscreen==STATE_HIGHSCORE){
            DrawText("HIGH SCORES", WIDTH/2 - MeasureText("HIGH SCORES", 60)/2, 150, 60, WHITE);
            
            const char* highscoreText = TextFormat("RECORD: %d PTS", highscore);
            DrawText(highscoreText, WIDTH/2 - MeasureText(highscoreText, 40)/2, HEIGHT/2 - 20, 40, YELLOW);
            
            DrawText("PRESS 'B' TO RETURN TO MAIN MENU", WIDTH/2 - MeasureText("PRESS 'B' TO RETURN TO MAIN MENU", 20)/2, HEIGHT - 150, 20, GRAY);
        }

        else if(currentscreen==HOWTOPLAY) 
        { 
            DrawText("HOW TO PLAY",WIDTH/2-MeasureText("HOW TO PLAY",60)/2,70,60,WHITE); 

            DrawText("RULES",100,170,35,RED); 
            DrawText("1. Destroy asteroids to earn points.",100,220,22,WHITE); 
            DrawText("2. Large asteroids split into smaller ones.",100,260,22,WHITE); 
            DrawText("3. Destroy enemy ships for 50 points.",100,300,22,WHITE); 
            DrawText("4. Avoid asteroids and enemy bullets.",100,340,22,WHITE); 
            DrawText("5. You have 3 lives.",100,380,22,WHITE); 
            DrawText("6. Game ends when all lives are lost.",100,420,22,WHITE); 

            DrawText("CONTROLS",700,170,35,RED); 
            DrawText("LEFT / RIGHT - Rotate",700,220,22,WHITE); 
            DrawText("UP - Move Forward",700,260,22,WHITE); 
            DrawText("A - Move Left",700,300,22,WHITE); 
            DrawText("D - Move Right",700,340,22,WHITE); 
            DrawText("SPACE - Fire Laser",700,380,22,WHITE); 

            DrawRectangleLinesEx(backbutton,2,WHITE); 
            DrawText("BACK TO MENU",backbutton.x+backbutton.width/2-MeasureText("BACK TO MENU",24)/2,backbutton.y+18,24,WHITE); 
            DrawText("B / ESC = BACK",WIDTH/2-MeasureText("B / ESC = BACK",18)/2,755,18,GRAY); 
        } 

        else if (currentscreen == ABOUTUS){
        DrawText("ABOUT US", WIDTH / 2 - MeasureText("ABOUT US", 60) / 2, 55, 60, WHITE);
        Rectangle sourceMehedi = {0, 0, (float)mehediPhoto.width, (float)mehediPhoto.height};
        Rectangle destMehedi = {170, 140, 300, 300};

        DrawTexturePro(mehediPhoto, sourceMehedi, destMehedi, (Vector2){0, 0}, 0, WHITE);

        Rectangle sourceHridoy = {0, 0, (float)hridoyPhoto.width, (float)hridoyPhoto.height};
        Rectangle destHridoy = {730, 140, 300, 300};

        DrawTexturePro(hridoyPhoto, sourceHridoy, destHridoy, (Vector2){0, 0}, 0, WHITE);
       
        DrawText("Md Mehedi Hasan Tahmid", 140, 475, 30, WHITE);
        DrawText("tahmidmehedihasan@gmail.com", 145, 515, 25, YELLOW);

        DrawText("Hridoy Banik", 805, 475, 30, WHITE);
        DrawText("hridoyanup316@gmail.com", 750, 515, 25, YELLOW);

        DrawRectangleLinesEx(backbutton, 2, WHITE);
        DrawText("BACK TO MENU", backbutton.x + backbutton.width / 2 - MeasureText("BACK TO MENU", 24) / 2, backbutton.y + 18, 24, WHITE);
        DrawText("B / ESC = BACK", WIDTH / 2 - MeasureText("B / ESC = BACK", 18) / 2, 755, 18, GRAY);
    }
        else if(currentscreen==GAME)
        {
            for(int i=0;i<MAX_ROCKS;i++)
            {
                if(rocks[i].active)
                {
                    Vector2 points[MAX_POINTS];
                    float angleStep =360.0/rocks[i].pointsCount;
                    for(int p =0;p<rocks[i].pointsCount;p++)
                    {
                        float angle =(rocks[i].rotation+(p*angleStep))*DEG2RAD;
                        float radius =rocks[i].baseRadius*rocks[i].depth*(1.0+rocks[i].pointOffsets[p]);
                        points[p].x =rocks[i].position.x+cosf(angle)*radius;
                        points[p].y =rocks[i].position.y+sinf(angle)*radius;
                    }
                    for(int p=0;p<rocks[i].pointsCount;p++)
                        DrawLineV(points[p],points[(p + 1) % rocks[i].pointsCount], WHITE);
                }
            }
            for(int i=0;i<MAX_LASERS;i++)
            {
                if(lasers[i].active)
                {
                    float beamLength=10.0; 
                    Vector2 startPos={
                    lasers[i].position.x-sinf(lasers[i].rotation)*(beamLength/2.0),
                    lasers[i].position.y+cosf(lasers[i].rotation)*(beamLength/2.0)
                    };
                    Vector2 endPos={
                    lasers[i].position.x+sinf(lasers[i].rotation)*(beamLength/2.0),
                    lasers[i].position.y-cosf(lasers[i].rotation)*(beamLength/2.0)
                    };        
                    DrawLineEx(startPos,endPos,2.0,WHITE);
                }
            }

            for(int i=0;i<MAX_ENEMIES;i++){
                if (enemies[i].active){
                    DrawEnemyShip(enemies[i].position);
                }
            }

        
            for(int i=0;i<MAX_ENEMY_BULLETS;i++) 
            {
                if(enemyBullets[i].active)
                {
                    float angle=atan2f(enemyBullets[i].velocity.y,enemyBullets[i].velocity.x);
                    float beamLength=8.0;
                    Vector2 startpos={
                        enemyBullets[i].position.x-cosf(angle)*(beamLength/2.0),
                        enemyBullets[i].position.y-sinf(angle)*(beamLength/2.0)
                    };
                    Vector2 endpos={
                        enemyBullets[i].position.x+cosf(angle)*(beamLength/2.0),
                        enemyBullets[i].position.y+sinf(angle)*(beamLength/2.0)
                    };
                    DrawLineEx(startpos,endpos,2.0,RED);
                }
            }

            if(currentState==STATE_GAMEPLAY)
            {
                DrawText(TextFormat("SCORE:%d",score),30,30,26,WHITE);
                DrawText(TextFormat("High Score: %d",highscore),30,70,20,WHITE);
                for(int i=0;i<player.lives;i++)
                {
                    Vector2 lifePos ={35.0+(i*30),105.0};
                    DrawTriangleLines(
                        (Vector2){lifePos.x,lifePos.y-15},
                        (Vector2){lifePos.x-10,lifePos.y+10},
                        (Vector2){lifePos.x+10,lifePos.y+10},
                        WHITE);
                }
                
                    Vector2 nose={
                        player.position.x+sinf(player.rotation*DEG2RAD)*30,
                        player.position.y-cosf(player.rotation*DEG2RAD)*30 
                    };
                    Vector2 leftBack={
                        player.position.x+sinf((player.rotation-140)*DEG2RAD)*30,
                        player.position.y-cosf((player.rotation-140)*DEG2RAD)*30
                    };
                    Vector2 rightBack={
                        player.position.x+sinf((player.rotation+140)*DEG2RAD)*30,
                        player.position.y-cosf((player.rotation+140)*DEG2RAD)*30
                    };
                    Vector2 leftMid={nose.x*0.3+leftBack.x*0.7,nose.y*0.3+leftBack.y*0.7};
                    Vector2 rightMid={nose.x*0.3+rightBack.x*0.7,nose.y*0.3+rightBack.y*0.7};
                    DrawLineV(nose,leftBack,WHITE);
                    DrawLineV(nose,rightBack,WHITE);
                    DrawLineV(leftMid,rightMid,WHITE);
                
                DrawText("B = MENU",WIDTH-120,30,18,GRAY);
            }
            else if(currentState==STATE_GAMEOVER)
            {
                DrawText("GAME OVER",screenWidth/2-MeasureText("GAME OVER",70)/2,screenHeight/2-70,70,RED);
                DrawText(TextFormat("FINAL SCORE: %d",score),screenWidth/2-MeasureText(TextFormat("FINAL SCORE: %d",score),25)/2,screenHeight/2+10,30,WHITE);
                if(isnewhighscore){
                    DrawText("Congrats!!New High Score",screenWidth/2.0-MeasureText("Congrats!!New High Score",50)/2.0,screenHeight/2.0+50,50,RED);
                }
                DrawText("PRESS ENTER TO RESTART",screenWidth/2-MeasureText("PRESS ENTER TO RESTART",30)/2,screenHeight/2+120,30,GRAY);

            }
            
        }
        EndDrawing();
    }
    UnloadTexture(mehediPhoto);
    UnloadTexture(hridoyPhoto);
    UnloadSound(crashSound);
    UnloadSound(lasersound);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
