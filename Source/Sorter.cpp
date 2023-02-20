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
#include <algorithm>

#include "Sorter.h"
#include "SpikeSorter.h"
#include "PCAComputingThread.h"

#include "BoxUnit.h"
#include "PCAUnit.h"

int Sorter::nextUnitId = 1;

Sorter::Sorter(Electrode* electrode_, PCAComputingThread* pcaThread_)
    : electrode(electrode_),
      computingThread(pcaThread_),
      bufferSize(200),
      spikeBufferIndex(-1),
      bPCAComputed(false),
      bPCAJobFinished(false),
      bPCAJobSubmitted(false),
      bPCAFirstJobFinished(false),
      bRePCA(false),
      selectedUnit(-1),
      selectedBox(-1),
      pc1min(-5),
      pc2min(-5),
      pc1max(5),
      pc2max(5),
      numChannels(electrode_->numChannels),
      waveformLength(electrode_->numSamples)
     
{

    pc1 = new float[int64(numChannels) * waveformLength];
    pc2 = new float[int64(numChannels) * waveformLength];

    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }
}

void Sorter::resizeWaveform(int numSamples)
{
    const ScopedLock myScopedLock(mut);

    waveformLength = numSamples;
    
    delete[] pc1;
    delete[] pc2;

    pc1 = new float[int64(numChannels) * waveformLength];
    pc2 = new float[int64(numChannels) * waveformLength];

    spikeBuffer.clear();
    
    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }
    
    bPCAComputed = false;
    spikeBufferIndex = -1;
	bPCAJobSubmitted = false;
	bPCAJobFinished = false;
	selectedUnit = -1;
	selectedBox = -1;
	bRePCA = false;
	pc1min = -1;
	pc2min = -1;
	pc1max = 1;
	pc2max = 1;

}

Sorter::~Sorter()
{
    delete[] pc1;
    delete[] pc2;
    pc1 = nullptr;
    pc2 = nullptr;
}

void Sorter::setSelectedUnitAndBox(int unitID, int boxID)
{
    selectedUnit = unitID;
    selectedBox = boxID;
}

void Sorter::getSelectedUnitAndBox(int& unitID, int& boxid)
{
    unitID = selectedUnit;
    boxid = selectedBox;
}

void Sorter::projectOnPrincipalComponents(SorterSpikePtr so)
{

    // 1. Add spike to buffer
    spikeBufferIndex++;
    spikeBufferIndex %= bufferSize;
    spikeBuffer.set(spikeBufferIndex, so);

    // 2. Check whether current PCA job has finished
    if (bPCAJobFinished)
    {
        bPCAComputed = true;

        if (!bPCAFirstJobFinished)
            bPCAFirstJobFinished = true;
    }

    // 3. If job has finished, project spike onto PC axes
    if (bPCAComputed)
    {
        

        so->pcProj[0] = so->pcProj[1] = 0;

        const int maxSample = so->getChannel()->getNumChannels() * so->getChannel()->getTotalSamples();

        for (int k = 0; k < maxSample; k++)
        {
            
            float v = so->spikeDataIndexToMicrovolts(k);

            so->pcProj[0] += pc1[k]* v;
            so->pcProj[1] += pc2[k]* v;

        }

        return;

    }

    // 4. If we have enough spikes, start a new PCA job
    if ((spikeBufferIndex == bufferSize -1 && !bPCAComputed && !bPCAJobSubmitted) || bRePCA)
    {
        bPCAJobSubmitted = true;
	    bPCAComputed = false;
        bRePCA = false;

        PCAJobPtr job = new PCAjob(spikeBuffer, pc1, pc2, pc1min, pc2min, pc1max, pc2max, bPCAJobFinished);
        computingThread->addPCAjob(job);
    }

}

void Sorter::getPCArange(float& p1min,float& p2min, float& p1max,  float& p2max)
{
    p1min = pc1min;
    p2min = pc2min;
    p1max = pc1max;
    p2max = pc2max;
}

void Sorter::setPCArange(float p1min,float p2min, float p1max,  float p2max)
{
    pc1min = p1min;
    pc2min = p2min;
    pc1max = p1max;
    pc2max = p2max;
}

void Sorter::resetJobStatus()
{
    bPCAJobFinished = false;
}

bool Sorter::isPCAfinished()
{
    return bPCAJobFinished;
}

bool Sorter::firstJobFinished()
{
    return bPCAFirstJobFinished;
}

void Sorter::RePCA()
{
    if (bPCAComputed)
    {
        bPCAComputed = false;
        bPCAJobSubmitted = false;
        bRePCA = true;
    }
}

void Sorter::addPCAunit(PCAUnit unit)
{
    const ScopedLock myScopedLock(mut);
    pcaUnits.push_back(unit);
}

