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

/** 
    Represents a polygon in 2D PCA space
*/
class cPolygon
{
public:

    /** Constructor */
    cPolygon() { }

    /** Returns true if 2D point is inside polygon */
    bool isPointInside(PointD p);

    std::vector<PointD> pts;

    PointD offset;
};

/** 
    A unit defined by a polygon in principal component space
*/
class PCAUnit
{
public:

    /** Default constructor */
    PCAUnit() { }

    /** Constructor with global and local IDs specified */
    PCAUnit(int id);

    /** Constructor with polygon + global and local IDs specified */
    PCAUnit(cPolygon B, int id);

    /** Destructor */
    ~PCAUnit();

    /** Returns global ID for this unit */
    int getUnitId();

    /** Checks whether waveform is inside this unit's polygon */
	bool isWaveFormInsidePolygon(SorterSpikePtr so);

    /** Checks whether a point is inside this unit's polygone */
    bool isPointInsidePolygon(PointD p);

    /** Updates the waveform for this unit */
	void updateWaveform(SorterSpikePtr so);

    /** Sets the color for this unit */
    static void setDefaultColors(uint8_t col[3], int ID);

    /** Updates the unit's color (when a new ID is assigned) */
    void updateColor();
  
    /** Identifier for this unit (global across the Spike Sorter) */
    int unitId;

    /** Polygon that defines this unit's boundaries in PCA space*/
    cPolygon poly;

    /** RGB color for this unit */
    uint8_t colorRGB[3];

    /** Ongoing stats for this unit (not currently used) */
    WaveformStats stats;

    /** True if this unit is active */
    bool isActive;

};


#endif // __PCA_UNIT_H
