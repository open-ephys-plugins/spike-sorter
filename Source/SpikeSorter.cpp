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

#include <stdio.h>
#include "SpikeSorter.h"
#include "SpikeSortBoxes.h"
#include "SpikeSorterCanvas.h"
#include "SpikePlot.h"

SpikeSorter::SpikeSorter()
    : GenericProcessor("Spike Sorter")
{

}

AudioProcessorEditor* SpikeSorter::createEditor()
{
    editor = std::make_unique<SpikeSorterEditor>(this);

    return editor.get();
}

void SpikeSorter::updateSettings()
{

    electrodes.clear();
    electrodeMap.clear();

    for (auto spikeChannel : spikeChannels)
    {
        if (spikeChannel->isValid())
        {
            Electrode* e = new Electrode(spikeChannel, &computingThread);
            electrodes.add(e);
            electrodeMap[spikeChannel] = e;
        }
    }
   
}


Electrode::Electrode(SpikeChannel* channel, PCAcomputingThread* computingThread_)
    : computingThread(computingThread_)
{

    name = channel->getName();
    numChannels = channel->getNumChannels();
    streamId = channel->getStreamId();

    if (computingThread != nullptr)
        spikeSort = new SpikeSortBoxes(name,
                                       computingThread, 
                                       numChannels, 
                                       channel->getSampleRate(), 
                                       channel->getPrePeakSamples() + channel->getPostPeakSamples());


}

Array<Electrode*> SpikeSorter::getElectrodesForStream(uint16 streamId)
{
    Array<Electrode*> electrodesForStream;

    for (auto electrode : electrodes)
    {
        if (electrode->streamId == streamId)
            electrodesForStream.add(electrode);
    }

    return electrodesForStream;
}

void SpikeSorter::setSortedID(const uint8* rawData, uint16 sortedID)
{
    uint8* modifiableBuffer = const_cast<uint8*>(rawData);

    *(reinterpret_cast<uint16*>(modifiableBuffer + 16)) = sortedID;
}

void SpikeSorter::handleSpike(const SpikeChannel* spikeChannel, const EventPacket& spike, int samplePosition, const uint8* rawData)
{
    SpikePtr newSpike = Spike::deserialize(spike, spikeChannel);

    SorterSpikePtr sorterSpike = new SorterSpikeContainer(spikeChannel, newSpike);

    Electrode* electrode = electrodeMap[spikeChannel];

    electrode->spikeSort->projectOnPrincipalComponents(sorterSpike);
    electrode->spikeSort->sortSpike(sorterSpike, true);

    // transfer buffered spikes to spike plot
    if (electrode->spikePlot != nullptr)
    {
        if (electrode->spikeSort->isPCAfinished())
        {
            electrode->spikeSort->resetJobStatus();
            float p1min, p2min, p1max, p2max;
            electrode->spikeSort->getPCArange(p1min, p2min, p1max, p2max);
            electrode->spikePlot->setPCARange(p1min, p2min, p1max, p2max);
        }

        electrode->spikePlot->processSpikeObject(sorterSpike);
    }

    setSortedID(rawData, sorterSpike->sortedId);
    
}

void SpikeSorter::process(AudioBuffer<float>& buffer)
{

    checkForEvents(true);

}

void SpikeSorter::saveCustomParametersToXml(XmlElement* parentElement)
{
    /*XmlElement* mainNode = parentElement->createNewChildElement("SpikeSorter");
    mainNode->setAttribute("numElectrodes", electrodes.size());

    SpikeSorterEditor* ed = (SpikeSorterEditor*) getEditor();

    mainNode->setAttribute("activeElectrode", ed->getSelectedElectrode()-1);
    mainNode->setAttribute("numPreSamples", numPreSamples);
    mainNode->setAttribute("numPostSamples", numPostSamples);
    mainNode->setAttribute("autoDACassignment",	autoDACassignment);
    mainNode->setAttribute("syncThresholds",syncThresholds);
    mainNode->setAttribute("uniqueID",uniqueID);
    mainNode->setAttribute("flipSignal",flipSignal);

    XmlElement* countNode = mainNode->createNewChildElement("ELECTRODE_COUNTER");

    countNode->setAttribute("numElectrodeTypes", (int)electrodeTypes.size());
    for (int k=0; k<electrodeTypes.size(); k++)
    {
        XmlElement* countNode2 = countNode->createNewChildElement("ELECTRODE_TYPE");
        countNode2->setAttribute("type", electrodeTypes[k]);
        countNode2->setAttribute("count", electrodeCounter[k]);
    }

    for (int i = 0; i < electrodes.size(); i++)
    {
        XmlElement* electrodeNode = mainNode->createNewChildElement("ELECTRODE");
        electrodeNode->setAttribute("name", electrodes[i]->name);
        electrodeNode->setAttribute("numChannels", electrodes[i]->numChannels);
        electrodeNode->setAttribute("prePeakSamples", electrodes[i]->prePeakSamples);
        electrodeNode->setAttribute("postPeakSamples", electrodes[i]->postPeakSamples);
        electrodeNode->setAttribute("advancerID", electrodes[i]->advancerID);
        electrodeNode->setAttribute("depthOffsetMM", electrodes[i]->depthOffsetMM);
        electrodeNode->setAttribute("electrodeID", electrodes[i]->electrodeID);

        for (int j = 0; j < electrodes[i]->numChannels; j++)
        {
            XmlElement* channelNode = electrodeNode->createNewChildElement("SUBCHANNEL");
            channelNode->setAttribute("ch",*(electrodes[i]->channels+j));
            channelNode->setAttribute("thresh",*(electrodes[i]->thresholds+j));
            channelNode->setAttribute("isActive",*(electrodes[i]->isActive+j));

        }

        // save spike sorting data.
        electrodes[i]->spikeSort->saveCustomParametersToXml(electrodeNode);

    }*/


}