int Sorter::addBoxUnit(int channel)
{
    const ScopedLock myScopedLock(mut);

    BoxUnit unit(Sorter::generateUnitId());
    boxUnits.push_back(unit);
    setSelectedUnitAndBox(nextUnitId, 0);

    return nextUnitId;
}

int Sorter::addBoxUnit(int channel, Box B)
{
    const ScopedLock myScopedLock(mut);

    BoxUnit unit(B, Sorter::generateUnitId());
    boxUnits.push_back(unit);
    setSelectedUnitAndBox(nextUnitId, 0);

    return nextUnitId;
}

void Sorter::getUnitColor(int unitId, uint8& R, uint8& G, uint8& B)
{
    
    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitId)
        {
            R = boxUnits[k].colorRGB[0];
            G = boxUnits[k].colorRGB[1];
            B = boxUnits[k].colorRGB[2];
            break;
        }
    }

    for (int k = 0; k < pcaUnits.size(); k++)
    {
        if (pcaUnits[k].getUnitId() == unitId)
        {
            R = pcaUnits[k].colorRGB[0];
            G = pcaUnits[k].colorRGB[1];
            B = pcaUnits[k].colorRGB[2];
            break;
        }
    }
}


int Sorter::generateUnitId()
{

    return nextUnitId++;

}

void Sorter::generateNewIds()
{
    const ScopedLock myScopedLock(mut);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        boxUnits[k].unitId = generateUnitId();
        boxUnits[k].updateColor();
    }
    for (int k = 0; k < pcaUnits.size(); k++)
    {
        pcaUnits[k].unitId = generateUnitId();
        pcaUnits[k].updateColor();
    }
}

void Sorter::removeAllUnits()
{
    const ScopedLock myScopedLock(mut);
    boxUnits.clear();
    pcaUnits.clear();
}

bool Sorter::removeUnit(int unitID)
{
    const ScopedLock myScopedLock(mut);
    
    LOGD("Sorter::removeUnit() ", unitID);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitID)
        {
            boxUnits.erase(boxUnits.begin()+k);
            return true;
        }
    }

    for (int k = 0; k < pcaUnits.size(); k++)
    {
        if (pcaUnits[k].getUnitId() == unitID)
        {
            pcaUnits.erase(pcaUnits.begin()+k);
            return true;
        }
    }

    return false;

}

bool Sorter::addBoxToUnit(int channel, int unitID)
{
    const ScopedLock myScopedLock(mut);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitID)
        {
            Box B = boxUnits[k].lstBoxes[boxUnits[k].lstBoxes.size() - 1];
            B.x += 100;
            B.y -= 30;
            B.channel = channel;
            boxUnits[k].addBox(B);
            setSelectedUnitAndBox(unitID, (int) boxUnits[k].lstBoxes.size() - 1);
            return true;
        }
    }

    return false;
}


bool Sorter::addBoxToUnit(int channel, int unitID, Box B)
{
    const ScopedLock myScopedLock(mut);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitID)
        {
            boxUnits[k].addBox(B);
            return true;
        }
    }

    return false;
}

std::vector<BoxUnit> Sorter::getBoxUnits()
{
    const ScopedLock myScopedLock(mut);
    std::vector<BoxUnit> unitsCopy = boxUnits;
    return unitsCopy;
}


std::vector<PCAUnit> Sorter::getPCAUnits()
{
    const ScopedLock myScopedLock(mut);
    std::vector<PCAUnit> unitsCopy = pcaUnits;
    return unitsCopy;
}

void Sorter::updatePCAUnits(std::vector<PCAUnit> _units)
{
    const ScopedLock myScopedLock(mut);
    pcaUnits = _units;
}

void Sorter::updateBoxUnits(std::vector<BoxUnit> _units)
{
    const ScopedLock myScopedLock(mut);
    boxUnits = _units;
}


bool Sorter::checkBoxUnits(SorterSpikePtr spike)
{
    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].isWaveFormInsideAllBoxes(spike))
        {
            spike->sortedId = boxUnits[k].getUnitId();
            spike->color[0] = boxUnits[k].colorRGB[0];
            spike->color[1] = boxUnits[k].colorRGB[1];
            spike->color[2] = boxUnits[k].colorRGB[2];
            boxUnits[k].updateWaveform(spike);
            return true;
        }
    }
}

bool Sorter::checkPCAUnits(SorterSpikePtr spike)
{
    for (int k = 0; k < pcaUnits.size(); k++)
    {
        if (pcaUnits[k].isWaveFormInsidePolygon(spike))
        {
            spike->sortedId = pcaUnits[k].getUnitId();
            spike->color[0] = pcaUnits[k].colorRGB[0];
            spike->color[1] = pcaUnits[k].colorRGB[1];
            spike->color[2] = pcaUnits[k].colorRGB[2];
            return true;
        }
    }
}

