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
#include "SpikeSorterCanvas.h"
#include "SpikeSorter.h"

#include <stdio.h>

SpikeSorterEditor::SpikeSorterEditor(GenericProcessor* parentNode)
    : VisualizerEditor(parentNode, 205),
      spikeSorterCanvas(nullptr)

{
    tabText = "Spike Sorter";

    electrodeList = new ComboBox("Electrode List");
    electrodeList->addListener(this);
    electrodeList->setBounds(20,70,140,20);
    addAndMakeVisible(electrodeList);

    electrodeSelectionLabel = new Label("Label", "Active Electrode:");
    electrodeSelectionLabel->setBounds(17, 40, 180, 20);
    addAndMakeVisible(electrodeSelectionLabel);
}

Visualizer* SpikeSorterEditor::createNewCanvas()
{

    SpikeSorter* processor = (SpikeSorter*) getProcessor();
    spikeSorterCanvas = new SpikeSorterCanvas(processor);

    selectedStreamHasChanged();

    return spikeSorterCanvas;
}

void SpikeSorterEditor::selectedStreamHasChanged()
{

    electrodeList->clear();

    if (selectedStream == 0)
    {
        return;
    }

    SpikeSorter* processor = (SpikeSorter*)getProcessor();

    currentElectrodes = processor->getElectrodesForStream(selectedStream);

    int id = 0;
    int viewedPlot = 1;

    for (auto electrode : currentElectrodes)
    {

        electrodeList->addItem(electrode->name, ++id);
            
        if (electrode->plot->isVisible())
            viewedPlot = id;
            
    }

    electrodeList->setSelectedId(viewedPlot, true);
}

void SpikeSorterEditor::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == electrodeList)
    {
        int index = electrodeList->getSelectedId() - 1;

        if (spikeSorterCanvas != nullptr)
            spikeSorterCanvas->setActiveElectrode(currentElectrodes[index]);
    }
   
}

void SpikeSorterEditor::nextElectrode()
{
    int numAvailable = electrodeList->getNumItems();

    int currentID = electrodeList->getSelectedId();

    int nextID = currentID + 1;

    if (nextID > numAvailable)
        nextID = 1;

    electrodeList->setSelectedId(nextID, sendNotification);
}

void SpikeSorterEditor::previousElectrode()
{
    int numAvailable = electrodeList->getNumItems();

    int currentID = electrodeList->getSelectedId();

    int previousID = currentID - 1;

    if (previousID == 0)
        previousID = numAvailable;

    electrodeList->setSelectedId(previousID, sendNotification);
}