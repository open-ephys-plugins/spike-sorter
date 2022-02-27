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

#include "SpikeSorterCanvas.h"

#include "SpikeSorter.h"
#include "SpikePlot.h"


SpikeSorterCanvas::SpikeSorterCanvas(SpikeSorter* n) :
    processor(n), newSpike(false)
{
    electrode = nullptr;
    viewport = new Viewport();
    spikeDisplay = new SpikeDisplay();

    viewport->setViewedComponent(spikeDisplay, false);
    viewport->setScrollBarsShown(true, true);

    inDrawingPolygonMode = false;
    scrollBarThickness = viewport->getScrollBarThickness();

    addUnitButton = new UtilityButton("New box unit", Font("Small Text", 13, Font::plain));
    addUnitButton->setRadius(3.0f);
    addUnitButton->addListener(this);
    addAndMakeVisible(addUnitButton);

    addPolygonUnitButton = new UtilityButton("New polygon", Font("Small Text", 13, Font::plain));
    addPolygonUnitButton->setRadius(3.0f);
    addPolygonUnitButton->addListener(this);
    addAndMakeVisible(addPolygonUnitButton);

    addBoxButton = new UtilityButton("Add box", Font("Small Text", 13, Font::plain));
    addBoxButton->setRadius(3.0f);
    addBoxButton->addListener(this);
    addAndMakeVisible(addBoxButton);

    delUnitButton = new UtilityButton("Delete", Font("Small Text", 13, Font::plain));
    delUnitButton->setRadius(3.0f);
    delUnitButton->addListener(this);
    addAndMakeVisible(delUnitButton);

    rePCAButton = new UtilityButton("Re-PCA", Font("Small Text", 13, Font::plain));
    rePCAButton->setRadius(3.0f);
    rePCAButton->addListener(this);
    addAndMakeVisible(rePCAButton);

    newIDbuttons = new UtilityButton("New IDs", Font("Small Text", 13, Font::plain));
    newIDbuttons->setRadius(3.0f);
    newIDbuttons->addListener(this);
    addAndMakeVisible(newIDbuttons);

    deleteAllUnits = new UtilityButton("Delete All", Font("Small Text", 13, Font::plain));
    deleteAllUnits->setRadius(3.0f);
    deleteAllUnits->addListener(this);
    addAndMakeVisible(deleteAllUnits);

    nextElectrode = new UtilityButton("Next Electrode", Font("Small Text", 13, Font::plain));
    nextElectrode->setRadius(3.0f);
    nextElectrode->addListener(this);
    addAndMakeVisible(nextElectrode);

    prevElectrode = new UtilityButton("Prev Electrode", Font("Small Text", 13, Font::plain));
    prevElectrode->setRadius(3.0f);
    prevElectrode->addListener(this);
    addAndMakeVisible(prevElectrode);

    addAndMakeVisible(viewport);

    setWantsKeyboardFocus(true);

    update();

}

void SpikeSorterCanvas::beginAnimation()
{
    startCallbacks();
}

void SpikeSorterCanvas::endAnimation()
{
    stopCallbacks();
}

void SpikeSorterCanvas::update()
{

    std::cout << "Updating SpikeSorterCanvas" << std::endl;

    /*int nPlots = processor->getNumElectrodes();
    processor->removeSpikePlots();
    spikeDisplay->removePlots();

    if (nPlots > 0)
    {
        // Plot only active electrode
        int currentElectrode = processor->getCurrentElectrodeIndex();
        electrode = processor->getActiveElectrode();
        SpikeHistogramPlot* sp = spikeDisplay->addSpikePlot(processor->getNumberOfChannelsForElectrode(currentElectrode), electrode->electrodeID,
                                                            processor->getNameForElectrode(currentElectrode));
        processor->addSpikePlotForElectrode(sp, currentElectrode);
        electrode->spikePlot->setFlipSignal(processor->getFlipSignalState());
        electrode->spikePlot->updateUnitsFromProcessor();

    }
    spikeDisplay->resized();
    spikeDisplay->repaint();*/
}


void SpikeSorterCanvas::refreshState()
{
    resized();
}

void SpikeSorterCanvas::resized()
{
    viewport->setBounds(130, 0, getWidth() - 140, getHeight());

    spikeDisplay->setBounds(0, 0, getWidth() - 140, spikeDisplay->getTotalHeight());

    nextElectrode->setBounds(0, 20, 120, 30);
    prevElectrode->setBounds(0, 60, 120, 30);

    addUnitButton->setBounds(0, 120, 120, 20);
    addPolygonUnitButton->setBounds(0, 150, 120, 20);
    addBoxButton->setBounds(0, 180, 120, 20);
    delUnitButton->setBounds(0, 210, 120, 20);

    rePCAButton->setBounds(0, 240, 120, 20);

    newIDbuttons->setBounds(0, 270, 120, 20);
    deleteAllUnits->setBounds(0, 300, 120, 20);

}

