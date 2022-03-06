﻿/*
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

#include "PCAUnit.h"


cPolygon::cPolygon()
{
};

bool cPolygon::isPointInside(PointD p)
{
    PointD p1, p2;

    bool inside = false;

    if (pts.size() < 3)
    {
        return inside;
    }

    PointD oldPoint(pts[pts.size()- 1].X + offset.X, pts[pts.size()- 1].Y + offset.Y);

    for (int i = 0; i < pts.size(); i++)
    {
        PointD newPoint(pts[i].X + offset.X, pts[i].Y + offset.Y);

        if (newPoint.X > oldPoint.X)
        {
            p1 = oldPoint;
            p2 = newPoint;
        }
        else
        {
            p1 = newPoint;
            p2 = oldPoint;
        }

        if ((newPoint.X < p.X) == (p.X <= oldPoint.X)
            && ((p.Y - p1.Y) * (p2.X - p1.X)	< (p2.Y - p1.Y) * (p.X - p1.X)))
        {
            inside = !inside;
        }

        oldPoint = newPoint;
    }

    return inside;
}

PCAUnit::PCAUnit()
{

}

void PCAUnit::setDefaultColors(uint8_t col[3], int ID)
{
    int IDmodule = (ID - 1) % 6; // ID can't be zero
    const int colors[6][3] =
    {
        {0xFF,0xFF,0x00},
        {0x00,0xFF,0x00},
        {0x00, 0xFF, 0xFF},
        {0xFF, 0x00, 0x00},
        {0x00,0x00,0xFF},
        {0xFF,0x00,0xFF}
    };
    col[0] = colors[IDmodule][0];
    col[1] = colors[IDmodule][1];
    col[2] = colors[IDmodule][2];
}


PCAUnit::PCAUnit(int ID, int localID_): UnitID(ID),localID(localID_)
{
    setDefaultColors(ColorRGB, localID);
};

PCAUnit::~PCAUnit()
{
}

PCAUnit::PCAUnit(cPolygon B, int ID, int localID_) : UnitID(ID), localID(localID_)
{
    poly = B;
}

int PCAUnit::getUnitID()
{
    return UnitID;
}
int PCAUnit::getLocalID()
{
    return localID;
}

bool PCAUnit::isPointInsidePolygon(PointD p)
{
    return poly.isPointInside(p);
}

bool PCAUnit::isWaveFormInsidePolygon(SorterSpikePtr so)
{
    return poly.isPointInside(PointD(so->pcProj[0],so->pcProj[1]));
}

void PCAUnit::resizeWaveform(int newlength)
{

}


void PCAUnit::updateWaveform(SorterSpikePtr so)
{
    WaveformStat.update(so);
}