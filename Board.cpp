#include "Board.h"

    void Board::create(float X, float Y, float Z, IDirect3DDevice9* pDevice, float depth) {

        this->x_bdCtr = X;
        this->y_bdCtr = Y;
        this->z_bdCtr = Z;

        rBoundary = 3;

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(1, 4);


        for (int i = 0; i < sizeof(brd) / sizeof(brd[0]); i++) //row
        {
            for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) //col
            {
                brd[i][j].create(pDevice);

                int color = dis(gen);
                brd[i][j].revColor(color);

                if (i % 2 == 0) {
                    brd[i][j].setCenter(x_bdCtr - (3.8 - j) * brd[i][j].getRadius() / 0.5, brd[0][0].getRadius(), z_bdCtr + depth / 2 - 0.5 - i * brd[i][j].getRadius() * sqrt(3));
                }
                else if (i % 2 == 1) {
                    brd[i][j].setCenter(x_bdCtr - (3.3 - j) * brd[i][j].getRadius() / 0.5, brd[0][0].getRadius(), z_bdCtr + depth / 2 - 0.5 - i * brd[i][j].getRadius() * sqrt(3));
                }
                brd[i][j].setExist(true);
                brd[i][j].setChflag(0);
                brd[i][j].setDeflag(0);

                if (i > 3)
                {
                    brd[i][j].setExist(false);
                    brd[i][j].setColor(d3d::MAGENTA);
                }
            }
        }
    }

    int Board::destroy(int m, int n, CSphere& ball) {

        int col = ball.getColor();
        this->chNeighball(m, n, col);

        int hit = 0;

        // �Ͷ߸��� �ݺ���
        for (int i = 0; i <= rBoundary; i++) {
            for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) {
                if (brd[i][j].getExist() && brd[i][j].getChflag() == 1) {
                    //�Ͷ߸��� ����
                    hit++;
                }
            }
        }
        if (hit >= 3)
        {
            for (int i = 0; i <= rBoundary; i++) {
                for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) {
                    if (brd[i][j].getExist() && brd[i][j].getChflag() == 1) {
                        //�Ͷ߸��� ����
                        brd[i][j].setExist(false);
                        brd[i][j].setColor(d3d::MAGENTA);
                    }
                }
            }

            //Sleep(500);
            //hit += bDetach();
            //hit++;

        }
        else
        {
            for (int i = 0; i <= rBoundary; i++) {
                for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) {
                    if (brd[i][j].getExist() && brd[i][j].getChflag() == 1) {
                        //�Ͷ߸��� ����
                        brd[i][j].setChflag(0);
                    }
                }
            }

            hit = 0;
        }
        for (int i = 0; i < sizeof(brd[0]) / sizeof(CSphere); i++) {
            if (brd[rBoundary][i].getExist())
                break;
            else if (i == sizeof(brd[0]) / sizeof(CSphere) - 1 && (!brd[rBoundary][i].getExist())) {
                i = -1;
                rBoundary--;

            }
        }

        return hit;
    }

    void Board::chNeighball(int m, int n, int col) {
        if (brd[m][n].getChflag() == 0) {
            if (brd[m][n].getColor() == col) {
                brd[m][n].setChflag(1);

                if (n < sizeof(brd[0]) / sizeof(CSphere) - 1) {
                    chNeighball(m, n + 1, col);
                }

                if (m % 2 == 0) {
                    if (m <= 10) { //rb                        
                        chNeighball(m + 1, n, col);
                        if (n > 0)
                            chNeighball(m + 1, n - 1, col);

                    }
                    if (n > 0) {
                        chNeighball(m, n - 1, col);
                        if (m > 0)
                            chNeighball(m - 1, n - 1, col);
                    }
                    if (m > 0)
                        chNeighball(m - 1, n, col);
                }

                else if (m % 2 == 1) {
                    if (m <= 10) { //rb
                        if (n < sizeof(brd[0]) / sizeof(CSphere) - 1)
                            chNeighball(m + 1, n + 1, col);

                        chNeighball(m + 1, n, col);
                    }
                    if (n > 0)
                        chNeighball(m, n - 1, col);

                    if (m > 0) {
                        chNeighball(m - 1, n, col);
                        if (n < sizeof(brd[0]) / sizeof(CSphere) - 1)
                            chNeighball(m - 1, n + 1, col);
                    }
                }

            }
        }
    }


    void Board::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld) {
        for (int i = 0; i < sizeof(brd) / sizeof(brd[0]); i++) //row
        {
            for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) //col
            {
                if (brd[i][j].getExist())
                    brd[i][j].draw(pDevice, mWorld);
            }
        }
    }

    int Board::attachWall(CSphere& ball)
    {
        float short_distance = 1000;
        float temp;
        int pos = 10;
        for (int i = 0; i < getCol(); i++)
        {
            if (!brd[0][i].getExist())
            {
                temp = brd[0][i].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = i;
                }
            }
        }

        brd[0][pos].setExist(true);
        brd[0][pos].revColor(ball.getColor());
        brd[0][pos].setChflag(0);
        return pos;
    }

    int Board::bAttach(int m, int n, CSphere& ball) {
        int available[6] = { 0, };
        int value;
        if (m == 0)
            available[0] = available[1] = 1;

        if (m == getRow())
            available[4] = available[5] = 1;

        if (m % 2 == 0)
        {
            float short_distance = 1000;
            float temp;
            int pos = 6; // 0 left above 1 right above 2 left 3 right 4 left under 5 right under 

            if (n == 0)
                available[0] = available[2] = available[4] = 1;

            if (n == getCol())
                available[3] = 1;

            if (available[0] == 0 && !brd[m - 1][n - 1].getExist())
            {
                temp = brd[m - 1][n - 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 0;
                }
            }

            if (available[1] == 0 && !brd[m - 1][n].getExist())
            {
                temp = brd[m - 1][n].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 1;
                }
            }

            if (available[2] == 0 && !brd[m][n - 1].getExist())
            {
                temp = brd[m][n - 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 2;
                }
            }

            if (available[3] == 0 && !brd[m][n + 1].getExist())
            {
                temp = brd[m][n + 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 3;
                }
            }

            if (available[4] == 0 && !brd[m + 1][n - 1].getExist())
            {
                temp = brd[m + 1][n - 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 4;
                }
            }

            if (available[5] == 0 && !brd[m + 1][n].getExist())
            {
                temp = brd[m + 1][n].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 5;
                }
            }

            switch (pos) {
            case 0:
                brd[m - 1][n - 1].setExist(true);
                brd[m - 1][n - 1].revColor(ball.getColor());
                brd[m - 1][n - 1].setChflag(0);
                value = (m - 1) * 10 + (n - 1);
                break;
            case 1:
                brd[m - 1][n].setExist(true);
                brd[m - 1][n].revColor(ball.getColor());
                brd[m - 1][n].setChflag(0);
                value = (m - 1) * 10 + (n);
                break;
            case 2:
                brd[m][n - 1].setExist(true);
                brd[m][n - 1].revColor(ball.getColor());
                brd[m][n - 1].setChflag(0);
                value = (m) * 10 + (n - 1);
                break;
            case 3:
                brd[m][n + 1].setExist(true);
                brd[m][n + 1].revColor(ball.getColor());
                brd[m][n + 1].setChflag(0);
                value = (m) * 10 + (n + 1);
                break;
            case 4:
                brd[m + 1][n - 1].setExist(true);
                brd[m + 1][n - 1].revColor(ball.getColor());
                brd[m + 1][n - 1].setChflag(0);
                value = (m + 1) * 10 + (n - 1);
                break;
            case 5:
                brd[m + 1][n].setExist(true);
                brd[m + 1][n].revColor(ball.getColor());
                brd[m + 1][n].setChflag(0);
                value = (m + 1) * 10 + (n);
                break;
            }

        }
        else
        {
            float short_distance = 1000;
            float temp;
            int pos = 6; // 0 left above 1 right above 2 left 3 right 4 left under 5 right under

            if (n == 0)
                available[2] = 1;

            if (n == getCol())
                available[1] = available[3] = available[5] = 1;

            if (available[0] == 0 && !brd[m - 1][n].getExist())
            {
                temp = brd[m - 1][n].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 0;
                }
            }

            if (available[1] == 0 && !brd[m - 1][n + 1].getExist())
            {
                temp = brd[m - 1][n + 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 1;
                }
            }

            if (available[2] == 0 && !brd[m][n - 1].getExist())
            {
                temp = brd[m][n - 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 2;
                }
            }

            if (available[3] == 0 && !brd[m][n + 1].getExist())
            {
                temp = brd[m][n + 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 3;
                }
            }

            if (available[4] == 0 && !brd[m + 1][n].getExist())
            {
                temp = brd[m + 1][n].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 4;
                }
            }

            if (available[5] == 0 && !brd[m + 1][n + 1].getExist())
            {
                temp = brd[m + 1][n + 1].getDistance(ball);
                if (short_distance > temp)
                {
                    short_distance = temp;
                    pos = 5;
                }
            }

            switch (pos) {
            case 0:
                brd[m - 1][n].setExist(true);
                brd[m - 1][n].revColor(ball.getColor());
                brd[m - 1][n].setChflag(0);
                value = (m - 1) * 10 + n;
                break;
            case 1:
                brd[m - 1][n + 1].setExist(true);
                brd[m - 1][n + 1].revColor(ball.getColor());
                brd[m - 1][n + 1].setChflag(0);
                value = (m - 1) * 10 + (n + 1);
                break;
            case 2:
                brd[m][n - 1].setExist(true);
                brd[m][n - 1].revColor(ball.getColor());
                brd[m][n - 1].setChflag(0);
                value = (m) * 10 + (n - 1);
                break;
            case 3:
                brd[m][n + 1].setExist(true);
                brd[m][n + 1].revColor(ball.getColor());
                brd[m][n + 1].setChflag(0);
                value = (m) * 10 + (n + 1);
                break;
            case 4:
                brd[m + 1][n].setExist(true);
                brd[m + 1][n].revColor(ball.getColor());
                brd[m + 1][n].setChflag(0);
                value = (m + 1) * 10 + (n);
                break;
            case 5:
                brd[m + 1][n + 1].setExist(true);
                brd[m + 1][n + 1].revColor(ball.getColor());
                brd[m + 1][n + 1].setChflag(0);
                value = (m + 1) * 10 + (n + 1);
                break;
            }
        }

        for (int i = sizeof(brd) / sizeof(brd[0]) - 1; i >= rBoundary; i--) {
            for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) {
                if (brd[i][j].getExist()) {
                    rBoundary = i;
                }
            }
        }

        return value;
    }

    void Board::chEmpty(int m, int n) {
        if (brd[m][n].getExist()) {
            if (!brd[m][n].getDeflag()) {
                brd[m][n].setDeflag(1);

                if (n - 1 >= 0)
                    chEmpty(m, n - 1);

                if (m % 2 == 0) {
                    if (m - 1 >= 0) {
                        if (n - 1 >= 0)
                            chEmpty(m - 1, n - 1);

                        chEmpty(m - 1, n);
                    }

                    if (n + 1 < sizeof(brd[0]) / sizeof(CSphere))
                        chEmpty(m, n + 1);

                    if (m + 1 < sizeof(brd) / sizeof(brd[0])) {
                        chEmpty(m + 1, n);

                        if (n - 1 >= 0)
                            chEmpty(m + 1, n - 1);
                    }
                }//end of even m

                else if (m % 2 == 1) {
                    if (m - 1 >= 0) {
                        chEmpty(m - 1, n);

                        if (n + 1 < sizeof(brd[0]) / sizeof(CSphere))
                            chEmpty(m - 1, n + 1);
                    }

                    if (n + 1 < sizeof(brd[0]) / sizeof(CSphere)) {
                        chEmpty(m, n + 1);

                        if (m + 1 < sizeof(brd) / sizeof(brd[0]))
                            chEmpty(m + 1, n + 1);
                    }
                    if (m + 1 < sizeof(brd) / sizeof(brd[0]))
                        chEmpty(m + 1, n);
                }//end of odd m
            }
        }
    }

    int Board::bDetach() {

        for (int i = 0; i < sizeof(brd[0]) / sizeof(CSphere); i++) {
            chEmpty(0, i);
        }

        int hit = 0;

        for (int i = 0; i < sizeof(brd) / sizeof(brd[0]); i++) {
            for (int j = 0; j < sizeof(brd[0]) / sizeof(CSphere); j++) {
                if (brd[i][j].getDeflag() != brd[i][j].getExist()) {
                    brd[i][j].setExist(false);
                    brd[i][j].setColor(d3d::MAGENTA);
                    hit++;
                }
                brd[i][j].setDeflag(0);
            }
        }
        return hit;
    }

    CSphere Board::getBall(int row, int col)
    {
        return this->brd[row][col];
    }

    int Board::getRow()
    {
        return sizeof(brd) / sizeof(brd[0]);
    }

    int Board::getCol()
    {
        return sizeof(brd[0]) / sizeof(CSphere);
    }

    bool Board::checkEndline()
    {
        for (int i = 0; i < getCol(); i++)
        {
            if (brd[getRow() - 1][i].getExist())
                return true;
        }
        return false;
    }


