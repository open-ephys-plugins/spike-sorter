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

#ifndef PCAPROJECTIONAXES_H_
#define PCAPROJECTIONAXES_H_

#include <VisualizerWindowHeaders.h>

#include "SpikeSorterCanvas.h"

class PCAProjectionAxes : public GenericDrawAxes,  
                          public Button::Listener
{
public:

    /** Constructor */
    PCAProjectionAxes(SpikeSorterCanvas* , Electrode* );

    /** Destructor */
    ~PCAProjectionAxes() {}

    /** Sets range for PCA*/
    void setPCARange(float p1min, float p2min, float p1max, float p2max);

    /** Adds a new spike object*/
	bool updateSpikeData(SorterSpikePtr s);

    /** Renders the PCA projections */
    void paint(Graphics& g);

    /** Turns polygon drawing mode on or off*/
    void setPolygonDrawingMode(bool on);
    void clear();

    /** Mouse callbacks*/
    void mouseDown(const juce::MouseEvent& event);
    void mouseUp(const juce::MouseEvent& event);
    void mouseMove(const juce::MouseEvent& event);
    void mouseDrag(const juce::MouseEvent& event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);

    bool keyPressed(const KeyPress& key);
    
    void redraw(bool subsample);

    void updateUnits(std::vector<PCAUnit> _units);

    void buttonClicked(Button* button);

    void drawUnit(Graphics& g, PCAUnit unit);
    void rangeDown();
    void rangeUp();

    SpikeSorterCanvas* canvas;
    Electrode* electrode;

private:
    float prevx,prevy;
    bool inPolygonDrawingMode;
	void drawProjectedSpike(SorterSpikePtr s);

    bool rangeSet;
    
    void updateProjectionImage(uint16_t, uint16_t, uint16_t, const uint8_t* col);
	void updateRange(SorterSpikePtr s);
    ScopedPointer<UtilityButton> rangeDownButton, rangeUpButton;

    SorterSpikeArray spikeBuffer;
    int bufferSize;
    int spikeIndex;
    bool updateProcessor;
	void calcWaveformPeakIdx(SorterSpikePtr, int, int, int*, int*);

    Image projectionImage;

    Colour pointColour;
    Colour gridColour;

    int imageDim;

    int rangeX;
    int rangeY;

    int spikesReceivedSinceLastRedraw;

    float pcaMin[2],pcaMax[2];
    std::list<PointD> drawnPolygon;

    std::vector<PCAUnit> units;
    int isOverUnit;
    PCAUnit drawnUnit;

    bool redrawSpikes;
};

#endif  // SPIKESORTERCANVAS_H_