void SpikeSorter::loadCustomParametersFromXml(XmlElement* xml)
{

    /*if (parametersAsXml != nullptr)
    {

        int electrodeIndex = -1;

        forEachXmlChildElement(*xml, mainNode)
        {

            // use parametersAsXml to restore state

            if (mainNode->hasTagName("SpikeSorter"))
            {
                //int numElectrodes = mainNode->getIntAttribute("numElectrodes");
                currentElectrode = mainNode->getIntAttribute("activeElectrode");
                numPreSamples = mainNode->getIntAttribute("numPreSamples");
                numPostSamples = mainNode->getIntAttribute("numPostSamples");
                autoDACassignment = mainNode->getBoolAttribute("autoDACassignment");
                syncThresholds = mainNode->getBoolAttribute("syncThresholds");
                uniqueID = mainNode->getIntAttribute("uniqueID");
                flipSignal = mainNode->getBoolAttribute("flipSignal");

                forEachXmlChildElement(*mainNode, xmlNode)
                {

                    if (xmlNode->hasTagName("ELECTRODE_COUNTER"))
                    {
                        int numElectrodeTypes = xmlNode->getIntAttribute("numElectrodeTypes");
                        electrodeCounter.resize(numElectrodeTypes);
                        electrodeTypes.resize(numElectrodeTypes);
                        int counter = 0;
                        forEachXmlChildElement(*xmlNode, xmltype)
                        {
                            if (xmltype->hasTagName("ELECTRODE_TYPE"))
                            {
                                electrodeTypes[counter] = xmltype->getStringAttribute("type");
                                electrodeCounter[counter] = xmltype->getIntAttribute("count");
                                counter++;
                            }
                        }
                    }
                    else if (xmlNode->hasTagName("ELECTRODE"))
                    {

                        electrodeIndex++;

                        int channelsPerElectrode = xmlNode->getIntAttribute("numChannels");

                        int advancerID = xmlNode->getIntAttribute("advancerID");
                        float depthOffsetMM = xmlNode->getDoubleAttribute("depthOffsetMM");
                        int electrodeID = xmlNode->getIntAttribute("electrodeID");
                        String electrodeName=xmlNode->getStringAttribute("name");


                        int channelIndex = -1;

                        int* channels = new int[channelsPerElectrode];
                        float* thres = new float[channelsPerElectrode];
                        bool* isActive = new bool[channelsPerElectrode];

                        forEachXmlChildElement(*xmlNode, channelNode)
                        {
                            if (channelNode->hasTagName("SUBCHANNEL"))
                            {
                                channelIndex++;
                                channels[channelIndex] = channelNode->getIntAttribute("ch");
                                thres[channelIndex] = channelNode->getDoubleAttribute("thresh");
                                isActive[channelIndex] = channelNode->getBoolAttribute("isActive");
                            }
                        }

                        int sourceNodeId = 102010; // some number

                        Electrode* newElectrode = new Electrode(electrodeID, 
                            &uniqueIDgenerator,
                            &computingThread, 
                            electrodeName, 
                            channelsPerElectrode, 
                            channels,getDefaultThreshold(),
                            numPreSamples,
                            numPostSamples, 
                            continuousChannels[0]->getSampleRate(), 
                            sourceNodeId,
                            0);

                        for (int k=0; k<channelsPerElectrode; k++)
                        {
                            newElectrode->thresholds[k] = thres[k];
                            newElectrode->isActive[k] = isActive[k];
                        }
			            delete[] channels;
			            delete[] thres;
			            delete[] isActive;

                        newElectrode->advancerID = advancerID;
                        newElectrode->depthOffsetMM = depthOffsetMM;
                        // now read sorted units information
                        newElectrode->spikeSort->loadCustomParametersFromXml(xmlNode);
                        addElectrode(newElectrode);

                    }
                }
            }
        }
    }
    SpikeSorterEditor* ed = (SpikeSorterEditor*) getEditor();
    //	ed->updateAdvancerList();

    if (currentElectrode >= 0)
    {
        ed->refreshElectrodeList(currentElectrode);
        ed->setSelectedElectrode(1+currentElectrode);
    }
    else
    {
        ed->refreshElectrodeList();
    }*/

}
