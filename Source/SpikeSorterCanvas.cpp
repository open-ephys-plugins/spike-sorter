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

#include "SpikeSorterEditor.h"
#include "SpikeSorter.h"
#include "SpikePlot.h"
#include "PCAUnit.h"
#include "BoxUnit.h"

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

    addUnitButton = new UtilityButton("New Box Unit", Font("Small Text", 13, Font::plain));
    addUnitButton->setRadius(3.0f);
    addUnitButton->addListener(this);
    addAndMakeVisible(addUnitButton);

    addPolygonUnitButton = new UtilityButton("New Polygon Unit", Font("Small Text", 13, Font::plain));
    addPolygonUnitButton->setRadius(3.0f);
    addPolygonUnitButton->addListener(this);
    addAndMakeVisible(addPolygonUnitButton);

    addBoxButton = new UtilityButton("Add Box", Font("Small Text", 13, Font::plain));
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

    nextElectrode = new UtilityButton(">>", Font("Small Text", 13, Font::plain));
    nextElectrode->setRadius(3.0f);
    nextElectrode->addListener(this);
    addAndMakeVisible(nextElectrode);

    prevElectrode = new UtilityButton("<<", Font("Small Text", 13, Font::plain));
    prevElectrode->setRadius(3.0f);
    prevElectrode->addListener(this);
    addAndMakeVisible(prevElectrode);

    addAndMakeVisible(viewport);
    
    addKeyListener(this);

    refreshRate = 10; // Hz

}

void SpikeSorterCanvas::refreshState()
{
    resized();
}

void SpikeSorterCanvas::resized()
{
    viewport->setBounds(130, 10, getWidth() - 140, getHeight()-20);

    spikeDisplay->setBounds(0, 0, getWidth() - 140, spikeDisplay->getTotalHeight());

    nextElectrode->setBounds(90, 10, 40, 20);
    prevElectrode->setBounds(45, 10, 40, 20);

    addUnitButton->setBounds(8, 120, 115, 20);
    addBoxButton->setBounds(8, 150, 115, 20);
    
    addPolygonUnitButton->setBounds(8, 190, 115, 20);
    
    delUnitButton->setBounds(8, 230, 115, 20);

    rePCAButton->setBounds(5, 270, 115, 20);

    newIDbuttons->setBounds(5, 300, 115, 20);
    deleteAllUnits->setBounds(5, 350, 115, 20);

}

void SpikeSorterCanvas::paint(Graphics& g)
{

    g.fillAll(Colours::darkgrey);
}

void SpikeSorterCanvas::refresh()
{
    spikeDisplay->refresh();
}

void SpikeSorterCanvas::setActiveElectrode(Electrode* electrode_)
{
    electrode = electrode_;

    if (electrode != nullptr)
    {
        spikeDisplay->setSpikePlot(electrode->plot.get());
    }
    else {
        spikeDisplay->setSpikePlot(nullptr);
    }

    
}

