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

class SpikeDisplayCache
{
public:
    SpikeDisplayCache () {}
    virtual ~SpikeDisplayCache() {}

    void setMonitor(std::string key, bool isMonitored) {
        monitors[key] = isMonitored;
    };

    bool isMonitored(std::string key) {
        return monitors[key];
    };

    void setRange(std::string key, int channelIdx, double range) {
        ranges[key][channelIdx] = range;
    };

    double getRange(std::string key, int channelIdx) {
        return ranges[key][channelIdx];
    };

    void setThreshold(std::string key,int channelIdx, double thresh) {
        thresholds[key][channelIdx] = thresh;
    };

    double getThreshold(std::string key, int channelIdx) {
        return thresholds[key][channelIdx];
    };

    bool hasCachedDisplaySettings(std::string cacheKey)
    {
        /*
        LOGDD("SpikeDisplayCache keys:");
        std::vector<std::string> keys = extract_keys(ranges);
        std::vector<std::map<int,double>> vals = extract_values(ranges);
        for (int i = 0; i < keys.size(); i++)
        {
            std::vector<int> channels = extract_keys(vals[i]);
            std::vector<double> ranges = extract_values(vals[i]);
            for (int j = 0; j < channels.size(); j++)
                LOGDD("Key: ", keys[i], " Channel: ", channels[j], " Range: ", ranges[j]);
        }
        */
        return thresholds.count(cacheKey) > 0;
    };

    std::string findSimilarKey(std::string key, int streamIndex)
    {
        std::vector<std::string> keys = extract_keys(ranges);

        unsigned sourcePos = 0;
        unsigned streamPos = key.find_first_of("|");
        unsigned namePos = key.find_last_of("|");

        // First check for a source ID change (match only stream + electrode name)
        for (int i = 0; i < keys.size(); i++)
        {
            std::string partToMatch = key.substr(streamPos, key.length() - streamPos);
            std::string possibleMatch = keys[i].substr(streamPos, keys[i].length() - streamPos);
            if (partToMatch.compare(possibleMatch) == 0)
                return keys[i];
        }

        // Next check for a stream name change (match only node + electrode name)
        std::vector<std::string> matches;
        for (int i = 0; i < keys.size(); i++)
        {
            int namePos2 = keys[i].find_last_of("|");
            std::string partToMatch = key.substr(sourcePos, streamPos - sourcePos) + key.substr(namePos, key.length() - namePos);
            std::string possibleMatch = keys[i].substr(sourcePos, streamPos - sourcePos) + keys[i].substr(namePos2, keys[i].length() - namePos2);
            if (partToMatch.compare(possibleMatch) == 0)
                matches.push_back(keys[i]);
        }

        // Check if multiple matches, if so, default to stream index
        if (matches.size() == 1)
            return matches[0];
        else if (matches.size() > streamIndex)
            return matches[streamIndex];

        // No match found
        return "";
    };

private:

    std::map<std::string, std::map<int, double>> ranges;
    std::map<std::string, std::map<int, double>> thresholds;
    std::map<std::string, bool> monitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayCache);
};


class Electrode
{
public:

    /** Constructor */
    Electrode(SpikeSorter* sorter, SpikeChannel* channel, PCAComputingThread* computingThread);

    /** Destructor */
    ~Electrode() { }

    /** Returns true if stream name and local index are the same */
    bool matchesChannel(SpikeChannel* channel);

    /** Updates settings with new SpikeChannel object */
    void updateSettings(SpikeChannel* channel);

    /** Applies cached display settings if inputs change */
    void applyCachedDisplaySettings(SpikeChannel* channel, std::string cacheKey);

    /** Sets 'isActive' to false */
    void reset() { isActive = false; }

    String name;
    String streamName;
    int sourceNodeId;
    String uniqueId;

    int numChannels;
    int numSamples;
    uint16 streamId;

    bool isActive;
  
    std::unique_ptr<SpikePlot> plot;
    std::unique_ptr<Sorter> sorter;

    SpikeSorter* processor;
    PCAComputingThread* computingThread;

    std::string getKey() { return key; }
    

private:

    std::string key; // used for caching

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
    
    /** Used to enable animation */
    bool startAcquisition() override;
    
    /** Used to disable animation */
    bool stopAcquisition() override;

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

    /** Manages connections from SpikeChannels to SpikePlots */
    std::unique_ptr<SpikeDisplayCache> cache;
   
private:

    CriticalSection mut;

    OwnedArray<Electrode> electrodes;
    std::map<const SpikeChannel*, Electrode*> electrodeMap;
    
    PCAComputingThread computingThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorter);

};

#endif  // __SPIKESORTER_H_3F920F95__
