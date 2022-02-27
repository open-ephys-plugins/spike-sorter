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

#ifndef __PCA_UNIT_H
#define __PCA_UNIT_H

#include <ProcessorHeaders.h>

#include "Containers.h"
#include "WaveformStats.h"

#include <algorithm>    // std::sort
#include <list>
#include <queue>
#include <atomic>


class cPolygon
{
public:
    cPolygon();
    bool isPointInside(PointD p);
    std::vector<PointD> pts;
    PointD offset;
};


class PCAUnit
{
public:
    PCAUnit();
    PCAUnit(int ID, int localID);
    PCAUnit(cPolygon B, int ID, int localID_);
    ~PCAUnit();
    int getUnitID();
    int getLocalID();
	bool isWaveFormInsidePolygon(SorterSpikePtr so);
    bool isPointInsidePolygon(PointD p);
    void resizeWaveform(int newlength);
	void updateWaveform(SorterSpikePtr so);
public:
    int UnitID;
    int localID; // used internally, for colors and position.
    cPolygon poly;
    uint8_t ColorRGB[3];
    static void setDefaultColors(uint8_t col[3], int ID);
    WaveformStats WaveformStat;
    bool Active;
    juce::int64 Activated_TS_S;
    Time timer;
};


#endif // __PCA_UNIT_H
