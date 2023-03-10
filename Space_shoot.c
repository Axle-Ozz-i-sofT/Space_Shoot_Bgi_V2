//------------------------------------------------------------------------------
// Name:        Space_shoot.c
// Purpose:     Example Game using SDL-BGI graphics.h
// Title:       "SDL-BGI_Space_Shoot_fps"
//
// Platform:    Win64, Ubuntu64, (No VirtualBox client)
//
// Compiler:    GCC V9.x.x, MinGw-64, libc (ISO C99)
// Depends:     SDL2-devel, SDL_bgi-3.0.0, WaveBeep.exe
//
// Author:      Axle
// Created:     24/02/2023
// Updated:     09/03/2023
// Version:     0.2.3.0 beta
// Copyright:   (c) Axle 2023
// Licence:     MIT No Attribution (MIT-0)
// Please see Credits.txt for MMedia and Artwork licence.
//------------------------------------------------------------------------------
// NOTES:
// This is a rewrite of V 0.1.0.0 to add a FPS ticks limiter as well as clean
// up some of the previous game logic.
// FPS will be limited to between 10<->60 FPS, 10/20 FPS for slow machines.
//
// This will not run well on a VirtualBox guests.
//
// At this time the keyboard repeat delay is enabled. In previous SDL we could
// Control this with SDL_EnableKeyRepeat(), but it does not exist in SDL2.
// I will have to re-impliment the keyboard input handling routines to test for
// a state of keydown, plus the key. Untill then there will be a slight delay
// when holding down the movement keys.
// UPDATE:
// I have created a partial fix using xkbhit() but sometimes the repeat still
// sticks.
// Implimented a workaround using xkbkit() to reset the event buffer, but
// still has some glitches.
// UPDATE2:
// // ## !! This seams to be compile time random issue?? !! ###
// It may be an SDL2 bug. can't figure it out.
// Just turn on or off each xkbhit() untill you get an OK compile.
//
// Game audio:
// I made a version using threads and the CLI tool "WavBeep.exe". A simple
// command live tool to play a wave using system("WavBeep.exe .\filname.wav")
// WavBeep.exe makes use of the Win-API PlaySound().
// In Linux versions I will use the commandline "aplay" tool.
// Other similar tools:
// "webmdshow-1.0.4.1" play webm files (includes video) Uses a playback window.
// https://www.webmproject.org/
// http://downloads.webmproject.org/releases/webm/index.html
// FFMpeg, NirCmd, sounder, sWavPlayer, ++
// This is a poor way to use audio in a modern game, but done in an attempt to
// stay within the capability of SDL_BGI.
//
// As I don't have direct control over the external CLI applications I have to
// "Kill" the sound player applications to end the additional threads.
//
// TODO:
// Rewrite shoot random generator (Too much entropy). [DONE - monitoring]
// Enemy shoot is not random!!! is not increasing fro UFO [DONE - monitoring]
// Slow down shoot intervals at screen lower. [Done - monitoring]
// Set some int MAX limits for safety.
// Create Linux (Ubuntu) version. [DONE - in testing]
// Tidy up some routines. Use logig FLAGS. [Mostly Done]
// Tweak game logic speed. [Done - Monitoring]
// level period [Done - monitor and adjust]
// level transition. [Future]
// Fix fire logic. Sstop fire when enemy hit. [Monitoring]
// Occasionally I get a random bullet from an enemy (Invisible?) [Monitoring]
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <graphics.h>
//#include "SDL2/SDL.h"
#include <SDL2/SDL_keyboard.h>

// Turn off compiler warnings for unused variables between (Windows/Linux etc.)
#define unused(x) (x) = (x)

// Most of the game logic is currently locked to the following dimentions.
// do not change them. I have not implimented screen resizing.
#define SCREEN_W 800  // 0 to 799
#define SCREEN_H 600  // 0 to 599

#define SPRITE_W 78  // 0 to 77
#define SPRITE_H 58  // 0 to 57

// Global declarations
int stop;  // FLAG Global Ends main routine.
int Game_Over = 0;  // Hero Game Over flag.

// Basic (x,y) coord structure
typedef struct coord_xy
    {
    int x;  // x
    int y;  // y
    } Coord_xy;

// Function declarations
int Fire_Sound(void *arg);
int Efire_Sound(void *arg);
int UFOfire_Sound(void *arg);
int Explode_Sound(void *arg);
int Level_Up_Sound(void *arg);
int Bonus_Sound(void *arg);
int Warpin_Sound(void *arg);
int Warpout_Sound(void *arg);
int Outro_sound(void *arg);
int Intro_Sound(void *arg);
int Drum_Sound(void *arg);
int Background_Sound(void *arg);
int BgiMessageBox(char * title, char *msgstr);

