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

#ifndef SPIKESORTERCANVAS_H_
#define SPIKESORTERCANVAS_H_

#include <VisualizerWindowHeaders.h>
#include "SpikeSorter.h"

#include <vector>

#define WAVE1 0
#define WAVE2 1
#define WAVE3 2
#define WAVE4 3
#define PROJ1x2 4
#define PROJ1x3 5
#define PROJ1x4 6
#define PROJ2x3 7
#define PROJ2x4 8
#define PROJ3x4 9

#define TETRODE_PLOT 1004
#define STEREO_PLOT  1002
#define SINGLE_PLOT  1001

#define MAX_NUMBER_OF_SPIKE_SOURCES 128
#define MAX_N_CHAN 4

class SpikeHistogramPlot;
class SpikeThresholdDisplay;
class SpikeDisplayNode;
class SpikePlot;
class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;
class SpikePlot;
class RecordNode;

/**

  Displays spike waveforms and projections for Spike Sorter

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/
class SpikeSorterCanvas : public Visualizer, public Button::Listener

{
public:

    /** Constructor */
    SpikeSorterCanvas(SpikeSorter* n);

    /** Destructor */
    ~SpikeSorterCanvas() { }

    /** Fills background*/
    void paint(Graphics& g);

    /** Called instead of "repaint" to avoid redrawing underlying components.*/
    void refresh();

    /** Starts animation callbacks*/
    void beginAnimation();

    /** Stops animation callbacks*/
    void endAnimation();

    /** Called when the component's tab becomes visible again*/
    void refreshState();

    /** Creates spike displays for incoming spike channels*/
    void update();

    /** Updates size of spike display*/
    void resized();

    /** Responds to C (clear), escape, and delete*/
    bool keyPressed(const KeyPress& key);

    /** Responds to button clicks*/
    void buttonClicked(Button* button);

    /** Updates the current electrode */
    void setActiveElectrode(Electrode* electrode);

    SpikeSorter* processor;

    ScopedPointer<UtilityButton> 
        addPolygonUnitButton,
        addUnitButton,
        delUnitButton,
        addBoxButton,
        delBoxButton,
        rePCAButton,
        nextElectrode,
        prevElectrode,
        newIDbuttons,
        deleteAllUnits;

private:
    void removeUnitOrBox();

    ScopedPointer<SpikeDisplay> spikeDisplay;
    ScopedPointer<Viewport> viewport;

    bool inDrawingPolygonMode;
    bool newSpike;

    Electrode* electrode;
    int scrollBarThickness;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorterCanvas);

};

/** 
    Holds multiple SpikePlot components

*/
class SpikeDisplay : public Component
{
public:

    /** Constructor */
    SpikeDisplay(SpikeSorter*, SpikeSorterCanvas*, Viewport*);

    /** Destructor */
    ~SpikeDisplay();

    void removePlots();
    void clear();
    SpikePlot* addSpikePlot(int numChannels, int electrodeNum, String name);

    void paint(Graphics& g);

    void resized();
    void setPolygonMode(bool on);
    void mouseDown(const juce::MouseEvent& event);

    void plotSpike(SorterSpikePtr spike, int electrodeNum);

    int getTotalHeight()
    {
        return totalHeight;
    }

private:
    int numColumns;
    int totalHeight;
    
    SpikeSorter* processor;
    SpikeSorterCanvas* canvas;
    Viewport* viewport;

    OwnedArray<SpikePlot> spikePlots;


};

/** 

    Base class for WaveformAxes and PCAProjectionAxes

*/
class GenericDrawAxes : public Component
{
public:

    GenericDrawAxes(int t);

    virtual ~GenericDrawAxes();

    virtual bool updateSpikeData(SorterSpikePtr s);

    void setXLims(double xmin, double xmax);
    void getXLims(double* xmin, double* xmax);
    void setYLims(double ymin, double ymax);
    void getYLims(double* ymin, double* ymax);

    void setType(int type);
    int getType();

    virtual void paint(Graphics& g) = 0;

    int roundUp(int, int);
    void makeLabel(int val, int gain, bool convert, char* s);

protected:
    double xlims[2];
    double ylims[2];

    SorterSpikePtr s;

    bool gotFirstSpike;

    int type;

    Font font;

    double ad16ToUv(int x, int gain);

};


#endif  // SPIKESORTERCANVAS_H_
