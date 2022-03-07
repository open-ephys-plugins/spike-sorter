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

#include "PCAUnit.h"


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

void PCAUnit::setDefaultColors(uint8_t col[3], int id)
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

void PCAUnit::updateColor()
{
    setDefaultColors(colorRGB, unitId);
}

PCAUnit::PCAUnit(int id, int localId_): unitId(id), localId(localId_)
{
    setDefaultColors(colorRGB, unitId);
};

PCAUnit::~PCAUnit()
{
}

PCAUnit::PCAUnit(cPolygon B, int id, int localId_) : unitId(id), localId(localId_)
{
    poly = B;
}

int PCAUnit::getUnitId()
{
    return unitId;
}
int PCAUnit::getLocalId()
{
    return localId;
}

bool PCAUnit::isPointInsidePolygon(PointD p)
{
    return poly.isPointInside(p);
}

bool PCAUnit::isWaveFormInsidePolygon(SorterSpikePtr so)
{
    return poly.isPointInside(PointD(so->pcProj[0],so->pcProj[1]));
}

void PCAUnit::updateWaveform(SorterSpikePtr so)
{
    stats.update(so);
}