int main(int argc, char *argv[])
    {
    unused(argc);  // Turns off the compiler warning for unused argc, argv
    unused(argv);  // Turns off the compiler warning for unused argc, argv

    // Non standard BGI function to try and stop repeat key sticking.
    SDL_Event sdl_ev;

    // Import resources into application memory
    // All image resources are imported into memory. This makes image loading
    // faster than reading from disk on every CPU cycle. CPU cycles are limited
    // to aproximately 0.001 seconds so as not to race the CPU at full speed.
    //==========================================================================
    unsigned int ImgSize = 0;  // For creating malloc()
    int i = 0;  // counter for array builing

    // Arrays to hold image data.
    void *Back_n[10];
    void *Hero;
    void *Enemy_n[10];
    void *Explode_n[10];
    void *Warp_n[10];

    // Resource file paths.
    char *f_space[10] = {"./assets/Space/Back_0_800x600.bmp",
                         "./assets/Space/Back_1_800x600.bmp",
                         "./assets/Space/Back_2_800x600.bmp",
                         "./assets/Space/Back_3_800x600.bmp",
                         "./assets/Space/Back_4_800x600.bmp",
                         "./assets/Space/Back_5_800x600.bmp",
                         "./assets/Space/Back_6_800x600.bmp",
                         "./assets/Space/Back_7_800x600.bmp",
                         "./assets/Space/Back_8_800x600.bmp",
                         "./assets/Space/Back_9_800x600.bmp",
                        };

    char *f_playerShip[1] = {"./assets/Player/playerShip3_red_78x58.bmp"};

    char *f_enemyShip[10] = {"./assets/Enemy/enemyBlack2_78x58.bmp",
                             "./assets/Enemy/enemyBlue2_78x58.bmp",
                             "./assets/Enemy/enemyBlack1_78x58.bmp",
                             "./assets/Enemy/enemyRed2_78x58.bmp",
                             "./assets/Enemy/enemyGreen2_78x58.bmp",
                             "./assets/Enemy/enemyBlue1_78x58.bmp",
                             "./assets/Enemy/enemyGreen1_78x58.bmp",
                             "./assets/Enemy/enemyRed1_78x58.bmp",
                             "./assets/Enemy/enemyRed4_78x58.bmp",
                             "./assets/Enemy/ufoYellow_78x58.bmp",
                            };

    char *f_explode[10] = {"./assets/Explode/bubble_explo1_78x58.bmp",
                           "./assets/Explode/bubble_explo2_78x58.bmp",
                           "./assets/Explode/bubble_explo3_78x58.bmp",
                           "./assets/Explode/bubble_explo4_78x58.bmp",
                           "./assets/Explode/bubble_explo5_78x58.bmp",
                           "./assets/Explode/bubble_explo6_78x58.bmp",
                           "./assets/Explode/bubble_explo7_78x58.bmp",
                           "./assets/Explode/bubble_explo8_78x58.bmp",
                           "./assets/Explode/bubble_explo9_78x58.bmp",
                           "./assets/Explode/bubble_explo10_78x58.bmp"
                          };

    char *f_warp[10] = {"./assets/Warp/warp0_78x58.bmp",
                        "./assets/Warp/warp1_78x58.bmp",
                        "./assets/Warp/warp2_78x58.bmp",
                        "./assets/Warp/warp3_78x58.bmp",
                        "./assets/Warp/warp4_78x58.bmp",
                        "./assets/Warp/warp5_78x58.bmp",
                        "./assets/Warp/warp6_78x58.bmp",
                        "./assets/Warp/warp7_78x58.bmp",
                        "./assets/Warp/warp8_78x58.bmp",
                        "./assets/Warp/warp9_78x58.bmp"
                       };


    // Get Sprite image to RAM
    // Set the SDL windows options.
    setwinoptions("Load image to RAM 78x58", // char *title
                  SDL_WINDOWPOS_CENTERED, // int x
                  SDL_WINDOWPOS_CENTERED, // int y
                  SDL_WINDOW_HIDDEN);  // -1 | SDL_WINDOW_HIDDEN

    int Win_ID_0 = initwindow(SPRITE_W, SPRITE_H);  // 78x58

    //==========================================================================
    // Reading an image file as a once off outside of a loop is convenient, but
    // best not used inside of a main loop as it reads and loads from the drive.
    //void readimagefile (char *filename, int x1, int y1, int x2, int y2 );
    //Reads a .bmp file and displays it immediately at (x1, y1 ).
    // If (x2, y2 ) are not 0, the bitmap is stretched to fit the
    // rectangle x1, y1, x2, y2 ; otherwise, the bitmap is clipped as nessisary.
    //==========================================================================
    // Create images in RAM
    // This places image files into RAM for faster access rather than loading
    // the BMP file in every cycle of the loop with readimagefile(). Reading the
    // file from a drive into memory is extremely slow and puts excess load on
    // the HDD/SSD.

    //==========================================================================
    // Load hero ship graphic.
    readimagefile(f_playerShip[0], 0, 0, getmaxx(), getmaxy());// x, y, W, H
    ImgSize = imagesize(0, 0, getmaxx(), getmaxy());
    Hero = malloc( ImgSize);
    getimage (0, 0, getmaxx(), getmaxy(), Hero);

    //==========================================================================
    // Load enemy ships graphic.
    // explode images x 10
    for(i = 0; i < 10; i++)
        {
        readimagefile(f_enemyShip[i], 0, 0, getmaxx(), getmaxy());// x, y, W, H
        ImgSize = imagesize(0, 0, getmaxx(), getmaxy());
        Enemy_n[i] = malloc( ImgSize);
        getimage (0, 0, getmaxx(), getmaxy(), Enemy_n[i]);
        }

    // Load explode graphic.
    // explode images x 10
    for(i = 0; i < 10; i++)
        {
        readimagefile(f_explode[i], 0, 0, getmaxx(), getmaxy());// x, y, W, H
        ImgSize = imagesize(0, 0, getmaxx(), getmaxy());
        Explode_n[i] = malloc( ImgSize);
        getimage (0, 0, getmaxx(), getmaxy(), Explode_n[i]);
        }

    // Load warp graphic.
    // warp images x 10
    for(i = 0; i < 10; i++)
        {
        readimagefile(f_warp[i], 0, 0, getmaxx(), getmaxy());// x, y, W, H
        ImgSize = imagesize(0, 0, getmaxx(), getmaxy());
        Warp_n[i] = malloc( ImgSize);
        getimage (0, 0, getmaxx(), getmaxy(), Warp_n[i]);
        }

    closewindow(Win_ID_0);  // Closes the temporary 78x58 window.


    // (Re)Set the SDL windows options.
    // This is our main game window options.
    setwinoptions("Load image to RAM 800x600", // char *title
                  SDL_WINDOWPOS_CENTERED, // int x
                  SDL_WINDOWPOS_CENTERED, // int y
                  SDL_WINDOW_HIDDEN);  // - 1 | SDL WINDOW HIDDEN

    // intiate the graphics driver and main game window size.
    int Win_ID_1 = initwindow(SCREEN_W, SCREEN_H);

    // Get Main screen Background graphic.
    // Background images x 10
    for(i = 0; i < 10; i++)
        {
        readimagefile(f_space[i], 0, 0, getmaxx(), getmaxy());// x, y, W, H
        ImgSize = imagesize(0, 0, getmaxx(), getmaxy());
        Back_n[i] = malloc( ImgSize);
        getimage (0, 0, getmaxx(), getmaxy(), Back_n[i]);
        }

    // NOTE! These window IDs remain active in SDL under the BGI library.
    // A max of ~14 window IDs are allowed in any SDL-BGI session.
    closewindow(Win_ID_1);  // Closes the temporary 800x600 window.

    // END Copy Images to RAM
    //==========================================================================

    // Begin main game window
    // Sets the window title, the initial position to (x, y ),
    // and SDL2 flags ORed together.
    // x and y can be set to SDL WINDOWPOS CENTERED or SDL WINDOWPOS UNDEFINED.
    // If title is an empty string, the window title is set to the default
    // value SDL bgi.
    // If either x or y is -1, the position parameters are ignored.
    // If flags is -1, the parameter is ignored; otherwise, only the values
    // SDL_WINDOW_FULL-SCREEN, SDL_WINDOW_FULLSCREEN_DESKTOP, SDL_WINDOW_SHOWN,
    // SDL_WINDOW_HIDDEN, SDL_WINDOW-BORDERLESS, and SDL_WINDOW_MINIMIZED
    // can be applied
    setwinoptions("Battlestar Gatica SDL-BGI (800x600)", // char *title
                  SDL_WINDOWPOS_CENTERED, // int x
                  SDL_WINDOWPOS_CENTERED, // int y
                  -1);

    int Win_ID_2 = initwindow(SCREEN_W, SCREEN_H);

    // --> Do Intro screen background and sound.
    readimagefile("./assets/Space/Intro.bmp", 0, 0, getmaxx(), getmaxy());
    SDL_CreateThread(Intro_Sound, NULL, NULL);

    // Do intro screen text
    settextstyle (DEFAULT_FONT, HORIZ_DIR, 6 );
    setcolor (BROWN);  // RED
    outtextxy (50, 100, "BATTLESTAR GATICA" );
    setcolor (LIGHTRED);  //LIGHTGREEN
    settextstyle (DEFAULT_FONT, HORIZ_DIR, 2 );
    outtextxy (50, 200, "You are a Viper pilot for the colony fleet," );
    outtextxy (50, 250, "Protect the GATICA from the Cylox Raiders." );
    //settextstyle (DEFAULT_FONT, HORIZ_DIR, 1 );
    setcolor (LIGHTBLUE);
    outtextxy (180, 350, "Use <-- Left and Right -->" );
    outtextxy (185, 400, "To control the Viper ship" );
    outtextxy (230, 450, "Space bar to shoot" );
    outtextxy (270, 500, "'Q' to quit" );

    //swapbuffers();
    refresh();  // refresh the screen to display the background and text.

    // Using SDL_Delay() here will crash SDL_BGI/SDL2
    //SDL_Delay(27000);  // Pause to allow time to read.
    // edelay() is a bussy wait and apropriate for SDL-BGI use.
    // I have used SDL_Delay() in the main loop to reduce CPU usage, but can
    // interfere with SDL_BGI interaction with SDL2.
    edelay(27000);

    cleardevice();
    readimagefile("./assets/Space/Intro.bmp", 0, 0, getmaxx(), getmaxy());
    //swapbuffers();
    refresh();
    SDL_CreateThread(Drum_Sound, NULL, NULL);
    //SDL_Delay(3000);
    edelay(3000);


    // Set to fast = manual refresh.
    // It defaults to fast, so I don't think this is needed.
    sdlbgifast();  // sdlbgiauto(void)

    // --> Start game variables. <--
    int Back_count = 0;  // select game backgrounds.
    //putimage (0, 0, Back_n[Back_count], COPY_PUT);  // First backgound image
    //==========================================================================

    // All Graphics.h, SDL functions must come after the initwindow() or initgraph()
    // getmaxx/y() uses the array dimensions , not the screen width, aka screen -1.
    int maxx = getmaxx();
    int maxy = getmaxy();
    // Get mid position in x and y -axis
    int midx = maxx / 2;
    //int midy = maxy / 2;  // not currently used.

    // Variables for main()
    stop = 1;  // Global FLAG required to end BeepWave thread. (1 = TRUE)

    // ##### FPS counters #####
    // 60 FPS is recomented.
    int SET_FPS = 60;  // Set wanted FPS 10, 20, 30, 40, 50, 60
    if (SET_FPS > 60)
        {
        SET_FPS = 60;
        }
    else if (SET_FPS < 10)
        {
        SET_FPS = 10;
        }
    else
        {
        ; // dummy
        }

    int var_fps = (int)(1000 / SET_FPS);  // convert FPS to wait (pause) time.
    int ani_sec = 2;  // 2 seconds (Only whole seconds at the moment)
    int animation_period = SET_FPS * ani_sec; // FPS * ani_sec seconds.

    // Change x.y movement step to compensate for low frame rate.
    // FPS of 10 gives a step of 6 px, FPS of 60 gives a step of 1
    int move_step = (int)((70 - SET_FPS) / 10);  // gives a step between 1 and 6

    // Get approximate milisecons for animation frame rate. This is used to
    // calculate when to display each of the 10 frames over the seconds period.
    int ani_frame_delay = (int)(1000 / SET_FPS);
    // The above will calculate the image number out of 10 so that all
    // 10 images are displayed durring the number of frames at the current frame
    // rate.
    // There should be a error test to chech that no more than the 10 images
    // are enumerated to avoid buffer overflow read errors.
    // This is currently done in the start and end of the explosion fame count
    // but may not be suficient in all cases.
    // aka if ( array_counter !> 8) // if ( array_counter < 9) // [0] to [9]
    // old example based upon 60 ticks per second.
    //array_counter = (int)(((hexplode_frame_cnt * 1.666667) / ani_sec) / 10)

    // Used for debug FPS display.
    int fps = var_fps;  // FPS DEBUG Display
    char char_fps[32] = {'\0'};  // FPS DEBUG Display

    // Initiate out time_t variable. time_t holds the current time as unigned int
    time_t t;
    // Intializes random number generator (Movement), and tick (FPS) count.
    srand((unsigned) time(&t));  // Seed for random

    // For tick count, FPS checks.
    struct tm *tdata;
    struct timeval tv;

    // Time how long each main loop takes.
    int sec = 0;
    int last_sec = 0;
    //int msec = 0;

    // The FPS count is accurate. the following just adjusts the wait period
    // to keep the loop cycle close to the required FPS. It may vary up  or down
    // a little from FPS but averages to FPS at the end of each second.
    int start_msec = 0;  // Test for duration of main loop.
    int end_msec = 0;  // Test for duration of main loop.
    int diff_msec = 0;  // differnce to test against FPS count.
    // use int wait

    int tick_cnt = 0;  // Used to get total FPS each second.

    // Set the main loop refresh rate to hold at FPS. 1 is just an abitrary
    // number and will self adjust.
    // SDL_Sleep() is used to reduce CPU usage as well as control the frame rate.
    // edelay() is the more appropriate use for SDL_BGI, but is a "Bussy Wait".
    int wait = 1;  // Uses an SDL_Sleep() function to lower the CPU usage.

    int Key_Event = 0;  // Retreve key "Events".
    int ev;  // eventtype();  // return (bgi_last_event);
    //int mc;  // mouseclick();  // Used for some mouse operations.

    //Set the initial start possition for hero ship at the center of the screen.
    // Top Left corner of image.
    Coord_xy hero_xy = { .x = midx - 39, .y = maxy - 59 };
    //int hero_xy.x = midx - 39;  // half the width/height of the sprite.
    //int hero_xy.y = maxy - 59;

    // Variables for hero fire.
    //int Hero_shoot_flag = 0;  // Not yet implimented!
    Coord_xy Hero_shoot = { .x = 0, .y = 0 }; // Variable for Hero fire.


    // Game display stats at the top of the screen.
    int Hero_Points = 0;  // Score
    char Buf_Hero_Points[32] = {'\0'};
    int Hero_lives = 5;  // Herolife
    char Buf_Hero_Lives[32] = {'\0'};
    int level = 1; // current level (stops at 10 at the moment. Only 10 backgrounds.)
    char Buf_Level[32] = {'\0'};

    //==========================================================================

    // Variables for Enemy, Enemy fire.
    // Random x, for Enemy sprite. 78x58px 78+6+6 = 90
    // This needs to have the movement move_step included.
    int e_random_y = 6 + (rand() % (maxx - 90));  // I am using a 6 pixel padding
    // Start enemy at a random x location
    // Leaving a 40 px y padding for game info display (Text).
    Coord_xy enemy_xy = { .x = e_random_y, .y = 40 };  // Set first enemy start position.

    Coord_xy Enemy_shoot = { .x = 0, .y = 0, }; // Variable for Enemy fire.

    //printf("%ld\n", sizeof(void*));  // DUBUG INT_MAX (Get compile bit width in bytes)
    //printf("%ld\n", RAND_MAX);  // DUBUG 32767 (16-bit) due to C99 MSVCRT.DLL.
    // Set random shoot variables. Random shoot frequency increases with each level.
    // The following is preset to 100. aproximatel 1.5 seconds after first enemy
    // exist at game start. After that it is a rnd value.
    int Enemy_shoot_rnd = 100;  // Will obtain a random number 0 to 9999.
    int E_Shoot_Speed = 50;  // Starting shoot speed.(Increases tick counts)
    int E_Shoot_tick_count = 0; // Count ticks before next shoot.
    int E_Nine_Fast = 0;  // Increase UFO shoot speed. (+/-50)
    int Enemy_9_shoot = 5;  // Increase UFO Shoot steps.
    int Enemy_shoot_flag = 0;  // FLAG set to 1 when enemy is shooting.

    // Level up tests
    int Enemy_counter = 0;  // Counts total enemy ships. used for level up intervals.
    int Level_Step = 20;  // Increases in greater level steps each level.
    int Level_step_mutilpy = 10;  // Makes levelup harder each level.

    // Set bonus life intervals after n enemy kills.
    // 1000, (=1300) 2300, (+1600) 3900, (+1900) 5800, ...
    // Testing. adjust values as required.
    int Bonus_Life_counter = 1000;  // Counter for life bonus trigger (increments).
    int bonus_step = 1000;  // Next life step incremented by 300 each time.
    int bonus_step_mutilpy = 300;  // multiplyer (bonus_step increases by 300 each level)

    // We have 10 enemy ships to randomly select from.
    int Enemy_n_sel = 0;  // rnd ememy ship select from Enemy_n[Enemy_n_sel]

    int e_LR_direction = 0;  // Random choose x left or right direction.
    // Randomly sets how far the enemy will move in either direction using e_LR_direction.
    // The random number is biased to keep it moving left or right for a longer
    // number of loop cycles. This stops the sprite from jittering side to side
    // in a narraw space, but still keeps some randomness as to how far it will
    // move left or right.
    int e_LR_dir_delay;  // This is obtained from a random generated number.
    // Uses random between 0 and 99 from e_LR_dir_delay.
    // If e_LR_dir_delay greater than threshold, rand change direction (e_LR_direction).
    // A higher value threshhold the wider the LR movevement before change direction.
    int e_LR_dir_change_threshhold = 98;  // recomend 98

    // The following control the y speed of the enemy ship.
    // Misses 0-6 CPU cycles befor incrementing enemy_xy.y + step_y.
    int e_downdelay = 3;  // Faster 0 to 6 Slower (reduces by 1 each level)
    int e_downdelay_count = 0;  // Counter for e_downdelay triger.

    // Set the number of x,y pixels to move each CPU cycle.
    // This is required to keep a uniform play speed for low FPS.
    int step_x = move_step;  // Set px movement steps in px( Higher value for more speed)
    int step_y = move_step;  // Set px movement steps in px( Higher value for more speed)

    // Set our bullet length and y step each loop cycle.
    // This is for both hero and enemy.
    int b_size = 10;  // length of bullet (make sure it doesn't go off screen!)
    int fire_step = 4 + (move_step * 2);  // Fire steps in px High = faster
    //printf("%d\n", fire_step);  // DEBUG

    // ENEMY logic flags and counters.
    // Controls enemy life + explode animations.
    int enemy_is_alive_flag = 1;  // Flag enemy is alive at start of game.
    int enemy_explode_flag = 0;  // 1 == Enemy is exploding.
    int eexplode_count = 0;  // Counter for explode animation frames.
    int E_explode_image_cnt = 0;  // Enemy explode animate (10 images).

    // Enemy "Warp In".
    int ewarp_in_flag = 1;  // 1 == flag enemy warp in at game start.
    // The count is set to 1 to trigger the warp in audio, but could also be
    // set to 0 and the adio will triger after the first increment.
    int ewarp_in_count = 1;  // FLAG to set warp in for new enemy.
    int warpin_image_cnt = 0;  // Enemy Warp in animate (10 images)

    // Enemy "Warp Out".
    int ewarp_out_flag = 0;  // flag enemy warp out
    int ewarp_out_count = 0;  // warp out frame counter
    int warpout_image_cnt = 0;  // Enemy Warp out animate (10 images)


    // HERO logic flags and counters
    int hero_is_alive_flag = 1;  // Flag Hero is alive.
    int Hero_explode_flag = 0;  // Hero explode flag. 1 == explode.
    int hexplode_frame_cnt = 0;  // Hero explasion animation count.
    int H_explode_image_cnt = 0;  // Hero Enemy explode animate (10 images)


    // Start background game music. This will loop untill game ends.
    // The Audio is played in a seperate thread (different CPU core on multicore
    // CPUS) so as not to interupt the main game loop ( Non Blocking wait).
    SDL_CreateThread(Background_Sound, NULL, NULL);

    //==========================================================================
    // main loop
    // NOTE! SDL_BGI.h #define kbhit k_bhit | kbhit() is in <conio.h> or
    // windows.h as _kbhit()
    // kbhit() is for the console windows, and xkbhit() is for the graphic window.
    // But can occassional produce undefined behavior from both.
    while (stop)// When our QUIT key is pressed, stop flag is set to 0
        {

        // Get "start" tick count micro seconds for FPS monitoring.
        //t = time(NULL);  // Not required
        //tdata = localtime(&t);  // Not required
        gettimeofday(&tv, NULL);  //gettimeofday(&tv,&tz);
        // Convert useconds to mseconds
        start_msec = (int)(tv.tv_usec / 1000);

        // No Aplha channel available for images stored in RAM.
        // Display our image previously stored in RAM.
        // COPY_PUT, XOR_PUT, OR_PUT, AND_PUT, NOT_PUT
        // NOTE! Background image could be placed outside of the main loop,
        // but not recomended in this usage.
        putimage (0, 0, Back_n[Back_count], COPY_PUT);  // Render Background image.
        //Bitmap.bmp only. See: SDL2_Image

        //======================================================================
        // Set Game stats and display.
        //void settextstyle (int font, int direction, int charsize );
        // If Game_Over == 1, display the into background.
        if (Game_Over == 0)
            {
            settextstyle (DEFAULT_FONT, HORIZ_DIR, 3 );  // Set our text values.
            }
        else
            {
            readimagefile("./assets/Space/Intro.bmp", 0, 0, getmaxx(), getmaxy());
            }
        //Sets the text font ( bitmap font DEFAULT FONT and vector fonts TRIPLEX FONT, SMALL-
        //FONT, SANS SERIF FONT, GOTHIC FONT, SCRIPT FONT, SIMPLEX FONT, TRIPLEX SCR FONT),
        //the text direction (HORIZ DIR, VERT DIR), and the size of the characters.
        //charsize is a scaling factor for the text (max. 10). If charsize is 0, the text will either
        //use the default size, or it will be scaled by the values set with setusercharsize().
        //Experimental feature: if a CHR font is available in the same directory as the running
        //program, it will be loaded and used instead of its internal equivalent.
        //void settextjustify (int horiz, int vert );
        //void gettextsettings (struct textsettingstype *texttypeinfo );
        setcolor (DARKGRAY);
        //setbkcolor (BLACK);
        outtextxy (5, 10, "Score" );
        sprintf(Buf_Hero_Points, "[%06d]", Hero_Points);
        setcolor (BROWN);
        outtextxy (125, 10, Buf_Hero_Points );
        setcolor (DARKGRAY);
        outtextxy (340, 10, "Level" );
        sprintf(Buf_Level, "[%03d]", level);
        setcolor (BROWN);
        outtextxy (455, 10, Buf_Level );
        setcolor (DARKGRAY);
        outtextxy (605, 10, "Life" );
        sprintf(Buf_Hero_Lives, "[%02d]", Hero_lives);
        setcolor (BROWN);
        outtextxy (700, 10, Buf_Hero_Lives );
        setcolor (WHITE);

        // FPS Debug display. Normally commented out.
        setcolor (LIGHTGRAY);  // DEBUG
        outtextxy (5, 40, "FPS" );  // DEBUG
        sprintf(char_fps, "[%02d]", fps);  // DEBUG
        outtextxy (120, 40, char_fps );  // DEBUG
        setcolor (WHITE);  // DEBUG

        //======================================================================

        // Just a hack to clear the screen except for
        // background before displaying Game Over text.
        // Can be moved outside of main loop, or leave here if you want to add
        // a game score save routine (Although this can also be done outside
        // of the main loop).
        if (Game_Over == 0)
            {

            // Do Game logic.
            // This holds most of the main game logic, movement and control
            // routines. Some control logic is found in the keyboard and mouse
            // events checks.
            //
            // The logic switches have become a little complex, so may be tricky
            // to follow for a beginer. Essentialy each routine is switched
            // ON or OFF durring the game play to control what is displayed.
            // Some routines require multiple flag tests to trigger. When an
            // event/routine is finished, the flags and counters are reset.
            // I have made some changes to enhance readability but it is still
            // a little messy and in need of some tidying up.

            //==================================================================
            // Do hero ship display and explode.
            // The hero display at else{} could be seperated from the explode
            // routine. if ((Hero_explode_flag == 0) && (hero_is_alive_flag == 1))
            // Movement logic for hero is in events tests.
            // CHECK! I don't think both flags are required?
            if ((Hero_explode_flag == 1) && (hero_is_alive_flag == 0))  // Explode hexplode_frame_cnt Hero_explode_flag
                {
                //printf("HERO_EXPLODE!\n");

                if (hexplode_frame_cnt <= (int)(animation_period / 3))  //((hexplode_frame_cnt > 0) && (hexplode_frame_cnt < 50))
                    {
                    // Fade out hero ship 5 images (30% frames)
                    putimage (hero_xy.x, hero_xy.y, Hero, OR_PUT);
                    // if ( H_explode_image_cnt < 9)
                    H_explode_image_cnt = (int)(((hexplode_frame_cnt * ani_frame_delay) / ani_sec) / 100);
                    if (H_explode_image_cnt < 10)
                        {
                        putimage (hero_xy.x, hero_xy.y, Explode_n[H_explode_image_cnt], OR_PUT);
                        }
                    hexplode_frame_cnt++;
                    }
                else if ((hexplode_frame_cnt > (int)(animation_period / 3)) && (hexplode_frame_cnt < animation_period))
                    {
                    // Fade out hero ship 5 images (30% frames)
                    // if ( H_explode_image_cnt < 10)
                    H_explode_image_cnt = (int)(((hexplode_frame_cnt * ani_frame_delay) / ani_sec) / 100);
                    if (H_explode_image_cnt < 10)
                        {
                        if (H_explode_image_cnt < 10)
                            {
                            putimage (hero_xy.x, hero_xy.y, Explode_n[H_explode_image_cnt], OR_PUT);
                            }
                        hexplode_frame_cnt++;
                        }
                    }
                else
                    {
                    Hero_lives--;  // decrement hero life.
                    if (Hero_lives == 0)  // if hero life == 0. Dead hero.
                        {
                        // Do game over and clean up
                        Game_Over = 1;
                        //stop = 0;  // only if Game Over routine moved outside of main loop.
                        }
                    hexplode_frame_cnt = 0;  // reset eplode frame count.
                    Hero_explode_flag = 0;  // reset hero is exploding flag.
                    hero_is_alive_flag = 1;  // reset hero is alive movement
                    }
                }
            else  // Normal hero ship dislay. This can be seperated from explode.
                {
                // Place hero image on the screen at x, y.
                // It may be an idea to place the hero at center screen or
                // at a random start location.
                putimage (hero_xy.x, hero_xy.y, Hero, OR_PUT);  // COPY_PUT, 2"XOR_PUT", 1"OR_PUT"
                }

//==============================================================================

            // Enemy sprite x movement logic.
            // This routine can do with some more tidy.
            if ((enemy_is_alive_flag == 1) && ( eexplode_count == 0) )
                {

                // Sets x movement based upon rnd values.
                e_LR_dir_delay = rand() % 100;  // sets wider random movement.
                // If the rnd value is above the threshold, do rnd change direction.
                if (e_LR_dir_delay > e_LR_dir_change_threshhold )
                    {
                    e_LR_direction = rand() % 2;  // returns 0|1
                    }
                else  // continue moving in the previous L R direction until rnd threshold.
                    {
                    if (e_LR_direction > 0)  // 1 = Move Right
                        {
                        if (enemy_xy.x < maxx - 90 - step_x) // R Padding test
                            {
                            enemy_xy.x = enemy_xy.x + step_x;
                            }
                        }
                    else  // 01 = Move Left
                        {
                        if (enemy_xy.x > 6 + step_x) // L Padding test
                            {
                            enemy_xy.x = enemy_xy.x - step_x;
                            }
                        }
                    }

                // Enemy Downward Y movement logic.
                // Test if enemy has made it past hero.
                if (enemy_xy.y < maxy - (58 + step_y + 1))  // 78x58
                    {
                    // This skips a number of loops to slow the downward movement
                    // at easy levels. e_downdelay is reduced when a levelup is
                    // acheived to speed up the game play.
                    if (e_downdelay_count >= e_downdelay)
                        {
                        enemy_xy.y += step_y;  // Downward steps
                        e_downdelay_count = 0;
                        }
                    else
                        {
                        e_downdelay_count += 1;  // increment delay counter.
                        }

                    }
                else  // if enemy success, remove hero points. (enemy_xy.y at bottom of screen)
                    {
                    // remove score points from hero if enemy makes it past.
                    // Each enemy ship has a differnt points value.
                    if (Enemy_n_sel == 9)  // ship 9 is the UFO
                        {
                        Hero_Points -= 50;
                        }
                    else if (Enemy_n_sel == 8)
                        {
                        Hero_Points -= 20;
                        }
                    else if ( (Enemy_n_sel > 3) && (Enemy_n_sel < 8))
                        {
                        Hero_Points -= 10;
                        }
                    else  // all other ships (for now)
                        {
                        Hero_Points -= 5;
                        }

                    // Enemy has reached the bottom of the screen, aka made it
                    // past to strike Gatica and will now warped out.
                    SDL_CreateThread(Warpout_Sound, NULL, NULL);  // Do warp out sound.
                    // TODO: I need to seperate counters and flags eexplode_count.
                    enemy_is_alive_flag = 0;  // Enemy is dead
                    ewarp_out_flag = 1;  // Do "Warp Out" routine.
                    //printf("ewarp_out_flag\n");  // DEBUG
                    ewarp_out_count++;  // Warp out frame counter == 1 starts sound.

                    }
                }  // END Enemy sprite movement logic.


//==============================================================================

            // Warp in routine (after explosion or enemy success)
            if (ewarp_in_flag == 1)
                {
                //printf("WARP_IN\n");  // DEBUG

                // Only trigger the warp in sound once.
                if (ewarp_in_count == 1)
                    {
                    SDL_CreateThread(Warpin_Sound, NULL, NULL);
                    }

                if ((ewarp_in_count > 0) && (ewarp_in_count <= (int)(animation_period / 2))) // Do warp fade in first 5 images.
                    {
                    // warp and no ship.
                    //printf("WARP_IN!\n");
                    warpin_image_cnt = (int)(((ewarp_in_count * ani_frame_delay) / ani_sec) / 100);  // sets images number for loops.
                    if (warpin_image_cnt < 10)
                        {
                        putimage (enemy_xy.x, enemy_xy.y, Warp_n[warpin_image_cnt], OR_PUT);  // insert warp(ainimation) image 0 to 4.
                        }
                    ewarp_in_count++;  // incriment each main loop to get image count.
                    }
                else if ( (ewarp_in_count > (int)(animation_period / 2) && (ewarp_in_count < animation_period)))  // Do warp in last 5 images.
                    {
                    // Warp + ship
                    //printf("WARP_IN!\n");
                    putimage (enemy_xy.x, enemy_xy.y, Enemy_n[Enemy_n_sel], OR_PUT);  // In this half we also dislpay the ship.
                    warpin_image_cnt = (int)(((ewarp_in_count * ani_frame_delay) / ani_sec) / 100);  // sets images number for loops.
                    if (warpin_image_cnt < 10)
                        {
                        putimage (enemy_xy.x, enemy_xy.y, Warp_n[warpin_image_cnt], OR_PUT);  // insert warp(ainimation) image 5 to 9.
                        }
                    ewarp_in_count++;  // incriment each main loop to get image count.
                    }
                else  // Finished warp in routine.
                    {
                    // Reset flags for normal enemy ship display.
                    ewarp_in_flag = 0;
                    enemy_is_alive_flag = 1;
                    }
                }// END enemy enemy warp in

//==============================================================================

            // Enemy ship normal placement.
            if ((enemy_explode_flag == 0) && (ewarp_in_flag == 0) && (ewarp_out_flag == 0)) //(enemy_explode_flag != 1) do normal enemy ship placement.
                {
                // Ship only.
                putimage (enemy_xy.x, enemy_xy.y, Enemy_n[Enemy_n_sel], OR_PUT);
                }  // END Put enemy ship
            else
                {
                ; //
                }

//==============================================================================

            // Warp out routine
            if (ewarp_out_flag == 1)
                {
                //printf("WARP_OUT\n");  // DEBUG

                if ((ewarp_out_count > 0) && (ewarp_out_count <= (int)(animation_period / 2))) // Do warp fade in 5 images 0 to 4.
                    {
                    // warp and no ship.
                    //printf("WARP_IN!\n");
                    putimage (enemy_xy.x, enemy_xy.y, Enemy_n[Enemy_n_sel], OR_PUT);  // we display the enemy ship for the first 5 images.
                    warpout_image_cnt = (int)(((ewarp_out_count * ani_frame_delay) / ani_sec) / 100);  // sets each of 10 images.
                    //printf("%d\n", ewarp_out_count);  // DEBUG
                    if (warpout_image_cnt < 10)
                        {
                        putimage (enemy_xy.x, enemy_xy.y, Warp_n[warpout_image_cnt], OR_PUT);
                        }
                    ewarp_out_count++;
                    }
                else if ( (ewarp_out_count > (int)(ewarp_out_count / 2) && (ewarp_out_count < animation_period)))  // Do warp in 5 images 5 to 9.
                    {
                    // Warp + ship
                    //printf("WARP_IN!\n");

                    warpout_image_cnt = (int)(((ewarp_out_count * ani_frame_delay) / ani_sec) / 100);  // sets each of 10 images.
                    if (warpout_image_cnt < 10)
                        {
                        putimage (enemy_xy.x, enemy_xy.y, Warp_n[warpout_image_cnt], OR_PUT);
                        }
                    ewarp_out_count++;
                    }
                else  // reset for the start of a new enemy ship at the top of screen.
                    {
                    // Can move enemy warp in sound here, + pause + level up pause
                    // reset enemy and start new enemy. Reset logic.
                    // This needs to be corrected to reflect the additional step padding for FPS
                    e_random_y = 6 + (rand() % (maxx - 90));  // I am using a 6 pixel padding
                    // temppory x, y for Enemy sprite. (Random) 78x58
                    //int enemy_xy.x = midx - 39;
                    Enemy_n_sel = rand() % 10;

                    enemy_xy.x = e_random_y;  // start enemy at a random x location
                    enemy_xy.y = 40;  // Leaving a 40 px padding for game info (Text).
                    //enemy_is_alive_flag = 1;  // Not sure if this should be active?

                    ewarp_in_flag = 1;  // start warp in animation.
                    ewarp_in_count = 1;  // set to start warp in animation and sound.

                    ewarp_out_flag = 0;  // reset warp out none.
                    ewarp_out_count = 0;  // reset warp out none.
                    warpout_image_cnt = 0;  // Reset warp out image count to 0.

                    Enemy_shoot_flag = 0;  // silence enemy shoot untill warp in complete ?

                    }
                }  // END Warp Out

//==============================================================================

            // Enemy explode routine
            if (enemy_explode_flag == 1)
                {
                //printf("EXPLODE\n");  // DEBUG
                if (( eexplode_count > 0) && (eexplode_count < animation_period))
                    {
                    // explosion routine
                    if (eexplode_count <= (int)(animation_period / 2))
                        {
                        putimage (enemy_xy.x, enemy_xy.y, Enemy_n[Enemy_n_sel], OR_PUT);
                        }

                    E_explode_image_cnt = (int)(((eexplode_count * ani_frame_delay) / ani_sec) / 100);
                    if (E_explode_image_cnt < 10)  // safty to avoide buffer overflow.
                        {
                        putimage (enemy_xy.x, enemy_xy.y, Explode_n[E_explode_image_cnt], OR_PUT);
                        }

                    eexplode_count++;
                    //enemy_is_alive_flag = 0;  // stop enemy/hero shooting?
                    }
                else if (eexplode_count >= animation_period) // reset to start next enemy after explosion/warp out.
                    {

                    // Can move enemy warp in sound here, + pause + level up pause
                    // reset enemy and start new enemy. Reset logic.
                    // This needs to be corrected to reflect the additional step padding for FPS
                    e_random_y = 6 + (rand() % (maxx - 90));  // I am using a 6 pixel padding
                    // temppory x, y for Enemy sprite. (Random) 78x58
                    //int enemy_xy.x = midx - 39;
                    Enemy_n_sel = rand() % 10;

                    enemy_xy.x = e_random_y;  // start enemy at a random x location
                    enemy_xy.y = 40;  // Leaving a 40 px padding for game info (Text).

                    enemy_is_alive_flag = 1;
                    enemy_explode_flag = 0;
                    eexplode_count = 0;
                    E_explode_image_cnt = 0;  // Reset explosion image count to 0

                    ewarp_in_flag = 1;  // Start the new enemy warp in animation.
                    ewarp_in_count = 1;  // set to start warp in animation and sound.

                    // I think the following 2 flags are redundent here.
                    ewarp_out_count = 0;
                    warpout_image_cnt = 0;  // Reset warp out image count to 0

                    // #### Counter to track the number of enemy ships for level up.
                    // This should be enempy ships killed only ?
                    Enemy_counter++;
                    Enemy_shoot_flag = 0;  // dont shoot untill after warp in.



                    // Check enemy ship count for level up.
                    if (Enemy_counter > level * Level_Step)  // Level up.
                        {
                        // Increases the number of enemy kills to get to next level.
                        Level_Step += Level_step_mutilpy;
                        level++;  // set level up.
                        //SDL_CreateThread(Level_Up_Sound, NULL, NULL);  // Do levelup sound.
                        SDL_CreateThread(Drum_Sound, NULL, NULL);
                        // Set new background for level up.
                        if (Back_count < 9)  // Avoide buffer overflow (Only 1o images).
                            {
                            Back_count++;
                            }

                        // Increase difficulty on level up.
                        // The dificulty increases in a non linear way.
                        if (e_downdelay > 0)  // test for loop cycles max.
                            {
                            e_downdelay--;  // increase steps per clock (speed).
                            E_Shoot_Speed += 10;  // increase shoot frequency cnt
                            }
                        else if ((e_downdelay < 1) && (step_y < 5))  // if loop cycle max, increase steps px.
                            {
                            //step_x++;  // Increase step size (speed).
                            step_y++;  // speed up downward steps in px
                            E_Shoot_Speed += 20;  // increase shoot frequency cnt
                            }
                        else  // When all other increases have occured.
                            {
                            E_Shoot_Speed += 30;  // increase shoot frequency cnt
                            }  // END Increase dificulty on level up.
                        }  // END Level up checks.

                    }
                }  // END enemy explode


//##############################################################################
            // Hero and Enemy shoot.

            // Hero shoot logic.
            // The projectile moves a "step" each loop cycle.
            // Hero_shoot_flag ??
            if ( Hero_shoot.y !=0 )
                {
                setlinestyle(SOLID_LINE, 1, 3);  // Can move this outside of the loop.
                line(Hero_shoot.x, Hero_shoot.y, Hero_shoot.x, Hero_shoot.y - b_size);  // set b_size line length in px
                if (Hero_shoot.y >= (45 + fire_step + b_size))  // Keep the bullet on the screen.
                    {
                    Hero_shoot.y = Hero_shoot.y - fire_step;  // Fire step in px
                    }
                else
                    {
                    Hero_shoot.y = 0;  // End hero shoot at top of screen
                    }
                }  // END Hero shoot logic

            //==================================================================
            //Set enemy shoot random.
            // This still needs some tweaks and work.
            if (Enemy_shoot_flag == 0)
                {

                if (E_Shoot_tick_count > Enemy_shoot_rnd) // 1000 == minimum tick period
                    {
                    Enemy_shoot_rnd = rand() % 10000;
                    E_Shoot_tick_count = 0;  // reset tick count

                    // set enemy shoot flag (dont use x, y) // int Shoot_freq = 997
                    //if ((Enemy_shoot_rnd > Shoot_freq) && (Enemy_shoot_flag == 0) && (Hero_explode_flag == 0) && (enemy_explode_flag == 0))  //&& (enemy_is_alive_flag == 0)
                    if ((enemy_is_alive_flag == 1) && (Hero_explode_flag == 0) && (enemy_explode_flag == 0))  //&& (enemy_is_alive_flag == 0)
                        {
                        if (Enemy_n_sel == 9)  // Dirrent fire sound for UFO
                            {
                            SDL_CreateThread(UFOfire_Sound, NULL, NULL);  // UFO Fire sound
                            }
                        else
                            {
                            SDL_CreateThread(Efire_Sound, NULL, NULL); // This is laggy
                            }
                        Enemy_shoot_flag = 1;  // set the fire (bullet) in motion.
                        // Calculate current position of enemy.
                        Enemy_shoot.x = enemy_xy.x + SPRITE_W / 2;
                        Enemy_shoot.y = enemy_xy.y + SPRITE_H + 1;
                        }
                    }
                else
                    {
                    // Adjust E_Shoot_Speed steps to slower as enemy gets closer
                    // to the bottom of the screen. This doesn't alter E_Shoot_Speed
                    // wich increases each level.
                    if (Enemy_n_sel == 9)
                        {
                        E_Nine_Fast = 50;
                        }
                    else
                        {
                        E_Nine_Fast = 0;
                        }
                    // this will max at 60 (600px / 6). It may be better to half?
                    int es_temp = (int)((enemy_xy.y / 6) / 2);  // / 2 just trying to het the balance correct?
                    if (E_Shoot_Speed + 1 > es_temp)
                        {
                        // slow down the tick count as enemy gets closer to the
                        // bottom to keep a more consistant time interval.
                        E_Shoot_tick_count += E_Shoot_Speed - es_temp + E_Nine_Fast;
                        }
                    else
                        {
                        // If already at slowest don't change.
                        E_Shoot_tick_count += 1 + E_Nine_Fast;  //E_Shoot_Speed;
                        }
                    // Count ticks between shoot random in E_Shoot_Speed steps.
                    //E_Shoot_tick_count += E_Shoot_Speed;  // adjust this for faster shoot intervals
                    }
                }
            else if ( Enemy_shoot_flag != 0 )  //Enemy_shoot. Don't shoot if exploding? this is done above.
                {
                if (Enemy_n_sel == 9)
                    {
                    setlinestyle(SOLID_LINE, 1, 3);
                    setcolor (CYAN);
                    line(Enemy_shoot.x, Enemy_shoot.y, Enemy_shoot.x, Enemy_shoot.y + b_size);  // set b_size line length in px
                    setcolor (WHITE);  // reset colour back to default.
                    }
                else
                    {
                    setlinestyle(SOLID_LINE, 1, 3);
                    setcolor (LIGHTRED);
                    line(Enemy_shoot.x, Enemy_shoot.y, Enemy_shoot.x, Enemy_shoot.y + b_size);  // set b_size line length in px
                    setcolor (WHITE);  // reset colour back to default.
                    }

                if (Enemy_shoot.y <= (maxy - (fire_step + b_size)))  // Test if too close to screen bottom.
                    {
                    if (Enemy_n_sel == 9)
                        {
                        Enemy_shoot.y = Enemy_shoot.y + fire_step + Enemy_9_shoot;
                        }
                    else
                        {
                        Enemy_shoot.y = Enemy_shoot.y + fire_step;  // Fire step in px
                        }
                    }
                else  // End enemy shoot
                    {
                    // Restet enemy shoot flags.
                    Enemy_shoot_flag = 0;
                    Enemy_shoot.y = 0;
                    // DEBUG. Looking for stray bullet. [Monitoring]
                    }
                }  // END Enemy shoot logic

//##############################################################################

            //==================================================================
            // Collision detection (Hero bullet at enemy ship).
            // Test the fire x.y against enemy sprite box x,y,W,H
            if ((Hero_shoot.y > 40 + SPRITE_H) &&(Hero_shoot.x > enemy_xy.x) && (Hero_shoot.x < enemy_xy.x + SPRITE_W)
                    && (Hero_shoot.y > enemy_xy.y) && (Hero_shoot.y < enemy_xy.y + SPRITE_H))
                {
                SDL_CreateThread(Explode_Sound, NULL, NULL); // Do Explode sound.
                //if ( enemy_is_alive_flag == 1)
                if (eexplode_count == 0)
                    {
                    if (Enemy_n_sel == 9)  // ship 9 is the UFO
                        {
                        Hero_Points += 50;
                        }
                    else if (Enemy_n_sel == 8)
                        {
                        Hero_Points += 20;
                        }
                    else if ( (Enemy_n_sel > 3) && (Enemy_n_sel < 8))
                        {
                        Hero_Points += 10;
                        }
                    else  // all other ships (for now)
                        {
                        Hero_Points += 5;
                        }
                    }


                // Bonus life counter. non linear. each gap increases by 300 each time.
                if (Hero_Points > Bonus_Life_counter)
                    {
                    SDL_CreateThread(Bonus_Sound, NULL, NULL);  // Do Bonus life sound
                    bonus_step = bonus_step + bonus_step_mutilpy;  // adds aan extra 300 to each bonus step required.
                    Bonus_Life_counter = Bonus_Life_counter + bonus_step;  // each bunus step is 1000 + 300 +300 each time.
                    //Bonus_Life_counter++;  // Set a limit on this !
                    Hero_lives += 1;  // Bonus life
                    }
                eexplode_count++;
                enemy_is_alive_flag = 0;
                enemy_explode_flag = 1;
                Hero_shoot.y = 0;  // stop shoot and reset.
                }
            //==================================================================

            // Collision detection (Enemy bullet at hero ship).
            // Test the fire x.y against hero sprite box x,y,W,H
            if ((Enemy_shoot.y < maxy - 10) && (Enemy_shoot.x > hero_xy.x) && (Enemy_shoot.x < hero_xy.x + SPRITE_W)
                    && (Enemy_shoot.y > hero_xy.y) && (Enemy_shoot.y < hero_xy.y + SPRITE_H))
                {
                SDL_CreateThread(Explode_Sound, NULL, NULL); // Do explode sound.

                // set hero life end and explode flags.
                hero_is_alive_flag = 0;
                Hero_explode_flag = 1;

                // stop enemy shoot and reset.
                Enemy_shoot_flag = 0;
                Enemy_shoot.y = 0;
                Enemy_shoot.x = 0;
                }

            //==================================================================
            // We can use double buffering wich is the standard method to create
            // smooth flowing animations without flicker.
            // Use void sdlbgifast (void); Mode + refresh()
            //swapbuffers(); //swapbuffers is the same as the 4 lines below.
            // Use swpapbuffers or getvisualpage() etc
            int olda = getactivepage();
            int oldv = getvisualpage();
            setvisualpage(olda);
            setactivepage(oldv);
            // refresh(), event(), x|kbhit()
            refresh();  // Note kbhit also preforms a refresh!
            //==========================================================================
            }  // Game Over hack
        else  // Game over routine (Game_Over == 1)
            {

#ifdef _WIN32
            // Windows
            system("Taskkill /IM WavBeep.exe  /F"); // kind of ugly hack here :(
            // Using TaskKill in this instance is safe as I know the child process
            // WavBeep.exe and the wave file will be cleared from the Windows memory.
            // Otherwise using this could leave data fragments orphened in the system
            // memory heap.
#elif __linux__
            // Ubuntu
            // killall -9 aplay
            system("pkill -9 aplay"); // kind of ugly hack here :( -9 task kill, -15 sigterm
#else
#endif

            // Do "Game Over" outro screen.
            SDL_CreateThread(Outro_sound, NULL, NULL);
            // Print Game over
            settextstyle (DEFAULT_FONT, HORIZ_DIR, 8 );
            setcolor (BROWN);
            outtextxy (80, 200, "GAME OVER!!!" );
            setcolor (WHITE);

            int olda = getactivepage();
            int oldv = getvisualpage();
            setvisualpage(olda);
            setactivepage(oldv);
            refresh();
            edelay(35000);

            // Can add credits text here.

            stop = 0;
            // end game and clean up.
            }  // End Game Over hack


        //==========================================================================
        // Main event handlers. Handle keyboard and mouse.
        // There are a number of different ways to handle events. I have done it
        // this way to deal with the keyboard repeeat delay, but can be improved.
        // BGI works differently to modern SDL2 and introduces some limitations.
        event();
        // Returns 1 if one of the following events has occurred: SDL_KEYDOWN,
        // SDL_MOUSEBUTTONDOWN, SDL_MOUSEWHEEL, or SDL_QUIT; 0 otherwise.

        ev = eventtype();  // return (bgi_last_event);
        //Returns the type of the last event. Reported events are SDL_KEYDOWN,
        // SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP,
        // SDL_MOUSEWHEEL, and SDL QUIT.

        // ## !! This seams to be compile time random issue?? !! ###
        xkbhit();  // DEBUG

        // Check for mouse events.
        if (ev == SDL_MOUSEBUTTONDOWN)  // Not used in this game.
            {
            // Alternative to using ismouseclick().
            //mc = mouseclick();
            //printf("SDL_MOUSEBUTTONDOWN\n");
            //mclick = mouseclick ();
            //switch (mclick)
            //    {
            //    case WM_MOUSEMOVE:
            //        ...

            if (ismouseclick (WM_LBUTTONDOWN))  // SDL_BUTTON_LEFT
                {
                //printf("WM_LBUTTONDOWN\n");
                ;
                }
            else if (ismouseclick (WM_MBUTTONDOWN))  // SDL_BUTTON_RIGHT
                {
                //printf("WM_MBUTTONDOWN\n");
                ;
                }
            else if (ismouseclick (WM_RBUTTONDOWN))  // SDL_BUTTON_MIDDLE
                {
                //printf("WM_RBUTTONDOWN\n");
                ;
                }
            else
                {
                ;  // dummy
                }
            }
        else if (ev == WM_MOUSEMOVE)  // SDL_MOUSEMOTION
            {
            // get x,y
            //printf("WM_MOUSEMOVE\n");
            ;  // dummy
            }
        else if (ev == SDL_KEYDOWN)  // Check for keyboard events
            {
            // This section was somewhat difficult to do outside of SDL2 and
            // using only the SDL-BGI library. The key repeat rate is locked so
            // I found a small work around :)
            // SDL Keydown and Key Events can't be cleared or released from
            // SDL-BGI. As a workaround I am using kbhit(). xkbhit() to clear
            // the last event to -1 by reading the keyboard buffer (bgi_last_key_pressed).
            // getch(); // using getch() introduces keyboard repeat delay.
            // This is still getting traped on repeat key for some reason.
            // ## !! This seams to be compile time random issue?? !! ###
            xkbhit();  // xkbhit(); are required, but sometimes still get stray KB Events.
            //xkbhit();  // retest with xkbhit() before break; in switch/case
            //SDL_Delay(1); // Sometimes xkbhit() skips. Can slow down motion with higher value.
            //edelay(1);  // edelay() is more apropriate in SDL_BGI. Test and monitor.

            // SDL routine derived from SDL_bgi.c event() bgi_last_key_pressed = (int) event.key.keysym.sym;
            // and SDLKey -- Keysym definitions.
            // https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlkey.html
            // https://wiki.libsdl.org/SDL2/SDL_Keycode

            Key_Event = lastkey();  // get the last keyboard event.

            switch (Key_Event)
                {
                //case SDLK_UP:
                //    printf("KEY_UP\n");    // DEBUG
                //    break;
                case SDLK_LEFT:
                    //printf("KEY_LEFT\n");  // DEBUG
                    // Hero movement logic.
                    if (hero_is_alive_flag == 1)
                        {
                        if ( (hero_xy.x > 6 + step_x) && (hero_xy.x < maxx - SPRITE_W - 6 - step_x))
                            {
                            hero_xy.x = hero_xy.x - 5 - step_x;
                            if ( (hero_xy.x <= 6 + step_x) || (hero_xy.x >= maxx - SPRITE_W - 6 - step_x))
                                {
                                hero_xy.x = hero_xy.x + 5 + step_x;
                                }
                            }
                        }
                    //xkbhit();  // DEBUG Test
                    break;
                case SDLK_RIGHT:
                    //("KEY_RIGHT\n");  // DEBUG
                    // Hero movement logic.
                    if (hero_is_alive_flag == 1)
                        {
                        if ( (hero_xy.x > 6 + step_x) && (hero_xy.x < maxx - SPRITE_W - 6 - step_x))
                            {
                            hero_xy.x = hero_xy.x + 5 + step_x;
                            if ( (hero_xy.x <= 6 + step_x) || (hero_xy.x >= maxx - SPRITE_W -6 - step_x))
                                {
                                hero_xy.x = hero_xy.x - 5 - step_x;
                                }
                            }
                        }
                    //xkbhit();
                    break;
                case SDLK_q: // 'q'
                    //printf("q\n");  // DEBUG
                    if ((stop == 0 ) || (Game_Over == 1))
                        {
                        stop = 0;
                        }
                    else
                        {
                        // Test if we really want to quit. Also can be used as a game pause.
                        int but_ret_ID = BgiMessageBox("Quit Game", "Do you really want to quit?");
                        if (but_ret_ID == 1)
                            {
                            stop = 0;  // Set the flag to end main loop.
                            }
                        else
                            {
                            ;  // resume game
                            }
                        }
                    //xkbhit();  // DEBUG Test
                    break;
                case SDLK_SPACE: // ' '
                    //printf("SPACE\n");  // DEBUG
                    // Set fire position and fire on flag from Hero position.
                    //if ((Hero_shoot.y == 0) && (eexplode_count > 99))  // duplicate (eexplode_count > 99)?
                    if ((Hero_shoot.y == 0) && (enemy_explode_flag == 0))
                        {
                        // Use SDL Multithreading for sound.
                        //Fire_Thread = SDL_CreateThread(Fire_Sound, NULL, NULL); //Thread name, data to send
                        SDL_CreateThread(Fire_Sound, NULL, NULL); //Thread name, data to send
                        Hero_shoot.x = hero_xy.x + SPRITE_W / 2;
                        Hero_shoot.y = hero_xy.y - 1;
                        }
                    //xkbhit();  // required to clear the keyboard buffer in SDL_BGI
                    break;
                default:  // No case match found
                    //printf("DEFAULT\n");  // DEBUG
                    //xkbhit();  // DEBUG Test
                    break;
                }
            }
        else
            {
            if (ev == QUIT)  // SDL_QUIT
                {
                //printf("QUIT\n");
                if ((stop == 0 ) || (Game_Over == 1))
                    {
                    stop = 0;
                    //printf("QUIT\n");
                    }
                else
                    {
                    // Test if we really want to quit. Also can be used as a game pause.
                    int but_ret_ID = BgiMessageBox("Quit Game", "Do you really want to quit?");
                    if (but_ret_ID == 1)
                        {
                        stop = 0;  // Set the flag to end main loop.
                        //printf("QUIT\n");
                        }
                    else
                        {
                        ;  // resume game
                        }
                    }
                //xkbhit();  // DEBUG Test
                //break;  // Dont use break here!
                }
            }

        // I have replaced other xkbhit() with this SDL2 routine. In its
        // current form this seams to clear the SDL_BGI repeat buffer, but
        // requires more testing.
        // ## !! This seams to be compile time random issue?? !! ###
        SDL_PumpEvents();
        SDL_PollEvent( &sdl_ev);
        while (sdl_ev.key.repeat != 0)
            {
            //printf("KBREPEAT:%d\n", repeat_cnt++);
            xkbhit();
            SDL_PollEvent( &sdl_ev);  // Pop the events off the buffer.
            //xkbhit();
            }

        // Legacy DEBUG tests in attempt to deal with repeat key issue.
        /*
                    while(kdelay(0))  // event() ev = eventtype(); kdelay(int msec );
                    {
                        printf("EVENT\n");
                        ev = eventtype();
                    Key_Event = lastkey();
                    xkbhit();
                    }
                    printf("NONE\n");
        */
        // DEBUG tests to cancele repeat key.
        //xkbhit();
        //ev = 0;  // May not be required, Used as DEBUG to stop key repeats.
        //Key_Event = 0;  // May not be required, Used as DEBUG to stop key repeats.


        // get our end time test to calculate wait times for FPS.
        t = time(NULL);
        tdata = localtime(&t);
        gettimeofday(&tv, NULL);  //gettimeofday(&tv,&tz);
        end_msec = (int)(tv.tv_usec / 1000);  // 0.0 to 0.9
        last_sec = sec;
        sec = tdata->tm_sec;  //

        // https://thenumb.at/cpp-course/sdl2/08/08.html

        // Calculate start-end tick count and adjust "wait' value.
        //end_msec = (int)(tv.tv_usec /16667);
        // This is a fast calculation of the FPS (in tick counts) to adjust
        // the wait time. It will vary + and - the required FPS. A slow cycle
        // will catch up, whereas a fast cycle will wait longer. In the end it
        // should hover at an average SET_FPS.
        if ( end_msec > start_msec)
            {
            diff_msec = (start_msec + var_fps) - end_msec ; // usec 0.000001 msec 0.001
            }

        if ((diff_msec > 0) && (diff_msec < 999))  // Just a quick hack to exclude the - number at clock changover.
            {
            wait = diff_msec;  //  diff_msec + 33;
            }

        // Get our FPS for display from tick count, and adjust var_fps closer
        // to actual FPS. FPS is calculated after evey 1 second. The changover
        // 59 secs to 0 secs is skipped in the count.
        if (( last_sec == 59) && (sec == 0))
            {
            tick_cnt = 0;  // Cancel our ticks for this on this cycle.
            tick_cnt++;
            }
        else if (sec > last_sec)  // If 1 sec has passed.
            {
            fps = tick_cnt;  // Shows last FPS (tick_cnt per 1 second)
            if (fps > SET_FPS)
                {
                var_fps++;// = var_fps + (int)((fps - SET_FPS) / 2);  // alt +1
                //wait = var_fps;
                }
            else if (fps < SET_FPS)
                {
                var_fps--;// = var_fps - (int)((SET_FPS - fps) / 2);  // alt -1
                //wait = var_fps;
                }
            else
                {
                ;  // dummy
                }
            tick_cnt = 0;
            tick_cnt++;

            }
        else
            {
            tick_cnt++;  // Count ticks (Frames) until 1 second has passed.
            }


        // Calculate start-end tick count and adjust "wait' value.
        // FPS is set here by the wait time in miliseconds calculated above.
        // I am using SDL_Delay() instead of edelay() to invoke the task
        // schedualer to reduce CPU usage. SDL_Delay() can introduce interference
        // between SDL_BGI and SDL2 whereas edelay() is a "bussy wait" but more
        // appropriate for use in SDL_BGI.
        SDL_Delay(wait);  // Keep our CPU use low and control FPS.

        }  // END main loop

//==========================================================================
// Clean up tasks and shut down.

// End Audio tasks.
// Wait for thread to finish. Can't use this due to while() loop:(
// even with the stop flag the music plays untill the end, so kill the app.
//SDL_WaitThread(Background_Thread, NULL);

#ifdef _WIN32
// Windows
    system("Taskkill /IM WavBeep.exe  /F"); // kind of ugly hack here :(
// Using TaskKill in this instance is safe as I know the child process
// WavBeep.exe and the wave file will be cleared from the Windows memory.
// Otherwise using this could leave data fragments orphened in the system
// memory heap.

#elif __linux__
// Ubuntu
// killall -9 aplay
    system("pkill -9 aplay"); // kind of ugly hack here :( -9 task kill, -15 sigterm
#else
#endif


// Always release dynamic memory!
    free(Hero);

    for(i = 0; i < 10; i++)
        {
        free(Enemy_n[i]);
        }

    for(i = 0; i < 10; i++)
        {
        free(Explode_n[i]);
        }

    for(i = 0; i < 10; i++)
        {
        free(Warp_n[i]);
        }

    for(i = 0; i < 10; i++)
        {
        free(Back_n[i]);
        }

// deallocate memory allocated for graphic screen
    closewindow(Win_ID_2);
    closegraph();


    return 0;
    }



