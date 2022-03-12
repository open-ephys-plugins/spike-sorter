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

#include "PCAComputingThread.h"
#include "Sorter.h"
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
    Electrode(SpikeChannel* channel, PCAComputingThread* computingThread);

    /** Destructor */
    ~Electrode() { }

    /** Returns true if stream name and local index are the same */
    bool matchesChannel(SpikeChannel* channel);

    /** Updates settings with new SpikeChannel object */
    void updateSettings(SpikeChannel* channel);

    /** Sets 'isActive' to false */
    void reset() { isActive = false; }

    String name;
    String streamName;
    int sourceNodeId;
    Uuid uniqueId;

    int numChannels;
    int numSamples;
    uint16 streamId;

    bool isActive;
  
    std::unique_ptr<SpikePlot> plot;
    std::unique_ptr<Sorter> sorter;

    PCAComputingThread* computingThread;

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
    void handleSpike(SpikePtr spike) override;

    /** Called whenever the signal chain is altered. */
    void updateSettings() override;

    /** Creates the SpikeSorterEditor. */
    AudioProcessorEditor* createEditor() override;

    /** Returns an array of available electrodes*/
    Array<Electrode*> getElectrodesForStream(uint16 streamId);

    /** Finds a matching electrode based on names and IDs */
    Electrode* findMatchingElectrode(String name, String stream_name, int stream_source);

    /** Saves all custom parameters */
    void saveCustomParametersToXml(XmlElement* parentElement) override;

    /** Loads all custom parameters*/
    void loadCustomParametersFromXml(XmlElement* xml) override;
   
private:

    CriticalSection mut;

    OwnedArray<Electrode> electrodes;
    std::map<const SpikeChannel*, Electrode*> electrodeMap;
    
    PCAComputingThread computingThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorter);

};

#endif  // __SPIKESORTER_H_3F920F95__
