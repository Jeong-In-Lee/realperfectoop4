#pragma once
#include <stdio.h>
#include <vector>
#include "CWall.h"
#include "CSphere.h"
#include <random>
#include <windows.h>

using namespace std;

class Board {

private:
    CSphere brd[13][8];
    float x_bdCtr, y_bdCtr, z_bdCtr;
    int rBoundary;

public:

    void create(float X, float Y, float Z, IDirect3DDevice9* pDevice, float depth);
    int destroy(int m, int n, CSphere& ball);
    void chNeighball(int m, int n, int col);
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

    int attachWall(CSphere& ball);
    int bAttach(int m, int n, CSphere& ball);
    void chEmpty(int m, int n);
    int bDetach();

    CSphere getBall(int row, int col);
    int getRow();
    int getCol();

    bool checkEndline();

};