//==========================================================================

// Please note that my use of "system()" here is not ideal.
// It would be far better to use SDL2s built in WAV audio functions,
// or
// the platform specific popen(), pclose() as found in stdio.h
// Not all C library headers directly support this. In Windows the C library
// uses _popen() which translates to CreateProcess(). There are many other system
// specific commands to execute another application such as ShellExecute()
// Most platforms provide multiple ways to run another executable depening
// upon your specific needs.
// I some sense below I am spawning 3 proccesses with each function. The first
// proccess with SDL_CreateThread() and the second with system() invoking the
// command line interpreter and the third the actual program lanched from the
// command line "WavBeep.exe/aplay". All 3 are a child proccess of our main()
// but system() gives us no direct control over the application called which is
// why I have to use Taskkill/pkill/killall to terminate the application.
// popen() (CreateProcess()) creat a communication pipe to the child process
// alowing us to send additional command to the child proccess while it is running.
// The system() call is also very expensive in CPU cycles in this use as we are
// opening 2 significant applications as well as reading the audio from disk
// each time it is played in my usage here.
// Ideally we would use SDL2 WAV playback functions to keep the short audio
// stream in RAM (as I have done with BMPs in this app) or a dedicated audio
// playback library such as SDL_Mixer. System is just a quick workaround to
// keep the demo as close as I can to the BGI API without introducing too many
// SDL2 or platform specific APIs. I am using SDL_CreateThread() with the
// additional functions as I intend to remove system() and replace with
// SDL2s internal WAV playback. Ideally the functions can be reduced to one or
// two functions using the *arg to send the wave file name to the function.

