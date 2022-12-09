#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>  
#include <string>  
#include<string.h>
#include "CWall.h"
#include "Csphere.h"
#include "CLight.h"
#include "Board.h"
using namespace std;

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 600;
const int Height = 800;

#define BLOCKNUM 54

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define SHOOTNUM 30  // 제한 횟수


// -----------------------------------------------------------------------------
// CScreen class definition
// -----------------------------------------------------------------------------
// INTRO + PAUSE + RESULT 작업

class CScreen {
private:
    int GameMode = 0;  // 0 : Intro, 1 : Play, 2 : Pause, 3 :  RESULT
    string str = "PRESS ARROW KEYS TO MOVE";
    int x = 170;
    int y = Height / 2 - 50;
    int score = 0;
    int selectmode = 0;   // 0 : Start / Resume, 1 : Reload / Save, 2 : Exit
public:
    void SetPlay() {
        this->GameMode = 1;
        this->str = "Score : ";
        this->str += to_string(this->score);
        //this->str += "                           PRESS TAB TO PAUSE";
        this->x = 20;
        this->y = 10;
    }
    void SetPause() {
        this->GameMode = 2;
        this->str = "PRESS ARROW KEYS TO MOVE";
        this->x = 160;
        this->y = Height / 2 - 50;
    }
    void ShowResult() {
        this->GameMode = 3;
        this->str = "                    Score : ";
        this->str += to_string(this->score);
        this->str += "\n\n ENTER TO RESTART OR ESC TO EXIT";
        this->x = 140;
        this->y = Height / 2 - 80;
    }
    int Gmode() { return this->GameMode; }
    string Gstr() { return this->str; }
    int Gx() { return this->x; }
    int Gy() { return this->y; }
    int Gselectmode() { return this->selectmode; }
    void ChangeSelectmodeUp() {
        int temp = this->selectmode;
        if (temp == 0) {
            this->selectmode = 2;
        }
        else {
            this->selectmode = temp - 1;
        }
    }
    void ChangeSelectmodeDown() {
        int temp = this->selectmode;
        if (temp == 2) {
            this->selectmode = 0;
        }
        else {
            this->selectmode = temp + 1;
        }
    }
    void UpdateScore(int newscore) { this->score = this->score + newscore; }
    void SetScore() { this->score = 0; }

    void GameOver() {
        this->ShowResult();
        this->str = "                     Game Over\n ENTER TO RESTART OR ESC TO EXIT";
        this->x = 140;
        this->y = Height / 2 - 80;
    }
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall   g_legoPlane;
CWall   g_legowall[4];
CWall g_legoLine;
CLight   g_light;
CScreen g_screen;
CSphere   g_target_blueball;
CSphere g_shoot_ball[SHOOTNUM];
Board makeBoard;

int forsleep = 0;
int shootnum = SHOOTNUM;
bool spacepress = false;
float g_vx = 0;
float g_vz = 0;
double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
    int i;

    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);

    makeBoard.create(0, 0, 0, Device, 9);

    // create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 6.4, 0.03f, 9, d3d::WHITE)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

    if (false == g_legoLine.create(Device, -1, -1, 6.4, 0.01f, 0.1f, d3d::mRED)) return false;
    g_legoLine.setPosition(0.0f, 0.03012, -3.0f);

    // create walls and set the position. note that there are four walls
    if (false == g_legowall[0].create(Device, -1, -1, 6.6f, 0.3f, 0.12f, d3d::BLACK)) return false; // 현재 가로 기준 위쪽 & 순서대로 가로 높이 세로
    {
        g_legowall[0].setPosition(0.0f, 0.12f, 4.5f);
        g_legowall[0].set_wallPosition(0);
    }
    if (false == g_legowall[1].create(Device, -1, -1, 0.12f, 0.3f, 9, d3d::BLACK)) return false; // 현재 가로 기준 오른쪽
    {
        g_legowall[1].setPosition(3.24f, 0.12f, 0.0f);
        g_legowall[1].set_wallPosition(2);
    }
    if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 9, d3d::BLACK)) return false; // 현재 가로 기준 왼쪽
    {
        g_legowall[2].setPosition(-3.24f, 0.12f, 0.0f);
        g_legowall[2].set_wallPosition(3);
    }

    // create blue ball for set direction
    if (false == g_target_blueball.create(Device, d3d::WHITE)) return false;
    g_target_blueball.setCenter(0, (float)M_RADIUS, 0);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(1, 4);

    // create shoot ball
    for (i = 0; i < SHOOTNUM; i++) {
        if (false == g_shoot_ball[i].create(Device, d3d::YELLOW)) return false;
        int ran_color = dis(gen);
        g_shoot_ball[i].revColor(ran_color);
        g_shoot_ball[i].setCenter(0.0f, (float)M_RADIUS, -4.5f);
        g_shoot_ball[i].setPower(0, 0);
    }


    // light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type = D3DLIGHT_POINT;
    lit.Diffuse = d3d::WHITE;
    lit.Specular = d3d::WHITE * 0.9f;
    lit.Ambient = d3d::WHITE * 0.9f;
    lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;

    // Position and aim the camera.
    D3DXVECTOR3 pos(0.0f, 12.0f, -1.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
    D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
    Device->SetTransform(D3DTS_VIEW, &g_mView);

    // Set the projection matrix.
    D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
    Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    g_light.setLight(Device, g_mWorld);
    return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
    for (int i = 0; i < 4; i++) {
        g_legowall[i].destroy();
    }
    destroyAllLegoBlock();
    g_light.destroy();
}



// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
    int i = 0;
    int j = 0;
    int screen = 0;
    int temp_score = 0;

    if (shootnum < 1) {
        g_screen.ShowResult();
    }

    if (Device)
    {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();

        if (makeBoard.checkEndline()) {  // 게임 오버됐는지 확인하는 함수 
            g_screen.GameOver();
        }

        if (shootnum > 0) {
            g_shoot_ball[shootnum - 1].ballUpdate(timeDelta);
        }

        for (i = 1; i < 3; i++) {

            g_legowall[i].hitBy(g_shoot_ball[shootnum - 1]);
        }

        bool loopcheck = true;
        int  tColor = g_shoot_ball[shootnum - 1].getColor();
        int tx = -1;
        int ty = -1;

        if (g_legowall[0].hasIntersected(g_shoot_ball[shootnum - 1]))
        {
            int temppose = makeBoard.attachWall(g_shoot_ball[shootnum - 1]);
            g_shoot_ball[shootnum - 1].destroy();
            temp_score = makeBoard.destroy(0, temppose, g_shoot_ball[shootnum - 1]);
            if (temp_score > 0) {
                forsleep = 30;
            }
            else {
                spacepress = false;
            }

            if (shootnum > 0) {
                shootnum--;
            }
            g_screen.UpdateScore(temp_score * 100);
            g_screen.SetPlay();
        }


        for (i = 0; i < makeBoard.getRow(); i++) {
            if (!loopcheck) {
                break;
            }
            for (j = 0; j < makeBoard.getCol(); j++) {
                if (makeBoard.getBall(i, j).getExist()) {
                    if (makeBoard.getBall(i, j).hasIntersected(g_shoot_ball[shootnum - 1])) {
                        int temp_xz = makeBoard.bAttach(i, j, g_shoot_ball[shootnum - 1]);
                        int temp_x = temp_xz / 10;
                        int temp_z = temp_xz % 10;
                        temp_score = makeBoard.destroy(temp_x, temp_z, g_shoot_ball[shootnum - 1]);
                        if (temp_score > 0) {
                            forsleep = 30;
                        }
                        else {
                            spacepress = false;
                        }
                        g_screen.UpdateScore(temp_score * 100);
                        g_screen.SetPlay();
                        if (shootnum > 0) {
                            shootnum--;
                        }
                        loopcheck = false;
                        break;

                    }
                }
            }
        }


        g_light.draw(Device);

        // draw plane, walls, and spheres
        g_legoPlane.draw(Device, g_mWorld);

        for (i = 0; i < 3; i++) {
            g_legowall[i].draw(Device, g_mWorld);
        }

        if (shootnum > 0) {
            g_shoot_ball[shootnum - 1].draw(Device, g_mWorld);
        }

        makeBoard.draw(Device, g_mWorld);
        if (forsleep > 0) {
            forsleep--;
            if (forsleep == 0) {
                temp_score = makeBoard.bDetach();
                g_screen.UpdateScore(temp_score * 100);
                g_screen.SetPlay();
                spacepress = false;
            }
        }
        g_target_blueball.draw(Device, g_mWorld);
        g_legoLine.draw(Device, g_mWorld);
        g_light.draw(Device);


        // INTRO + PAUSE + RESULT 작업
        LPD3DXFONT Font;
        D3DXCreateFont(Device, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &Font);

        RECT rt;
        SetRect(&rt, g_screen.Gx(), g_screen.Gy(), 0, 0);
        char ch[100];
        strcpy(ch, g_screen.Gstr().c_str());
        Font->DrawText(NULL, ch, -1, &rt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
        Font->Release();


        if (g_screen.Gmode() == 0) {
            LPD3DXFONT Menu;
            D3DXCreateFont(Device, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &Menu);

            RECT MenuRt;
            SetRect(&MenuRt, 240, 400, 0, 0);
            char Sch[100];
            if (g_screen.Gselectmode() == 0) {
                strcpy(Sch, "> START\n   RESTART\n   EXIT");
            }
            else if (g_screen.Gselectmode() == 1) {
                strcpy(Sch, "   START\n> RESTART\n   EXIT");
            }
            else {
                strcpy(Sch, "   START\n   RESTART\n> EXIT");
            }
            Menu->DrawText(NULL, Sch, -1, &MenuRt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
            Menu->Release();
        }
        else if (g_screen.Gmode() == 2) {
            LPD3DXFONT Menu;
            D3DXCreateFont(Device, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &Menu);

            RECT MenuRt;
            SetRect(&MenuRt, 240, 400, 0, 0);
            char Sch[100];
            if (g_screen.Gselectmode() == 0) {
                strcpy(Sch, "> RESUME\n   RESTART\n   EXIT");
            }
            else if (g_screen.Gselectmode() == 1) {
                strcpy(Sch, "   RESUME\n> RESTART\n   EXIT");
            }
            else {
                strcpy(Sch, "   RESUME\n   RESTART\n> EXIT");
            }
            Menu->DrawText(NULL, Sch, -1, &MenuRt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
            Menu->Release();
        }
        else if (g_screen.Gmode() == 1) {   // 게임 중 shoot num 보이기
            LPD3DXFONT fFont;
            D3DXCreateFont(Device, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &fFont);

            RECT frt;
            SetRect(&frt, 150, g_screen.Gy(), 0, 0);
            char fch[100];
            string a = "LEFT : ";
            a += to_string(shootnum);
            if (!spacepress) {
                a += "          READY";
            }
            strcpy(fch, a.c_str());
            fFont->DrawText(NULL, fch, -1, &frt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
            fFont->Release();

            LPD3DXFONT tabf;
            D3DXCreateFont(Device, 20, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &tabf);
            char tabch[100] = "PRESS TAB TO PAUSE";
            RECT tabrt;
            SetRect(&tabrt, 400, g_screen.Gy(), 0, 0);
            tabf->DrawText(NULL, tabch, -1, &tabrt, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
            tabf->Release();
        }

        // INTRO + PAUSE + RESULT 작업 끝
        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);
    }
    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool wire = false;
    static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

    switch (msg) {
    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        break;
    }
    case WM_KEYDOWN:
    {
        // INTRO + PAUSE + RESULT 작업 

        if (g_screen.Gmode() == 0) {  // Intro
            switch (wParam) {
            case VK_ESCAPE:
                ::DestroyWindow(hwnd);
                break;
            case VK_UP:
                g_screen.ChangeSelectmodeUp();
                break;
            case VK_DOWN:
                g_screen.ChangeSelectmodeDown();
                break;
            case VK_RETURN:
                if (isReset) {
                    isReset = false;
                }
                if (g_screen.Gselectmode() == 0) {  // START
                    g_screen.SetPlay();
                    break;
                }
                else if (g_screen.Gselectmode() == 1) {  // RESTART
                    makeBoard.create(0, 0, 0, Device, 9);
                    shootnum = 30;
                    g_screen.SetScore();
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<int> dis(1, 4);
                    for (int i = 0; i < SHOOTNUM; i++) {
                        if (false == g_shoot_ball[i].create(Device, d3d::YELLOW)) return false;
                        int ran_color = dis(gen);
                        g_shoot_ball[i].revColor(ran_color);
                        g_shoot_ball[i].setCenter(0.0f, (float)M_RADIUS, -4.5f);
                        g_shoot_ball[i].setPower(0, 0);
                    }

                    break;
                }
                else {
                    ::DestroyWindow(hwnd);
                    break;
                }
                break;
            default:
                break;
            }
            break;
        }

        if (g_screen.Gmode() == 2) {  // Pause
            switch (wParam) {
            case VK_ESCAPE:
                ::DestroyWindow(hwnd);
                break;
            case VK_UP:
                g_screen.ChangeSelectmodeUp();
                break;
            case VK_DOWN:
                g_screen.ChangeSelectmodeDown();
                break;
            case VK_RETURN:
                if (g_screen.Gselectmode() == 0) {  // RESUME
                    g_shoot_ball[shootnum - 1].setPower(g_vx, g_vz);
                    g_screen.SetPlay();

                    break;
                }
                else if (g_screen.Gselectmode() == 1) {  // RESTART
                    makeBoard.create(0, 0, 0, Device, 9);
                    shootnum = 30;
                    g_screen.SetScore();
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<int> dis(1, 4);
                    for (int i = 0; i < SHOOTNUM; i++) {
                        if (false == g_shoot_ball[i].create(Device, d3d::YELLOW)) return false;
                        int ran_color = dis(gen);
                        g_shoot_ball[i].revColor(ran_color);
                        g_shoot_ball[i].setCenter(0.0f, (float)M_RADIUS, -4.5f);
                        g_shoot_ball[i].setPower(0, 0);
                    }
                    g_screen.SetPlay();
                    spacepress = false;
                    break;
                }
                else {
                    ::DestroyWindow(hwnd);
                    break;
                }
            default:
                break;
            }
            break;
        }

        if (g_screen.Gmode() == 3) {  // Result
            switch (wParam) {
            case VK_ESCAPE:
                ::DestroyWindow(hwnd);
                break;
            case VK_RETURN:
                if (isReset) {
                    isReset = false;
                }
                else {
                    makeBoard.create(0, 0, 0, Device, 9);
                    shootnum = 30;
                    g_screen.SetScore();
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<int> dis(1, 4);
                    for (int i = 0; i < SHOOTNUM; i++) {
                        if (false == g_shoot_ball[i].create(Device, d3d::YELLOW)) return false;
                        int ran_color = dis(gen);
                        g_shoot_ball[i].revColor(ran_color);
                        g_shoot_ball[i].setCenter(0.0f, (float)M_RADIUS, -4.5f);
                        g_shoot_ball[i].setPower(0, 0);
                    }
                    g_screen.SetPlay();
                    spacepress = false;
                    break;
                }

            default:
                break;
            }
            break;
        }

        if (g_screen.Gmode() == 1) {
            switch (wParam) {
            case VK_ESCAPE:
                ::DestroyWindow(hwnd);
                break;
            case VK_RETURN:
                if (isReset) {
                    isReset = false;
                }
                //g_screen.ShowResult();
                isReset = false;
                break;

            case VK_TAB:
                if (isReset) {
                    isReset = false;
                }
                g_vx = g_shoot_ball[shootnum - 1].getVelocity_X();
                g_vz = g_shoot_ball[shootnum - 1].getVelocity_Z();
                g_shoot_ball[shootnum - 1].setPower(0, 0);
                g_screen.SetPause();
                break;

                // INTRO + PAUSE + RESULT 작업 끝

            case VK_SPACE:    // space를 누르면 white_ball 위에 있던 red_ball의 m_is_used()값이 바뀌면서 위로 움직임
                if (isReset) {
                    isReset = false;
                }
                else {
                    if (!spacepress) {
                        D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
                        D3DXVECTOR3   whitepos = g_shoot_ball[shootnum - 1].getCenter();
                        double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
                            pow(targetpos.z - whitepos.z, 2)));      // 기본 1 사분면
                        if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }   //4 사분면
                        if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
                        if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0) { theta = PI + theta; } // 3 사분면
                        g_shoot_ball[shootnum - 1].setPower(5 * cos(theta), 5 * sin(theta));
                        spacepress = true;
                        isReset = false;

                        break;
                    }
                    break;
                }
                break;
            }
        }


    case WM_MOUSEMOVE:
    {
        if (g_screen.Gmode() == 1) {
            int new_x = LOWORD(lParam);
            int new_y = HIWORD(lParam);
            float dx;
            float dy;
            if (isReset) {
                isReset = false;
                break;
            }
            else {
                D3DXVECTOR3 vDist;
                D3DXVECTOR3 vTrans;
                D3DXMATRIX mTrans;
                D3DXMATRIX mX;
                D3DXMATRIX mY;
            }

            if (LOWORD(wParam) & MK_LBUTTON) {
                break;
            }
            else {
                isReset = true;

                if (LOWORD(wParam) & MK_RBUTTON) {
                    D3DXVECTOR3 coord3d = g_target_blueball.getCenter();
                    g_target_blueball.setCenter(new_x * 0.011 - 3.1, (float)M_RADIUS, -new_y * 0.011 + 4);
                }
                old_x = new_x;
                old_y = new_y;

                move = WORLD_MOVE;
                isReset = false;
                break;
            }
            break;
        }
        break;
    }
    default:
        break;
        break;
    }
    case WM_KEYUP:
        break;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE prevInstance,
    PSTR cmdLine,
    int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));

    if (!d3d::InitD3D(hinstance,
        Width, Height, true, D3DDEVTYPE_HAL, &Device))
    {
        ::MessageBox(0, "InitD3D() - FAILED", 0, 0);
        return 0;
    }

    if (!Setup())
    {
        ::MessageBox(0, "Setup() - FAILED", 0, 0);
        return 0;
    }

    d3d::EnterMsgLoop(Display);

    Cleanup();

    Device->Release();

    return 0;
}