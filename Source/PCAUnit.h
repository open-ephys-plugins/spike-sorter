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
    PCAUnit(int id, int localId);

    /** Constructor with polygon + global and local IDs specified */
    PCAUnit(cPolygon B, int id, int localId);

    /** Destructor */
    ~PCAUnit();

    /** Returns global ID for this unit */
    int getUnitId();

    /** Returns the local ID for this unit */
    int getLocalId();

    /** Checks whether waveform is inside this unit's polygon */
	bool isWaveFormInsidePolygon(SorterSpikePtr so);

    /** Checks whether a point is inside this unit's polygone */
    bool isPointInsidePolygon(PointD p);

    /** Updates the waveform for this unit */
	void updateWaveform(SorterSpikePtr so);

public:
    
    int unitId;
    int localId; // used internally, for colors and position.

    cPolygon poly;
    uint8_t ColorRGB[3];
    static void setDefaultColors(uint8_t col[3], int ID);
    WaveformStats stats;
    bool Active;
    juce::int64 Activated_TS_S;
    Time timer;
};


#endif // __PCA_UNIT_H