//#ifdef _WIN32
//TerminateThread(SDL_GetThreadID(t), 0);
//#elif __linux__
//pthread_kill(SDL_GetThreadID(t), 0);
//#else
//#endif


// https://gist.github.com/ghedo/963382/
// Linux: (Built in aplay)
// aplay [flags] [filename [filename]] ...
// aplay a.wav
// aplay /home/Axle1/Music/krank_sounds/magnet_action.wav
// see also popen()
// or see mpg123
//int Fire_Sound(void *arg)
int Fire_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Fire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/laser8.wav");
    // Linux
#elif __linux__
    system("aplay --quiet ./assets/Sound/laser8.wav");
#else
#endif
    return 0;
    }

int Efire_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("EFire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/laser4.wav");
    //Linux
#elif __linux__
    system("aplay --quiet ./assets/Sound/laser4.wav");
#else
#endif
    return 0;
    }

int UFOfire_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("EFire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/bullet-plasma.wav");
    //Linux
#elif __linux__
    system("aplay --quiet ./assets/Sound/bullet-plasma.wav");
#else
#endif
    return 0;
    }

int Explode_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Explode Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/explosionCrunch_000.wav");
#elif __linux__
    system("aplay --quiet ./assets/Sound/explosionCrunch_000.wav");
#else
#endif
    return 0;
    }