bool Sorter::sortSpike(SorterSpikePtr spike, bool PCAfirst)
{
    const ScopedLock myScopedLock(mut);

    if (PCAfirst)
    {
        if (checkPCAUnits(spike))
            return true;

        if (checkBoxUnits(spike))
            return true;
    }
    else
    {
        if (checkBoxUnits(spike))
            return true;

        if (checkPCAUnits(spike))
            return true;
    }

    return false;
}


bool Sorter::removeBoxFromUnit(int unitId, int boxIndex)
{
    const ScopedLock myScopedLock(mut);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitId)
        {
            bool s= boxUnits[k].deleteBox(boxIndex);
            setSelectedUnitAndBox(-1,-1);

            return s;
        }
    }

    return false;
}

std::vector<Box> Sorter::getUnitBoxes(int unitId)
{
    std::vector<Box> boxes;
    const ScopedLock myScopedLock(mut);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitId)
        {

            boxes = boxUnits[k].getBoxes();

            return boxes;
        }
    }

    return boxes;
}


int Sorter::getNumBoxes(int unitId)
{
    const ScopedLock myScopedLock(mut);

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitId() == unitId)
        {

            int n = boxUnits[k].getNumBoxes();
            return n;
        }
    }

    return -1;
}

void Sorter::saveCustomParametersToXml(XmlElement* xml)
{

    xml->setAttribute("name", electrode->name);
    xml->setAttribute("stream_name", electrode->streamName);
    xml->setAttribute("source_node_id", electrode->sourceNodeId);

    xml->setAttribute("selectedUnit", selectedUnit);
    xml->setAttribute("selectedBox", selectedBox);

    XmlElement* pcaNode = xml->createNewChildElement("PCA");
    pcaNode->setAttribute("numChannels", numChannels);
    pcaNode->setAttribute("waveformLength", waveformLength);
    pcaNode->setAttribute("pc1min", pc1min);
    pcaNode->setAttribute("pc2min", pc2min);
    pcaNode->setAttribute("pc1max", pc1max);
    pcaNode->setAttribute("pc2max", pc2max);

    for (int k = 0; k < numChannels * waveformLength; k++)
    {
        XmlElement* dimNode = pcaNode->createNewChildElement("PCA_DIM");
        dimNode->setAttribute("pc1", pc1[k]);
        dimNode->setAttribute("pc2", pc2[k]);
    }

    for (int pcaUnitIter = 0; pcaUnitIter < pcaUnits.size(); pcaUnitIter++)
    {
        XmlElement* PcaUnitNode = pcaNode->createNewChildElement("UNIT");

        PcaUnitNode->setAttribute("UnitID", pcaUnits[pcaUnitIter].unitId);
        PcaUnitNode->setAttribute("ColorR", pcaUnits[pcaUnitIter].colorRGB[0]);
        PcaUnitNode->setAttribute("ColorG", pcaUnits[pcaUnitIter].colorRGB[1]);
        PcaUnitNode->setAttribute("ColorB", pcaUnits[pcaUnitIter].colorRGB[2]);
        PcaUnitNode->setAttribute("PolygonNumPoints", (int)pcaUnits[pcaUnitIter].poly.pts.size());
        PcaUnitNode->setAttribute("PolygonOffsetX", (int)pcaUnits[pcaUnitIter].poly.offset.X);
        PcaUnitNode->setAttribute("PolygonOffsetY", (int)pcaUnits[pcaUnitIter].poly.offset.Y);

        for (int p = 0; p < pcaUnits[pcaUnitIter].poly.pts.size(); p++)
        {
            XmlElement* PolygonNode = PcaUnitNode->createNewChildElement("POLYGON_POINT");
            PolygonNode->setAttribute("pointX", pcaUnits[pcaUnitIter].poly.pts[p].X);
            PolygonNode->setAttribute("pointY", pcaUnits[pcaUnitIter].poly.pts[p].Y);
        }
    }

    XmlElement* boxNode = xml->createNewChildElement("BOXES");

    for (auto unit : boxUnits)
    {
        XmlElement* boxUnitNode = boxNode->createNewChildElement("UNIT");

        boxUnitNode->setAttribute("UnitID", unit.unitId);
        boxUnitNode->setAttribute("ColorR", unit.colorRGB[0]);
        boxUnitNode->setAttribute("ColorG", unit.colorRGB[1]);
        boxUnitNode->setAttribute("ColorB", unit.colorRGB[2]);

        for (auto box : unit.lstBoxes)
        {
            XmlElement* boxNode = boxUnitNode->createNewChildElement("BOX");
            boxNode->setAttribute("ch", (int) box.channel);
            boxNode->setAttribute("x", (int) box.x);
            boxNode->setAttribute("y", (int) box.y);
            boxNode->setAttribute("w", (int) box.w);
            boxNode->setAttribute("h", (int) box.h);
        }
    }
}