void SpikeSorterCanvas::removeUnitOrBox()
{
    int unitID, boxID;
    electrode->plot->getSelectedUnitAndBox(unitID, boxID);
    
    bool selectNewBoxUnit = false;
    bool selectNewPCAUnit = false;

    if (unitID > 0)
    {
        if (boxID >= 0)
        {
            // box unit
            int numBoxes = electrode->sorter->getNumBoxes(unitID);
            
            std::cout << "Removing box unit" << std::endl;

            if (numBoxes > 1)
            {
                // delete box, but keep unit
                electrode->sorter->removeBoxFromUnit(unitID, boxID);
                electrode->plot->updateUnits();
                electrode->plot->setSelectedUnitAndBox(unitID, 0);
            }
            else
            {
                // delete unit
                electrode->sorter->removeUnit(unitID);
                electrode->plot->updateUnits();

                std::vector<BoxUnit> boxunits = electrode->sorter->getBoxUnits();
                std::vector<PCAUnit> pcaunits = electrode->sorter->getPCAUnits();
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
                    electrode->plot->setSelectedUnitAndBox(-1, -1);
                }
            }
        }
        else
        {
            // pca unit
            electrode->sorter->removeUnit(unitID);
            electrode->plot->updateUnits();

            std::vector<BoxUnit> boxunits = electrode->sorter->getBoxUnits();
            std::vector<PCAUnit> pcaunits = electrode->sorter->getPCAUnits();
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
                electrode->plot->setSelectedUnitAndBox(-1, -1);
            }


        }
        if (selectNewPCAUnit)
        {
            // set new selected unit to be the last existing unit
            std::vector<PCAUnit> u = electrode->sorter->getPCAUnits();
            if (u.size() > 0)
            {
                electrode->plot->setSelectedUnitAndBox(u[u.size() - 1].getUnitId(), -1);
            }
            else
            {
                electrode->plot->setSelectedUnitAndBox(-1, -1);
            }
        }
        if (selectNewBoxUnit)
        {
            // set new selected unit to be the last existing unit
            std::vector<BoxUnit> u = electrode->sorter->getBoxUnits();
            if (u.size() > 0)
            {
                electrode->plot->setSelectedUnitAndBox(u[u.size() - 1].getUnitId(), 0);
            }
            else
            {
                electrode->plot->setSelectedUnitAndBox(-1, -1);
            }
        }
    }

}

bool SpikeSorterCanvas::keyPressed(const KeyPress& key, Component* c)
{
    if (key.getKeyCode() == KeyPress::deleteKey ||
        key.getKeyCode() == KeyPress::backspaceKey)
    {
        removeUnitOrBox();
        return false;
    }
    
    return true;
}

void SpikeSorterCanvas::buttonClicked(Button* button)
{
    int channel = 0;
    int unitID = -1;
    int boxID = -1;
    Time t;

    if (button == addPolygonUnitButton)
    {

        if (electrode->sorter->firstJobFinished())
        {
            inDrawingPolygonMode = true;
            electrode->plot->setPolygonDrawingMode(true);
        }

        
    }
    else if (button == addUnitButton)
    {

        if (electrode != nullptr)
        {
            int newUnitID = electrode->sorter->addBoxUnit(0);

            uint8 r, g, b;
            electrode->sorter->getUnitColor(newUnitID, r, g, b);
            electrode->plot->updateUnits();
            electrode->plot->setSelectedUnitAndBox(newUnitID, 0);
        }

    }
    else if (button == delUnitButton)
    {
        //std::cout << "Delete button pressed" << std::endl;
        removeUnitOrBox();

    }
    else if (button == addBoxButton)
    {

        electrode->plot->getSelectedUnitAndBox(unitID, boxID);

        if (unitID > 0)
        {
            //std::cout << "Adding box to channel " << channel << " with unitID " << unitID << std::endl;
            electrode->sorter->addBoxToUnit(channel, unitID);
            electrode->plot->updateUnits();
        }
    }
    else if (button == rePCAButton)
    {
        electrode->sorter->RePCA();
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
        electrode->sorter->generateNewIds();
        electrode->plot->updateUnits();
    }
    else if (button == deleteAllUnits)
    {
        // delete all units
        electrode->sorter->removeAllUnits();
        electrode->plot->updateUnits();
        electrode->plot->setSelectedUnitAndBox(-1, -1);
    }

    refresh();
}


SpikeDisplay::SpikeDisplay() : 
    activePlot(nullptr),
    totalHeight(430)
{

}

void SpikeDisplay::clear()
{
    if (activePlot != nullptr)
        activePlot->clear();
}


void SpikeDisplay::setSpikePlot(SpikePlot* plot)
{

    if (activePlot != nullptr)
    {
        activePlot->setVisible(false);
        removeChildComponent(activePlot);
    }

    //std::cout << "Spike display updating active plot" << std::endl;
    
    activePlot = plot;

    if (activePlot != nullptr)
    {
        addAndMakeVisible(activePlot);
    }
    
    resized();
}

void SpikeDisplay::refresh()
{
    if (activePlot != nullptr)
        activePlot->refresh();
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