int Level_Up_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Explode Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/tone1.wav");
#elif __linux__
    system("aplay --quiet ./assets/Sound/tone1.wav");
#else
#endif
    return 0;
    }

int Bonus_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Explode Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/doorOpen_002.wav");
#elif __linux__
    system("aplay --quiet ./assets/Sound/doorOpen_002.wav");
#else
#endif
    return 0;
    }

int Warpin_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Fire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/Warpin.wav");
#elif __linux__
    system("aplay --quiet ./assets/Sound/Warpin.wav");
#else
#endif
    return 0;
    }

int Warpout_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Fire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Sound/Warpout.wav");
#elif __linux__
    system("aplay --quiet ./assets/Sound/Warpout.wav");
#else
#endif
    return 0;
    }

int Intro_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Fire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Intro/BSG_Main_theme_1b.wav");
#elif __linux__
    system("aplay --quiet ./assets/Intro/BSG_Main_theme_1b.wav");
#else
#endif
    return 0;
    }

int Outro_sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Fire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Intro/Watchtower_1b.wav");
#elif __linux__
    system("aplay --quiet ./assets/Intro/Watchtower_1b.wav");
#else
#endif
    return 0;
    }

int Drum_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("Fire Audio Thread\n");  // DEBUG
    // Windows
#ifdef _WIN32
    system("WavBeep.exe ./assets/Intro/BSG_War_Drums.wav");
