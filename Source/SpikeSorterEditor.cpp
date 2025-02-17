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

#include "SpikeSorterEditor.h"
#include "SpikeSorter.h"
#include "SpikeSorterCanvas.h"

#include <stdio.h>

SpikeSorterEditor::SpikeSorterEditor (GenericProcessor* parentNode)
    : VisualizerEditor (parentNode, "Spike Sorter", 205),
      spikeSorterCanvas (nullptr)

{
    tabText = "Spike Sorter";
    addComboBoxParameterEditor (Parameter::STREAM_SCOPE, "electrode_index", 20, 30);

    ParameterEditor* electrodeIndexEditor = getParameterEditor ("electrode_index");
    electrodeIndexEditor->setLayout (ParameterEditor::Layout::nameOnTop);
    electrodeIndexEditor->setSize (160, 40);
}

Visualizer* SpikeSorterEditor::createNewCanvas()
{
    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    spikeSorterCanvas = new SpikeSorterCanvas (processor);

    updateView();

    return spikeSorterCanvas;
}

void SpikeSorterEditor::selectedStreamHasChanged()
{
    updateView();
    if (selectedStream == 0)
    {
        return;
    }

    SpikeSorter* processor = (SpikeSorter*) getProcessor();

    currentElectrodes = processor->getElectrodesForStream (selectedStream);

    int id = 0;
    int viewedPlot = 1;

    for (auto electrode : currentElectrodes)
    {
        if (electrode->plot->isVisible())
            viewedPlot = id;
    }

    int electrodeIndex = processor->getDataStream (selectedStream)->getParameter ("electrode_index")->getValue();
}

void SpikeSorterEditor::updateView()
{
    if (selectedStream == 0 || !spikeSorterCanvas)
        return;

    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    currentElectrodes = processor->getElectrodesForStream (selectedStream);
    if (currentElectrodes.size() == 0)
    {
        spikeSorterCanvas->setActiveElectrode (nullptr);
        return;
    }

    if (spikeSorterCanvas != nullptr)
    {
        int electrodeIndex = processor->getDataStream (selectedStream)->getParameter ("electrode_index")->getValue();
        spikeSorterCanvas->setActiveElectrode (currentElectrodes[electrodeIndex]);
    }
}

void SpikeSorterEditor::updateSettings()
{
    updateView();
}

void SpikeSorterEditor::nextElectrode()
{
    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    int numAvailable = currentElectrodes.size();
    int currentID = processor->getDataStream (selectedStream)->getParameter ("electrode_index")->getValue();

    int nextID = currentID + 1;

    if (nextID > numAvailable)
        nextID = 1;

    processor->getDataStream (selectedStream)->getParameter ("electrode_index")->setNextValue (nextID);
}

void SpikeSorterEditor::previousElectrode()
{
    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    int numAvailable = currentElectrodes.size();

    int currentID = processor->getDataStream (selectedStream)->getParameter ("electrode_index")->getValue();

    int previousID = currentID - 1;

    if (previousID == 0)
        previousID = numAvailable;

    processor->getDataStream (selectedStream)->getParameter ("electrode_index")->setNextValue (previousID);
}