void SpikeSorterCanvas::paint(Graphics& g)
{

    g.fillAll(Colours::darkgrey);

}

void SpikeSorterCanvas::refresh()
{
    spikeDisplay->repaint();
}


void SpikeSorterCanvas::setActiveElectrode(Electrode* electrode_)
{
    electrode = electrode_;

    spikeDisplay->setSpikePlot(electrode->spikePlot.get());
}

void SpikeSorterCanvas::removeUnitOrBox()
{
    int unitID, boxID;
    electrode->spikePlot->getSelectedUnitAndBox(unitID, boxID);
    bool selectNewBoxUnit = false;
    bool selectNewPCAUnit = false;

    if (unitID > 0)
    {
        if (boxID >= 0)
        {
            // box unit
            int numBoxes = electrode->spikeSort->getNumBoxes(unitID);

            if (numBoxes > 1)
            {
                // delete box, but keep unit
                electrode->spikeSort->removeBoxFromUnit(unitID, boxID);
                electrode->spikePlot->updateUnits();
                electrode->spikePlot->setSelectedUnitAndBox(unitID, 0);
            }
            else
            {
                // delete unit
                electrode->spikeSort->removeUnit(unitID);
                electrode->spikePlot->updateUnits();

                std::vector<BoxUnit> boxunits = electrode->spikeSort->getBoxUnits();
                std::vector<PCAUnit> pcaunits = electrode->spikeSort->getPCAUnits();
                if (boxunits.size() > 0)
                {
                    selectNewBoxUnit = true;
                }
                else if (pcaunits.size() > 0)
                {
                    selectNewPCAUnit = true;
                }
                else
                {
                    electrode->spikePlot->setSelectedUnitAndBox(-1, -1);
                }
            }
        }
        else
        {
            // pca unit
            electrode->spikeSort->removeUnit(unitID);
            electrode->spikePlot->updateUnits();

            std::vector<BoxUnit> boxunits = electrode->spikeSort->getBoxUnits();
            std::vector<PCAUnit> pcaunits = electrode->spikeSort->getPCAUnits();
            if (pcaunits.size() > 0)
            {
                selectNewPCAUnit = true;
            }
            else if (boxunits.size() > 0)
            {
                selectNewBoxUnit = true;
            }
            else
            {
                electrode->spikePlot->setSelectedUnitAndBox(-1, -1);
            }


        }
        if (selectNewPCAUnit)
        {
            // set new selected unit to be the last existing unit
            std::vector<PCAUnit> u = electrode->spikeSort->getPCAUnits();
            if (u.size() > 0)
            {
                electrode->spikePlot->setSelectedUnitAndBox(u[u.size() - 1].getUnitID(), -1);
            }
            else
            {
                electrode->spikePlot->setSelectedUnitAndBox(-1, -1);
            }
        }
        if (selectNewBoxUnit)
        {
            // set new selected unit to be the last existing unit
            std::vector<BoxUnit> u = electrode->spikeSort->getBoxUnits();
            if (u.size() > 0)
            {
                electrode->spikePlot->setSelectedUnitAndBox(u[u.size() - 1].getUnitID(), 0);
            }
            else
            {
                electrode->spikePlot->setSelectedUnitAndBox(-1, -1);
            }
        }
    }

}

bool SpikeSorterCanvas::keyPressed(const KeyPress& key)
{

    KeyPress c = KeyPress::createFromDescription("c");
    KeyPress e = KeyPress::createFromDescription("escape");
    KeyPress d = KeyPress::createFromDescription("delete");

    if (key.isKeyCode(c.getKeyCode())) // C
    {
        spikeDisplay->clear();

        std::cout << "Clearing display" << std::endl;
        return true;
    }
    else  if (key.isKeyCode(e.getKeyCode()))   // ESC
    {
        spikeDisplay->setPolygonMode(false);
        return true;
    }
    else  if (key.isKeyCode(d.getKeyCode()))   // Delete
    {
        removeUnitOrBox();
        return true;
    }

    return false;

}

