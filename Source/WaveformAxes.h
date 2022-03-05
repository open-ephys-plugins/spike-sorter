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

#ifndef WAVEFORMAXES_H_
#define WAVEFORMAXES_H_

#include <VisualizerWindowHeaders.h>

#include "Containers.h"
#include "SpikeSorterCanvas.h"

#include <vector>

class BoxUnit;
class Electrode;

class WaveformAxes : public GenericDrawAxes
{
public:

    /** Constructor */
    WaveformAxes(Electrode*, int channelIndex);

    /** Destructor*/
    ~WaveformAxes() {}

    /** Handles an incoming spike*/
	bool updateSpikeData(SorterSpikePtr s);

    /** Renders the incoming waveforms */
    void paint(Graphics& g);

    /** Plots an individual spike*/
    void plotSpike(SorterSpikePtr s, Graphics& g);

    /** Renders the box boundaries*/
    void drawBoxes(Graphics& g);

    void isOverUnitBox(float x, float y, int& UnitID, int& BoxID, String& where) ;

    /** Clears internal spike buffer */
    void clear();

    int findUnitIndexByID(int ID);

    /** Mouse callbacks*/
    void mouseMove(const MouseEvent& event);
    void mouseExit(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);

    void setRange(float);

    float getRange()
    {
        return range;
    }

    float getDisplayThreshold();
    void setDetectorThreshold(float);

    void updateUnits(std::vector<BoxUnit> _units);

private:

    int electrodeID;
    bool editAll = false;
    bool signalFlipped = false;
    bool bDragging ;
    Colour waveColour;
    Colour thresholdColour;
    Colour gridColour;
    int channel;
    bool drawGrid;

    float displayThresholdLevel;
    float detectorThresholdLevel;

    void drawWaveformGrid(Graphics& g);

    void drawThresholdSlider(Graphics& g);

    int spikesReceivedSinceLastRedraw;

    Font font;
    float mouseDownX, mouseDownY;
    float mouseOffsetX, mouseOffsetY;

    SorterSpikeArray spikeBuffer;

    int spikeIndex;
    int bufferSize;

    float range;

    bool isOverThresholdSlider;
    bool isDraggingThresholdSlider;
    int isOverUnit,isOverBox;
    String strOverWhere;

    std::vector<BoxUnit> units;

    Electrode* electrode;

    MouseCursor::StandardCursorType cursorType;

};


#endif 
