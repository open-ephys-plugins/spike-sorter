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

#ifndef __BOX_UNIT_H
#define __BOX_UNIT_H

#include <ProcessorHeaders.h>

#include "Containers.h"
#include "WaveformStats.h"

#include <algorithm>    // std::sort
#include <list>
#include <queue>
#include <atomic>

/** 
    Represents a Box in waveform amplitude space
*/
class Box
{
public:

    /** Default constructor */
    Box();

    /** Constructor for a single channel */
    Box(int channel);

    /** Constructor with dimensions */
    Box(float X, float Y, float W, float H, int ch=0);

    /** Returns true if a line segment is inside the box */
    bool LineSegmentIntersection(PointD p11, PointD p12, PointD p21, PointD p22);

    /** Returns true if a waveform is inside the box */
    bool isWaveFormInside(SorterSpikePtr so);

    /** Microseconds */
    double x, w;

    /** Microvolts */
    double y, h;
    
    /** Channel index*/
    int channel;
};


/** 

    Represents a single unit, which can be defined
    by multiple boxes across different channels

*/
class BoxUnit
{
public:

    /** Default constructor */
    BoxUnit() { }

    /** Constructor based on unit ID*/
    BoxUnit(int ID, int localID);

    /** Constructor with a Box*/
    BoxUnit(Box B, int ID, int localID);

    /** Returns true if spike waveform is inside all boxes*/
    bool isWaveFormInsideAllBoxes(SorterSpikePtr so);

    /** Returns true if unit is active */
    bool isActivated();
    void activateUnit();
    void deactivateUnit();
    double getNumSecondsActive();
    void toggleActive();
    void addBox(Box b);
    void addBox();
    int getNumBoxes();
    void modifyBox(int boxindex, Box b);
    bool deleteBox(int boxindex);
    Box getBox(int box);
    void setBox(int boxid, Box B);
    void setBoxPos(int boxid, PointD P);
    void setBoxSize(int boxid, double W, double H);
    void MoveBox(int boxid, int dx, int dy);
    std::vector<Box> getBoxes();
    int getUnitID();
    int getLocalID();
	void updateWaveform(SorterSpikePtr so);
    static void setDefaultColors(uint8_t col[3], int ID);
    void resizeWaveform(int newlength);
public:
    int UnitID;
    int localID; // used internally, for colors and position.
    std::vector<Box> lstBoxes;
    uint8_t ColorRGB[3];
    WaveformStats WaveformStat;
    bool Active;
    juce::int64 Activated_TS_S;
    Time timer;

};

#endif // __BOX_UNIT_H
