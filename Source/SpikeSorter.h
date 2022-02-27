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

#ifndef __SPIKESORTER_H_3F920F95__
#define __SPIKESORTER_H_3F920F95__

#include <ProcessorHeaders.h>

#include "SpikeSortBoxes.h"
#include "SpikePlot.h"

#include <algorithm>    // Needed for std::sort
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


class Electrode
{
public:

    /** Constructor */
    Electrode(SpikeChannel* channel, PCAcomputingThread* computingThread);

    /** Destructor */
    ~Electrode() { }

    String name;
    int numChannels;
    int numSamples;
    uint16 streamId;
  
    std::unique_ptr<SpikePlot> spikePlot;
    std::unique_ptr<SpikeSortBoxes> spikeSort;

    PCAcomputingThread* computingThread;

};


class SpikeSorter : public GenericProcessor
{
public:

    /** Constructor */
    SpikeSorter();

    /** Destructor */
    ~SpikeSorter() { }

    /** Calls checkForEvents(true) */
    void process(AudioBuffer<float>& buffer) override;

    /** Handles incoming spikes */
    void handleSpike(const SpikeChannel* spikeChannel, const EventPacket& spike, int samplePosition, const uint8* rawData) override;

    /** Updates the sortedID of an incoming spike*/
    void setSortedID(const uint8* rawData, uint16 sortedID);

    /** Called whenever the signal chain is altered. */
    void updateSettings() override;

    /** Creates the SpikeSorterEditor. */
    AudioProcessorEditor* createEditor() override;

    /** Returns an array of available electrodes*/
    Array<Electrode*> getElectrodesForStream(uint16 streamId);

    /** Saves all custom parameters */
    void saveCustomParametersToXml(XmlElement* parentElement) override;

    /** Loads all custom parameters*/
    void loadCustomParametersFromXml(XmlElement* xml) override;
   
private:

    CriticalSection mut;

    OwnedArray<Electrode> electrodes;
    std::map<const SpikeChannel*, Electrode*> electrodeMap;
    PCAcomputingThread computingThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorter);

};

#endif  // __SPIKESORTER_H_3F920F95__
