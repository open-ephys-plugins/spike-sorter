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
class SpikePlot;

class WaveformAxes : public GenericDrawAxes
{
public:

    /** Constructor */
    WaveformAxes(SpikePlot*, Electrode*, int channelIndex);

    /** Destructor*/
    ~WaveformAxes() {}

    /** Handles an incoming spike*/
	bool updateSpikeData(SorterSpikePtr s) override;

    /** Renders the incoming waveforms */
    void paint(Graphics& g) override;
    
    /** Sets whether spikes should be redrawn*/
    void refresh();

    /** Plots an individual spike*/
    void plotSpike(SorterSpikePtr s, Graphics& g);

    /** Called when axes are resized */
    void resized() override;

    void isOverUnitBox(float x, float y, int& UnitID, int& BoxID, String& where) ;

    /** Clears internal spike buffer */
    void clear();

    int findUnitIndexById(int id);

    /** Mouse callbacks*/
    void mouseMove(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

    /** Get/set axes range */
    void setRange(float);
    float getRange() { return range; }

    /** Get/set display threshold */
    float getDisplayThreshold();
    void setDetectorThreshold(float);

    /** Updates the box units for this plot*/
    void updateUnits(std::vector<BoxUnit> units);

private:

    /**
        Class used to draw annotations, so waveforms can be
     redrawn independently
     */
    class AnnotationComponent : public Component
    {
    public:
        
        /** Constructor */
        AnnotationComponent(Electrode* electrode, std::vector<BoxUnit>* units);
        
        /** Render the component */
        void paint(Graphics& g) override;
        
        Colour thresholdColour = Colours::red;
        bool signalFlipped = false;
        bool isOverThresholdSlider = false;
        float displayThresholdLevel = 0.0f;
        float range;
        
        int isOverUnit = -1;
        int isOverBox = -1;
        
        std::vector<BoxUnit>* units;
    private:

        /** Renders the box boundaries*/
        void drawBoxes(Graphics& g);

        /** Draws threshold slider*/
        void drawThresholdSlider(Graphics& g);
        
        Electrode* electrode;
        
    };
    
    std::unique_ptr<AnnotationComponent> annotationComponent;
    
    /** Draws tick marks behind waveforms */
    void drawWaveformGrid(Graphics& g);

    bool editAll = false;
    bool signalFlipped = false;
    bool bDragging = false;
    Colour waveColour;
    Colour gridColour;
    int channel;

    float displayThresholdLevel = 0.0f;
    float detectorThresholdLevel;

    int spikesReceivedSinceLastRedraw = 0;

    float mouseDownX, mouseDownY;
    float mouseOffsetX, mouseOffsetY;

    SorterSpikeArray spikeBuffer;

    int spikeIndex = 0;
    int bufferSize = 5;

    float range = 250.0f;

    bool isOverThresholdSlider = false;
    bool isDraggingThresholdSlider = false;
    int isOverUnit = -1;
    int isOverBox = -1;
    String strOverWhere;

    std::vector<BoxUnit> units;

    Electrode* electrode;
    SpikePlot* plot;

    MouseCursor::StandardCursorType cursorType;

};


#endif 
