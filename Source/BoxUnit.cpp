/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <algorithm>

#include "BoxUnit.h"

Box::Box()
{
    x = -0.2; // in ms
    y = -10; // in uV
    w = 0.5; // in ms
    h = 70; // in uV
    channel=0;
}


Box::Box(int ch)
{
    x = -0.2; // in ms
    y = -10; // in uV
    w = 0.5; // in ms
    h = 70; // in uV
    channel = ch;
}

Box::Box(float X, float Y, float W, float H, int ch)
{
    x = X;
    y = Y;
    w = W;
    h = H;
    channel = ch;
}

bool Box::LineSegmentIntersection(PointD p11, PointD p12, PointD p21, PointD p22)
{
    PointD r = (p12 - p11);
    PointD s = (p22 - p21);
    PointD q = p21;
    PointD p = p11;
    double rs = r.cross(s);
    double eps = 1e-6;
    if (fabs(rs) < eps)
        return false; // lines are parallel
    double t = (q - p).cross(s) / rs;
    double u = (q - p).cross(r) / rs;
    return (t>=0&&t<=1 &&u>0&&u<=1);
}



bool Box::isWaveFormInside(SorterSpikePtr so)
{
    PointD BoxTopLeft(x, y);
    PointD BoxBottomLeft(x, (y - h));

    PointD BoxTopRight(x + w, y);
    PointD BoxBottomRight(x + w, (y - h));

    // y and h are given in microvolts
    // x and w and given in microseconds

    // no point testing all wave form points. Just ones that are between x and x+w...
    int BinLeft = so->microSecondsToSpikeTimeBin(x);
    int BinRight = so->microSecondsToSpikeTimeBin(x+w);

    for (int pt = BinLeft; pt < BinRight; pt++)
    {
        PointD Pwave1(so->spikeTimeBinToMicrosecond(pt), so->spikeDataBinToMicrovolts(pt, channel));
        PointD Pwave2(so->spikeTimeBinToMicrosecond(pt+1), so->spikeDataBinToMicrovolts(pt+1, channel));

        bool bLeft = LineSegmentIntersection(Pwave1,Pwave2,BoxTopLeft,BoxBottomLeft) ;
        bool bRight = LineSegmentIntersection(Pwave1,Pwave2,BoxTopRight,BoxBottomRight);
        bool bTop = LineSegmentIntersection(Pwave1,Pwave2,BoxTopLeft,BoxTopRight);
        bool bBottom = LineSegmentIntersection(Pwave1, Pwave2, BoxBottomLeft, BoxBottomRight);
        if (bLeft || bRight || bTop || bBottom)
        {
            return true;
        }

    }
    return false;
}


BoxUnit::BoxUnit(Box B, int id) 
    : unitId(id), isActive(false)
{
    addBox(B);
}

BoxUnit::BoxUnit(int id) 
    : unitId(id), isActive(false)
{

    setDefaultColors(colorRGB, unitId);

    Box B(50, -20, 300, 40);
    
    addBox(B);
}


void BoxUnit::setDefaultColors(uint8_t col[3], int id)
{
    int IDmodule = (id - 1) % 8; // ID can't be zero
    
    const int colors[8][3] =
    {
        {255,224,93},
        {255,178,99},
        {255,109,161},
        {246,102,255},
        {175,98,255},
        {90,241,233},
        {109,175,136},
        {160,237,181}
    };

    col[0] = colors[IDmodule][0];
    col[1] = colors[IDmodule][1];
    col[2] = colors[IDmodule][2];
}

void BoxUnit::updateColor()
{
    setDefaultColors(colorRGB, unitId);
}


bool BoxUnit::isWaveFormInsideAllBoxes(SorterSpikePtr so)
{
    
    for (int k = 0; k < lstBoxes.size(); k++)
    {
        if (!lstBoxes[k].isWaveFormInside(so))
            return false;
    }

    return lstBoxes.size() == 0 ? false : true;
}

bool BoxUnit::isActivated()
{
    return isActive;
}

void BoxUnit::activateUnit()
{
    isActive = true;
}

void BoxUnit::deactivateUnit()
{
    isActive = false;
}


void BoxUnit::toggleActive()
{
    if (isActive)
        deactivateUnit();
    else
        activateUnit();
}

void BoxUnit::addBox(Box b)
{
    lstBoxes.push_back(b);
}

void BoxUnit::addBox()
{
    Box B(50 + 350 * lstBoxes.size(), -20 - unitId * 20, 300, 40);
    lstBoxes.push_back(B);
}

int BoxUnit::getNumBoxes()
{
    return (int) lstBoxes.size();
}

void BoxUnit::modifyBox(int boxindex, Box b)
{
    lstBoxes[boxindex] = b;
}


bool BoxUnit::deleteBox(int boxindex)
{

    if (lstBoxes.size() > boxindex)
    {
        lstBoxes.erase(lstBoxes.begin()+boxindex);
        return true;
    }
    return false;
}

Box BoxUnit::getBox(int box)
{
    return lstBoxes[box];
}

void BoxUnit::setBox(int boxid, Box B)
{
    lstBoxes[boxid].x = B.x;
    lstBoxes[boxid].y = B.y;
    lstBoxes[boxid].w = B.w;
    lstBoxes[boxid].h = B.h;
}


void BoxUnit::setBoxPos(int boxid, PointD P)
{
    lstBoxes[boxid].x = P.X;
    lstBoxes[boxid].y = P.Y;
}

void BoxUnit::setBoxSize(int boxid, double W, double H)
{
    lstBoxes[boxid].w = W;
    lstBoxes[boxid].h = H;
}

void BoxUnit::moveBox(int boxid, int dx, int dy)
{
    lstBoxes[boxid].x += dx;
    lstBoxes[boxid].y += dy;
}

std::vector<Box> BoxUnit::getBoxes()
{
    return lstBoxes;
}

int BoxUnit::getUnitId()
{
    return unitId;
}

void BoxUnit::updateWaveform(SorterSpikePtr so)
{
    stats.update(so);
}
