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

#include "Containers.h"

#include <vector>

class SpikePlot;
class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;
class SpikeSorter;
class Electrode;


/**

  Displays spike waveforms and projections for Spike Sorter

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/
class SpikeSorterCanvas :
    public Visualizer,
    public Button::Listener,
    public KeyListener

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

    /** Called when the component's tab becomes visible again*/
    void refreshState();

    /** Creates spike displays for incoming spike channels*/
    void update() { }

    /** Updates size of spike display*/
    void resized();

    /** Responds to button clicks*/
    void buttonClicked(Button* button);

    /** Updates the current electrode */
    void setActiveElectrode(Electrode* electrode);
    
    /** Responds to keypress*/
    bool keyPressed(const KeyPress& key, Component*);

    /** Pointer to the underlying processor */
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
    
    /** Deletes currently selected unit or box */
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
    SpikeDisplay();

    /** Destructor */
    ~SpikeDisplay() { }

    /** Clears the current plot*/
    void clear();

    /** Sets the spike plot to display */
    void setSpikePlot(SpikePlot* plot);
    
    /** Called on each animation cycle*/
    void refresh();

    /** Resizes spike plot location*/
    void resized();

    /** Sets polygon drawing mode in the active plot*/
    void setPolygonMode(bool on);

    /** Returns the total height of the display */
    int getTotalHeight()
    {
        return totalHeight;
    }

private:

    int totalHeight;
    
    SpikePlot* activePlot;

};

/** 

    Base class for WaveformAxes and PCAProjectionAxes

*/
class GenericDrawAxes : public Component
{
public:

    enum AxesType {
        WAVE1 = 0,
        WAVE2,
        WAVE3,
        WAVE4,
        PCA = 4
    };

    /** Constructor */
    GenericDrawAxes(AxesType t);

    /** Destructor */
    virtual ~GenericDrawAxes();

    /** Add new spike to the plot */
    virtual bool updateSpikeData(SorterSpikePtr s);

    void setXLims(double xmin, double xmax);
    void getXLims(double* xmin, double* xmax);
    void setYLims(double ymin, double ymax);
    void getYLims(double* ymin, double* ymax);

    void setType(AxesType type);
    AxesType getType();

    virtual void paint(Graphics& g) = 0;

    int roundUp(int, int);
    void makeLabel(int val, int gain, bool convert, char* s);

protected:
    double xlims[2];
    double ylims[2];

    SorterSpikePtr s;

    bool gotFirstSpike;

    AxesType type;

    Font font;

    double ad16ToUv(int x, int gain);

};


#endif  //
