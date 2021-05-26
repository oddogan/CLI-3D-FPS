#include <ncursesw/ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <math.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f; // Angle of the player look

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.1459 / 4.0;
float fDepth = 16;

int main(int argc, char *argv[])
{
    // (void) signal(SIGINT, finish); /* arrange intterupts to terminate */

    // (void) initscr(); /* initialize the curses library */
    // keypad(stdscr, TRUE); /* enable keyboard mapping */
    // leaveok(stdscr, TRUE);
    // (void) nonl(); /* tell curses not to do NL->CR/NL on output */
    // (void) cbreak(); /* take input chars one at a time, no wait for \n */
    // (void) echo(); /* echo input - in color */

    /* initialize curses */
    setlocale(LC_ALL, "");
    initscr();
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();
    clear();

    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    
    wstring map;

    map += L"################";
    map += L"#.........#....#";
    map += L"#.........#....#";
    map += L"#.........#....#";
    map += L"#.........#....#";
    map += L"#..............#";
    map += L"########.......#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#...#..........#";
    map += L"#..............#";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while(1)
    {

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();
        float multiplier = 30*fElapsedTime;

        // Controls
        // Handle CCW Rotation
        char c;
        c = getchar();
        if(c == 'a')
            fPlayerA -= (1.5f) * multiplier;
        else if(c == 'd')
            fPlayerA += (1.5f) * multiplier;
        else if(c == 'w')
        {
            fPlayerX += sinf(fPlayerA) * 6.0f * multiplier;
            fPlayerY += cosf(fPlayerA) * 6.0f * multiplier;

            if(map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * 6.0f * multiplier;
                fPlayerY -= cosf(fPlayerA) * 6.0f * multiplier;
            }
        }
        else if(c == 's')
        {
            fPlayerX -= sinf(fPlayerA) * 6.0f * multiplier;
            fPlayerY -= cosf(fPlayerA) * 6.0f * multiplier;

            if(map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 6.0f * multiplier;
                fPlayerY += cosf(fPlayerA) * 6.0f * multiplier;
            }
        }
        tp1 = chrono::system_clock::now();

        for(int x=0; x < nScreenWidth; x++)
        {
            // For each column, calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
            
            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle); // unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);

            while(!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
            
                if(nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    if(map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true;

                        vector<pair<float, float>> p; // distance, dot

                        for(int tx = 0; tx < 2; tx++)
                        {
                            for(int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx*vx + vy*vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot)); 
                            }
                        }
                            

                            sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> & right) {return left.first < right.first;});
                    
                            float fBound = 0.0075;
                            if (acos(p.at(0).second) < fBound) bBoundary = true;
                            if (acos(p.at(1).second) < fBound) bBoundary = true;
                            // if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }

            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';

            for(int y = 0; y < nScreenHeight; y++)
            {
                if(y < nCeiling)
                    screen[y*nScreenWidth + x] = ' ';
                else if(y > nCeiling && y <= nFloor)
                {
                    if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588; // Very close to wall
                    else if (fDistanceToWall < fDepth / 3.0f)   nShade = 0x2593;
                    else if (fDistanceToWall < fDepth / 2.0f)   nShade = 0x2592;
                    else if (fDistanceToWall < fDepth)          nShade = 0x2591;
                    else                                        nShade = ' '; // Too far

                    if(bBoundary) nShade = ' ';
                    screen[y*nScreenWidth + x] = nShade;
                }                    
                else
                {
                    // Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));

                    if (b < 0.25)       nShade = '#';
                    else if (b < 0.5)   nShade = 'x';
                    else if (b < 0.75)  nShade = '.';
                    else if (b < 0.9)   nShade = '-';
                    else                nShade = ' ';
                    screen[y*nScreenWidth + x] = nShade;
                }
            }
        }

        screen[nScreenWidth * nScreenHeight -1] = '\0';
        for (int nx = 0; nx < nMapWidth; nx++)
        {
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny+1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        }
        screen[((int)fPlayerY+1) * nScreenWidth + (int)fPlayerX] = 'P';
        mvaddwstr(0, 0, screen);
        
        mvprintw(0, 0, "X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f/fElapsedTime);
        
        refresh();
    }

    getch();
    endwin();

    return 0;
}
