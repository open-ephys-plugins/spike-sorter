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
    BoxUnit(int id);

    /** Constructor with a Box*/
    BoxUnit(Box B, int id);

    /** Returns true if spike waveform is inside all boxes*/
    bool isWaveFormInsideAllBoxes(SorterSpikePtr so);

    /** Returns the global ID for this unit */
    int getUnitId();

    /** Returns the local ID for this unit */
    int getLocalId();

    /** Returns true if unit is active */
    bool isActivated();

    /** Sets the unit's status to active */
    void activateUnit();

    /** Sets the unit's status to inactive */
    void deactivateUnit();
 
    /** Toggle's the unit's active state */
    void toggleActive();

    /** Adds a box to this unit */
    void addBox(Box b);

    /** Adds a box with default boundaries */
    void addBox();

    /** Returns the total number of boxes for this unit */
    int getNumBoxes();

    /** Changes the boundaries of a box at a particular index */
    void modifyBox(int boxindex, Box b);

    /** Removes a box at a particular index */
    bool deleteBox(int boxindex);

    /** Returns the box at a particular index */
    Box getBox(int box);

    /** Updates the box based on ID*/
    void setBox(int boxid, Box B);

    /** Sets the position of a box by ID*/
    void setBoxPos(int boxid, PointD P);

    /** Sets the size (width and height) of a box by ID */
    void setBoxSize(int boxid, double W, double H);

    /** Moves a box by a particular step size*/
    void moveBox(int boxid, int dx, int dy);
    
    /** Returns a vector of boxes for this unit */
    std::vector<Box> getBoxes();
    
    /** Adds a new waveform to this unit's stats counter */
	void updateWaveform(SorterSpikePtr so);

    /** Sets the color for this unit */
    static void setDefaultColors(uint8_t col[3], int ID);

    /** Changes the unit color after the ID is updated */
    void updateColor();

    /** Identifier for this unit (global across the Spike Sorter)*/
    int unitId;

    /** Vector of boxes for this unit */
    std::vector<Box> lstBoxes;

    /** RGB color for this unit */
    uint8_t colorRGB[3];
    
    /** Ongoing stats for this unit (not currently used) */
    WaveformStats stats;
    
    /** True if the unit is active */
    bool isActive;

};

#endif // __BOX_UNIT_H
