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





Electrode::Electrode(SpikeChannel* channel, PCAComputingThread* computingThread_)
    : computingThread(computingThread_),
      isActive(true)
{

    name = channel->getName();
    streamId = channel->getStreamId();
    uniqueId = channel->getUniqueId();

    numChannels = channel->getNumChannels();
    numSamples = channel->getPrePeakSamples() + channel->getPostPeakSamples();
    
    sorter = std::make_unique<Sorter>(this, computingThread);

    plot = std::make_unique<SpikePlot>(this);

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

    plot->setName(name);
}

SpikeSorter::SpikeSorter() : GenericProcessor("Spike Sorter")
{

}

AudioProcessorEditor* SpikeSorter::createEditor()
{
    editor = std::make_unique<SpikeSorterEditor>(this);

    return editor.get();
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
                Electrode* e = new Electrode(spikeChannel, &computingThread);
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
                                                          newSpike->getTimestamp(),
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

void SpikeSorter::saveCustomParametersToXml(XmlElement* parentElement)
{
    
    for (auto electrode : electrodes)
    {
        
        XmlElement* electrodeNode = parentElement->createNewChildElement("ELECTRODE");

        electrode->sorter->saveCustomParametersToXml(electrodeNode);

    }

}

void SpikeSorter::loadCustomParametersFromXml(XmlElement* xml)
{


}
