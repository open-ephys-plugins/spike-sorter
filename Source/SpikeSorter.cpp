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

#include "SpikeSorter.h"
#include "SpikeSorterEditor.h"
#include "Containers.h"

#include <stdio.h>


Electrode::Electrode(SpikeSorter* processor_, SpikeChannel* channel, PCAComputingThread* computingThread_)
    : processor(processor_),
      computingThread(computingThread_),
      isActive(true)
{

    name = channel->getName();
    streamName = channel->getStreamName();
    sourceNodeId = channel->getSourceNodeId();

    streamId = channel->getStreamId();
    uniqueId = channel->getUniqueId();

    numChannels = channel->getNumChannels();
    numSamples = channel->getPrePeakSamples() + channel->getPostPeakSamples();
    
    sorter = std::make_unique<Sorter>(this, computingThread);

    plot = std::make_unique<SpikePlot>(processor, this);

    key = channel->getIdentifier().toStdString();

}

bool Electrode::matchesChannel(SpikeChannel* channel)
{
    if (channel->getUniqueId() == uniqueId)
    {
        isActive = true;
        return true;
    }
    else {
        return false;
    }

}

void Electrode::updateSettings(SpikeChannel* channel)
{
    name = channel->getName();

    key = channel->getIdentifier().toStdString();

    if (processor->cache->hasCachedDisplaySettings(key))
    {

        for (int i = 0; i < channel->getNumChannels(); i++)
        {
            plot->setDisplayThresholdForChannel(i, processor->cache->getThreshold(key, i));
            plot->setDisplayRangeForChannel(i, processor->cache->getRange(key, i));
        }

    }

    plot->setName(name);
    plot->refresh();
}

SpikeSorter::SpikeSorter() : GenericProcessor("Spike Sorter")
{

    cache = std::make_unique<SpikeDisplayCache>();

}

AudioProcessorEditor* SpikeSorter::createEditor()
{
    editor = std::make_unique<SpikeSorterEditor>(this);

    return editor.get();
}

bool SpikeSorter::startAcquisition()
{
    
    SpikeSorterEditor* editor = (SpikeSorterEditor*) getEditor();
    
    editor->enable();
    
    return true;
}


bool SpikeSorter::stopAcquisition()
{
    
    SpikeSorterEditor* editor = (SpikeSorterEditor*) getEditor();
    
    editor->disable();
    
    return true;
}



void SpikeSorter::updateSettings()
{

    for (auto electrode : electrodes)
    {
        electrode->reset();
    }

    for (auto spikeChannel : spikeChannels)
    {
        if (spikeChannel->isValid())
        {

            bool foundMatch = false;

            for (auto electrode : electrodes)
            {
                if (electrode->matchesChannel(spikeChannel))
                {
                    electrode->updateSettings(spikeChannel);
                    foundMatch = true;
                    electrodeMap[spikeChannel] = electrode;
                    break;
                }
            }

            if (!foundMatch)
            {

                Electrode* e = new Electrode(this, spikeChannel, &computingThread);
                electrodes.add(e);
                electrodeMap[spikeChannel] = e;
            }
            
        }
    }

}

Array<Electrode*> SpikeSorter::getElectrodesForStream(uint16 streamId)
{
    Array<Electrode*> electrodesForStream;

    for (auto electrode : electrodes)
    {
        if (electrode->streamId == streamId && electrode->isActive)
            electrodesForStream.add(electrode);
    }

    return electrodesForStream;
}

void SpikeSorter::handleSpike(SpikePtr newSpike)
{

    const SpikeChannel* channelInfo = newSpike->getChannelInfo();

    SorterSpikePtr sorterSpike = new SorterSpikeContainer(channelInfo, 
                                                          newSpike->getSortedId(),
                                                          newSpike->getSampleNumber(),
                                                          newSpike->getDataPointer());

    

    Electrode* electrode = electrodeMap[channelInfo];

    if (sorterSpike->checkThresholds(electrode->plot->getDisplayThresholds()))
    {
        electrode->sorter->projectOnPrincipalComponents(sorterSpike);

        electrode->sorter->sortSpike(sorterSpike, true);

        if (electrode->plot->isVisible())
        {
            if (electrode->sorter->isPCAfinished())
            {
                electrode->sorter->resetJobStatus();
                float p1min, p2min, p1max, p2max;
                electrode->sorter->getPCArange(p1min, p2min, p1max, p2max);
                electrode->plot->setPCARange(p1min, p2min, p1max, p2max);
            }

            electrode->plot->processSpikeObject(sorterSpike);
        }

        if (sorterSpike->sortedId > 0)
            newSpike->setSortedId(sorterSpike->sortedId);
    }

    
}

void SpikeSorter::process(AudioBuffer<float>& buffer)
{

    checkForEvents(true);

}

Electrode* SpikeSorter::findMatchingElectrode(String name, String stream_name, int stream_source)
{
    LOGD("Searching for electrode with ", name, " : ", stream_name, " : ", stream_source);
    for (auto electrode : electrodes)
    {
        if (electrode->name == name &&
            electrode->streamName == stream_name &&
            electrode->sourceNodeId == stream_source)
        {
            LOGD("  Found matching electrode!");
            return electrode;
        }
            
    }

    LOGD("  No match ");

    return nullptr;
}

void SpikeSorter::saveCustomParametersToXml(XmlElement* parentElement)
{
    
    for (auto electrode : electrodes)
    {
        
        XmlElement* electrodeNode = parentElement->createNewChildElement("ELECTRODE");

        electrode->plot->saveCustomParametersToXml(electrodeNode);
        electrode->sorter->saveCustomParametersToXml(electrodeNode);

    }

}

void SpikeSorter::loadCustomParametersFromXml(XmlElement* xml)
{

    for (auto* paramsXml : xml->getChildIterator())
    {

        if (paramsXml->hasTagName("ELECTRODE"))
        {
            String name = paramsXml->getStringAttribute("name", "");
            String stream_name = paramsXml->getStringAttribute("stream_name", "");
            int stream_source = paramsXml->getIntAttribute("source_node_id", 0);

            Electrode* electrode = findMatchingElectrode(name, stream_name, stream_source);

            if (electrode != nullptr)
            {
                electrode->sorter->loadCustomParametersFromXml(paramsXml);
                electrode->plot->loadCustomParametersFromXml(paramsXml);
            }
        }
    }
}