void SpikeSorterCanvas::buttonClicked(Button* button)
{
    int channel = 0;
    int unitID = -1;
    int boxID = -1;
    Time t;

    if (button == addPolygonUnitButton)
    {
        inDrawingPolygonMode = true;
        addPolygonUnitButton->setToggleState(true, dontSendNotification);
        electrode->spikePlot->setPolygonDrawingMode(true);
    }
    else if (button == addUnitButton)
    {

        if (electrode != nullptr)
        {
            int newUnitID = electrode->spikeSort->addBoxUnit(0);

            uint8 r, g, b;
            electrode->spikeSort->getUnitColor(newUnitID, r, g, b);
            electrode->spikePlot->updateUnits();
            electrode->spikePlot->setSelectedUnitAndBox(newUnitID, 0);
        }

    }
    else if (button == delUnitButton)
    {
        removeUnitOrBox();

    }
    else if (button == addBoxButton)
    {

        electrode->spikePlot->getSelectedUnitAndBox(unitID, boxID);

        if (unitID > 0)
        {
            std::cout << "Adding box to channel " << channel << " with unitID " << unitID << std::endl;
            electrode->spikeSort->addBoxToUnit(channel, unitID);
            electrode->spikePlot->updateUnits();
        }
    }
    else if (button == rePCAButton)
    {
        electrode->spikeSort->RePCA();
    }
    else if (button == nextElectrode)
    {
        SpikeSorterEditor* ed = (SpikeSorterEditor*)processor->getEditor();
        ed->nextElectrode();
    }
    else if (button == prevElectrode)
    {

        SpikeSorterEditor* ed = (SpikeSorterEditor*)processor->getEditor();

        ed->previousElectrode();

    }
    else if (button == newIDbuttons)
    {
        electrode->spikeSort->generateNewIDs();
        electrode->spikePlot->updateUnits();
    }
    else if (button == deleteAllUnits)
    {
        // delete unit
        electrode->spikeSort->removeAllUnits();
    }


    repaint();
}


SpikeDisplay::SpikeDisplay() : totalHeight(430), activePlot(nullptr)
{

}

void SpikeDisplay::clear()
{
    activePlot->clear();
}


void SpikeDisplay::setSpikePlot(SpikePlot* plot)
{

    if (activePlot != nullptr)
    {
        activePlot->setVisible(false);
        removeChildComponent(activePlot);
    }
    
    activePlot = plot;
    addAndMakeVisible(activePlot);
    resized();
}

void SpikeDisplay::paint(Graphics& g)
{

    g.fillAll(Colours::grey);

}

void SpikeDisplay::setPolygonMode(bool on)
{
    if (activePlot != nullptr)
        activePlot->setPolygonDrawingMode(on);
}

void SpikeDisplay::resized()
{

    if (activePlot != nullptr)
        activePlot->setBounds(0, 0, getWidth(), totalHeight);

    setBounds(0, 0, getWidth(), totalHeight);

}


GenericDrawAxes::GenericDrawAxes(GenericDrawAxes::AxesType t)
    : gotFirstSpike(false), type(t)
{
    ylims[0] = 0;
    ylims[1] = 1;

    xlims[0] = 0;
    xlims[1] = 1;

    font = Font("Default", 12, Font::plain);

}

GenericDrawAxes::~GenericDrawAxes()
{

}

bool GenericDrawAxes::updateSpikeData(SorterSpikePtr newSpike)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    s = newSpike;
    return true;
}

void GenericDrawAxes::setYLims(double ymin, double ymax)
{

    //std::cout << "setting y limits to " << ymin << " " << ymax << std::endl;
    ylims[0] = ymin;
    ylims[1] = ymax;
}
void GenericDrawAxes::getYLims(double* min, double* max)
{
    *min = ylims[0];
    *max = ylims[1];
}
void GenericDrawAxes::setXLims(double xmin, double xmax)
{
    xlims[0] = xmin;
    xlims[1] = xmax;
}
void GenericDrawAxes::getXLims(double* min, double* max)
{
    *min = xlims[0];
    *max = xlims[1];
}


void GenericDrawAxes::setType(GenericDrawAxes::AxesType t)
{
    if (t < GenericDrawAxes::WAVE1 || t > GenericDrawAxes::PCA)
    {
        std::cout << "Invalid Axes type specified";
        return;
    }

    type = t;
}

GenericDrawAxes::AxesType GenericDrawAxes::getType()
{
    return type;
}

int GenericDrawAxes::roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
    {
        return numToRound;
    }

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
}


void GenericDrawAxes::makeLabel(int val, int gain, bool convert, char* s)
{
    if (convert)
    {
        double volt = ad16ToUv(val, gain) / 1000.;
        if (abs(val) > 1e6)
        {
            //val = val/(1e6);
            sprintf(s, "%.2fV", volt);
        }
        else if (abs(val) > 1e3)
        {
            //val = val/(1e3);
            sprintf(s, "%.2fmV", volt);
        }
        else
            sprintf(s, "%.2fuV", volt);
    }
    else
    {
        sprintf(s, "%d", (int)val);
    }
}

double GenericDrawAxes::ad16ToUv(int x, int gain)
{
    int result = (double)(x * 20e6) / (double)(gain * pow(2.0, 16));
    return result;
}