void Sorter::loadCustomParametersFromXml(XmlElement* xml)
{

    selectedUnit = xml->getIntAttribute("selectedUnit", 0);
    selectedBox = xml->getIntAttribute("selectedBox", 0);

    forEachXmlChildElement(*xml, sorterNode)
    {
        if (sorterNode->hasTagName("PCA"))
        {

            numChannels = sorterNode->getIntAttribute("numChannels");
            waveformLength = sorterNode->getIntAttribute("waveformLength");

            pc1min = sorterNode->getDoubleAttribute("pc1min");
            pc2min = sorterNode->getDoubleAttribute("pc2min");
            pc1max = sorterNode->getDoubleAttribute("pc1max");
            pc2max = sorterNode->getDoubleAttribute("pc2max");

            delete[] pc1;
            delete[] pc2;

            pc1 = new float[waveformLength * numChannels];
            pc2 = new float[waveformLength * numChannels];
            int dimcounter = 0;

            forEachXmlChildElement(*sorterNode, dimNode)
            {
                if (dimNode->hasTagName("PCA_DIM"))
                {
                    pc1[dimcounter] = dimNode->getDoubleAttribute("pc1");
                    pc2[dimcounter] = dimNode->getDoubleAttribute("pc2");
                    dimcounter++;
                }
            }

            forEachXmlChildElement(*sorterNode, unitNode)
            {
                if (unitNode->hasTagName("UNIT"))
                {

                    LOGD(" Found a PCA unit ");

                    PCAUnit pcaUnit;

                    pcaUnit.unitId = unitNode->getIntAttribute("UnitID");

                    nextUnitId = jmax(pcaUnit.unitId + 1, nextUnitId);

                    pcaUnit.colorRGB[0] = unitNode->getIntAttribute("ColorR");
                    pcaUnit.colorRGB[1] = unitNode->getIntAttribute("ColorG");
                    pcaUnit.colorRGB[2] = unitNode->getIntAttribute("ColorB");

                    int numPolygonPoints = unitNode->getIntAttribute("PolygonNumPoints");
                    pcaUnit.poly.pts.resize(numPolygonPoints);
                    pcaUnit.poly.offset.X = unitNode->getDoubleAttribute("PolygonOffsetX");
                    pcaUnit.poly.offset.Y = unitNode->getDoubleAttribute("PolygonOffsetY");
                    
                    int pointCounter = 0;
                    forEachXmlChildElement(*unitNode, polygonPoint)
                    {
                        if (polygonPoint->hasTagName("POLYGON_POINT"))
                        {
                            pcaUnit.poly.pts[pointCounter].X = polygonPoint->getDoubleAttribute("pointX");
                            pcaUnit.poly.pts[pointCounter].Y = polygonPoint->getDoubleAttribute("pointY");
                            pointCounter++;
                        }
                    }

                    pcaUnits.push_back(pcaUnit);
                }
            }
        }
        else if (sorterNode->hasTagName("BOXES"))
        {
            forEachXmlChildElement(*sorterNode, unitNode)
            {
                if (unitNode->hasTagName("UNIT"))
                {

                    LOGD(" Found a box unit ");

                    BoxUnit boxUnit;
                    boxUnit.unitId = unitNode->getIntAttribute("UnitID");

                    nextUnitId = jmax(boxUnit.unitId + 1, nextUnitId);

                    boxUnit.colorRGB[0] = unitNode->getIntAttribute("ColorR");
                    boxUnit.colorRGB[1] = unitNode->getIntAttribute("ColorG");
                    boxUnit.colorRGB[2] = unitNode->getIntAttribute("ColorB");

                    forEachXmlChildElement(*unitNode, boxNode)
                    {
                        if (boxNode->hasTagName("BOX"))
                        {
                            Box box;
                            
                            box.channel = boxNode->getIntAttribute("ch");
                            box.x = boxNode->getDoubleAttribute("x");
                            box.y = boxNode->getDoubleAttribute("y");
                            box.w = boxNode->getDoubleAttribute("w");
                            box.h = boxNode->getDoubleAttribute("h");
                            
                            boxUnit.lstBoxes.push_back(box);
                        }
                    }

                    boxUnits.push_back(boxUnit);
                }
            }
        }
    }

    electrode->plot->updateUnits();

}