#elif __linux__
    system("aplay --quiet ./assets/Intro/BSG_War_Drums.wav");
#else
#endif
    return 0;
    }

int Background_Sound(void *arg)
    {
    unused(arg);  // Turns off the compiler warning for unused argc, argv
    //printf("background Audio Thread\n");  // DEBUG

    // Ring_Leader_(loop).wav, Taiko1_Gayatri-b.wav, Taiko2_Gayatri_small-b.wav
    // Unfortunately this plays untill the song finishes at close of program.
    // Use Taskkill, pkill, killall to end the CLI proccess.
    while((stop == 1) && ( Game_Over == 0))
        {
        // We could introduce a few random, or in line background music tracks.
#ifdef _WIN32
        system("WavBeep.exe ./assets/Sound/Taiko2_Gayatri_small-c.wav");
#elif __linux__
        system("aplay --quiet ./assets/Sound/Taiko2_Gayatri_small-c.wav");
#else
#endif
        }
    return 0;
    }

//==========================================================================

// This is an SDL2 message box function and is not part of SDL_BGI.
// BGI expects a DOS terminal where text mode and Graphic mode are a single
// screen, wheras modern multitasking system have 2 seperate windows for
// text mode (CLI) and graphics mode. I needed the basic Yes/No dialog :)
int BgiMessageBox(char * title, char *msgstr)  // Can add more for button data etc
    {
    // Build the data structures for the message box function.
    const SDL_MessageBoxButtonData buttons[] =
        {
        /* .flags, .buttonid, .text */
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Ok" },
        };

    // This can be omitited and replaced with NULL to use the system
    // defualt coulour scheme.
/*
    const SDL_MessageBoxColorScheme colorScheme =
        {
            { // .colors (.r, .g, .b)
            // [SDL_MESSAGEBOX_COLOR_BACKGROUND]
                { 255,   0,   0 },
            // [SDL_MESSAGEBOX_COLOR_TEXT]
                { 0, 255,   0 },
            // [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER]
                { 255, 255,   0 },
            // [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND]
                { 0,   0, 255 },
            // [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED]
                { 255,   0, 255 }
            }
        };
*/

    //string msg = "Couldn't load image %s.\nIMG_Error: " + path + IMG_GetError();

    // msg.c_str() (Used in C++ Strings) as we are using C it is not required.
    // The c_str() method converts a string to an array of characters with a
    // null character at the end. The function takes in no parameters and
    // returns a pointer to this character array (also called a c-string).

    // Relpace NULL with &colorScheme. Un-comment SDL_MessageBoxColorScheme structure.
    const SDL_MessageBoxData messageBoxData =
        {
        SDL_MESSAGEBOX_INFORMATION, /* .flags */
        NULL, /* .window */
        title, /* .title */
        msgstr, /*msg.c_str(), *//* message */
        SDL_arraysize(buttons), /* .numbuttons */
        buttons, /* .buttons */
        NULL //&colorScheme /* .colorScheme *//* NULL == System defualt*/
        };

    int buttonid;
    // The actual message box function.
    SDL_ShowMessageBox(&messageBoxData, &buttonid);

    return buttonid;  // return but ID 0|1 (No|Ok)
    }



//==========================================================================
