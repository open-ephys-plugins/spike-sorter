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
      computingThread(pcaThread_)
{

    pc1 = pc2 = nullptr;
    bufferSize = 200;
    spikeBufferIndex = -1;
    bPCAcomputed = false;
    bPCAJobSubmitted = false;
    bPCAjobFinished = false;
    selectedUnit = -1;
    selectedBox = -1;
    bRePCA = false;
    pc1min = -1;
    pc2min = -1;
    pc1max = 1;
    pc2max = 1;
    numChannels = electrode->numChannels;
    waveformLength = electrode->numSamples;

    pc1 = new float[numChannels * waveformLength];
    pc2 = new float[numChannels * waveformLength];

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
    pc1 = new float[numChannels * waveformLength];
    pc2 = new float[numChannels * waveformLength];
    spikeBuffer.clear();
    
    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }
    
    bPCAcomputed = false;
    spikeBufferIndex = -1;
	bPCAJobSubmitted = false;
	bPCAjobFinished = false;
	selectedUnit = -1;
	selectedBox = -1;
	bRePCA = false;
	pc1min = -1;
	pc2min = -1;
	pc1max = 1;
	pc2max = 1;

    for (int k=0; k<pcaUnits.size(); k++)
    {
        pcaUnits[k].resizeWaveform(waveformLength);
    }
    for (int k=0; k<boxUnits.size(); k++)
    {
        boxUnits[k].resizeWaveform(waveformLength);
    }

}

void Sorter::loadCustomParametersFromXml(XmlElement* electrodeNode)
{

    forEachXmlChildElement(*electrodeNode, spikesortNode)
    {
        if (spikesortNode->hasTagName("SPIKESORTING"))
        {
            //int numBoxUnit  = spikesortNode->getIntAttribute("numBoxUnits");
            //int numPCAUnit  = spikesortNode->getIntAttribute("numPCAUnits");
            selectedUnit  = spikesortNode->getIntAttribute("selectedUnit");
            selectedBox =  spikesortNode->getIntAttribute("selectedBox");

            pcaUnits.clear();
            boxUnits.clear();

            forEachXmlChildElement(*spikesortNode, UnitNode)
            {
                if (UnitNode->hasTagName("PCA"))
                {
                    numChannels = UnitNode->getIntAttribute("numChannels");
                    waveformLength = UnitNode->getIntAttribute("waveformLength");

                    pc1min = UnitNode->getDoubleAttribute("pc1min");
                    pc2min = UnitNode->getDoubleAttribute("pc2min");
                    pc1max = UnitNode->getDoubleAttribute("pc1max");
                    pc2max = UnitNode->getDoubleAttribute("pc2max");

                    bPCAjobFinished = UnitNode->getBoolAttribute("PCAjobFinished");
                    bPCAcomputed = UnitNode->getBoolAttribute("PCAcomputed");

                    delete[] pc1;
                    delete[] pc2;

                    pc1 = new float[waveformLength*numChannels];
                    pc2 = new float[waveformLength*numChannels];
                    int dimcounter = 0;
                    forEachXmlChildElement(*UnitNode, dimNode)
                    {
                        if (dimNode->hasTagName("PCA_DIM"))
                        {
                            pc1[dimcounter]=dimNode->getDoubleAttribute("pc1");
                            pc2[dimcounter]=dimNode->getDoubleAttribute("pc2");
                            dimcounter++;
                        }
                    }
                }

                if (UnitNode->hasTagName("BOXUNIT"))
                {
                    BoxUnit boxUnit;
                    boxUnit.UnitID = UnitNode->getIntAttribute("UnitID");
                    boxUnit.ColorRGB[0] = UnitNode->getIntAttribute("ColorR");
                    boxUnit.ColorRGB[1] = UnitNode->getIntAttribute("ColorG");
                    boxUnit.ColorRGB[2] = UnitNode->getIntAttribute("ColorB");
                    int numBoxes = UnitNode->getIntAttribute("NumBoxes");
                    boxUnit.lstBoxes.resize(numBoxes);
                    int boxCounter = 0;
                    forEachXmlChildElement(*UnitNode, boxNode)
                    {
                        if (boxNode->hasTagName("BOX"))
                        {
                            Box box;
                            box.channel = boxNode->getIntAttribute("ch");
                            box.x = boxNode->getDoubleAttribute("x");
                            box.y = boxNode->getDoubleAttribute("y");
                            box.w = boxNode->getDoubleAttribute("w");
                            box.h = boxNode->getDoubleAttribute("h");
                            boxUnit.lstBoxes[boxCounter++] = box;
                        }
                    }
                    // add box unit
                    boxUnits.push_back(boxUnit);
                }
                if (UnitNode->hasTagName("PCAUNIT"))
                {
                    PCAUnit pcaUnit;

                    pcaUnit.UnitID = UnitNode->getIntAttribute("UnitID");
                    pcaUnit.ColorRGB[0] = UnitNode->getIntAttribute("ColorR");
                    pcaUnit.ColorRGB[1] = UnitNode->getIntAttribute("ColorG");
                    pcaUnit.ColorRGB[2] = UnitNode->getIntAttribute("ColorB");

                    int numPolygonPoints = UnitNode->getIntAttribute("PolygonNumPoints");
                    pcaUnit.poly.pts.resize(numPolygonPoints);
                    pcaUnit.poly.offset.X = UnitNode->getDoubleAttribute("PolygonOffsetX");
                    pcaUnit.poly.offset.Y = UnitNode->getDoubleAttribute("PolygonOffsetY");
                    // read polygon
                    int pointCounter = 0;
                    forEachXmlChildElement(*UnitNode, polygonPoint)
                    {
                        if (polygonPoint->hasTagName("POLYGON_POINT"))
                        {
                            pcaUnit.poly.pts[pointCounter].X =  polygonPoint->getDoubleAttribute("pointX");
                            pcaUnit.poly.pts[pointCounter].Y =  polygonPoint->getDoubleAttribute("pointY");
                            pointCounter++;
                        }
                    }
                    // add polygon unit
                    pcaUnits.push_back(pcaUnit);
                }
            }
        }
    }
}

void Sorter::saveCustomParametersToXml(XmlElement* electrodeNode)
{

    XmlElement* spikesortNode = electrodeNode->createNewChildElement("SPIKESORTING");
    spikesortNode->setAttribute("numBoxUnits", (int)boxUnits.size());
    spikesortNode->setAttribute("numPCAUnits", (int)pcaUnits.size());
    spikesortNode->setAttribute("selectedUnit",selectedUnit);
    spikesortNode->setAttribute("selectedBox",selectedBox);


    XmlElement* pcaNode = electrodeNode->createNewChildElement("PCA");
    pcaNode->setAttribute("numChannels",numChannels);
    pcaNode->setAttribute("waveformLength",waveformLength);
    pcaNode->setAttribute("pc1min", pc1min);
    pcaNode->setAttribute("pc2min", pc2min);
    pcaNode->setAttribute("pc1max", pc1max);
    pcaNode->setAttribute("pc2max", pc2max);

    pcaNode->setAttribute("PCAjobFinished", bPCAjobFinished);
    pcaNode->setAttribute("PCAcomputed", bPCAcomputed);

    for (int k=0; k<numChannels*waveformLength; k++)
    {
        XmlElement* dimNode = pcaNode->createNewChildElement("PCA_DIM");
        dimNode->setAttribute("pc1",pc1[k]);
        dimNode->setAttribute("pc2",pc2[k]);
    }

    for (int boxUnitIter=0; boxUnitIter<boxUnits.size(); boxUnitIter++)
    {
        XmlElement* BoxUnitNode = spikesortNode->createNewChildElement("BOXUNIT");

        BoxUnitNode->setAttribute("UnitID",boxUnits[boxUnitIter].UnitID);
        BoxUnitNode->setAttribute("ColorR",boxUnits[boxUnitIter].ColorRGB[0]);
        BoxUnitNode->setAttribute("ColorG",boxUnits[boxUnitIter].ColorRGB[1]);
        BoxUnitNode->setAttribute("ColorB",boxUnits[boxUnitIter].ColorRGB[2]);
        BoxUnitNode->setAttribute("NumBoxes", (int)boxUnits[boxUnitIter].lstBoxes.size());
        for (int boxIter=0; boxIter<boxUnits[boxUnitIter].lstBoxes.size(); boxIter++)
        {
            XmlElement* BoxNode = BoxUnitNode->createNewChildElement("BOX");
            BoxNode->setAttribute("ch", (int)boxUnits[boxUnitIter].lstBoxes[boxIter].channel);
            BoxNode->setAttribute("x", (int)boxUnits[boxUnitIter].lstBoxes[boxIter].x);
            BoxNode->setAttribute("y", (int)boxUnits[boxUnitIter].lstBoxes[boxIter].y);
            BoxNode->setAttribute("w", (int)boxUnits[boxUnitIter].lstBoxes[boxIter].w);
            BoxNode->setAttribute("h", (int)boxUnits[boxUnitIter].lstBoxes[boxIter].h);
        }
    }

    for (int pcaUnitIter=0; pcaUnitIter<pcaUnits.size(); pcaUnitIter++)
    {
        XmlElement* PcaUnitNode = spikesortNode->createNewChildElement("PCAUNIT");

        PcaUnitNode->setAttribute("UnitID",pcaUnits[pcaUnitIter].UnitID);
        PcaUnitNode->setAttribute("ColorR",pcaUnits[pcaUnitIter].ColorRGB[0]);
        PcaUnitNode->setAttribute("ColorG",pcaUnits[pcaUnitIter].ColorRGB[1]);
        PcaUnitNode->setAttribute("ColorB",pcaUnits[pcaUnitIter].ColorRGB[2]);
        PcaUnitNode->setAttribute("PolygonNumPoints",(int)pcaUnits[pcaUnitIter].poly.pts.size());
        PcaUnitNode->setAttribute("PolygonOffsetX",(int)pcaUnits[pcaUnitIter].poly.offset.X);
        PcaUnitNode->setAttribute("PolygonOffsetY",(int)pcaUnits[pcaUnitIter].poly.offset.Y);

        for (int p=0; p<pcaUnits[pcaUnitIter].poly.pts.size(); p++)
        {
            XmlElement* PolygonNode = PcaUnitNode->createNewChildElement("POLYGON_POINT");
            PolygonNode->setAttribute("pointX", pcaUnits[pcaUnitIter].poly.pts[p].X);
            PolygonNode->setAttribute("pointY", pcaUnits[pcaUnitIter].poly.pts[p].Y);
        }
    }

}

Sorter::~Sorter()
{
    // wait until PCA job is done (if one was submitted).
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
    spikeBufferIndex++;
    spikeBufferIndex %= bufferSize;
    spikeBuffer.set(spikeBufferIndex, so);
    if (bPCAjobFinished)
    {
        bPCAcomputed = true;
    }

    if (bPCAcomputed)
    {
        so->pcProj[0] = so->pcProj[1] = 0;
        for (int k=0; k<so->getChannel()->getNumChannels()*so->getChannel()->getTotalSamples(); k++)
        {
            float v = so->spikeDataIndexToMicrovolts(k);
            so->pcProj[0] += pc1[k]* v;
            so->pcProj[1] += pc2[k]* v;
        }
        if (so->pcProj[0] > 1e5 || so->pcProj[0] < -1e5 || so->pcProj[1] > 1e5 || so->pcProj[1] < -1e5)
        {
            //int dbg = 1;
        }
    }
    else
    {
        // add a spike object to the buffer.
        // if we have enough spikes, start the PCA computation thread.
        if ((spikeBufferIndex == bufferSize -1 && !bPCAcomputed && !bPCAJobSubmitted) || bRePCA)
        {
            bPCAJobSubmitted = true;
	    bPCAcomputed = false;
            bRePCA = false;
            // submit a new job to compute the spike buffer.
            PCAJobPtr job = new PCAjob(spikeBuffer,pc1,pc2, pc1min, pc2min, pc1max, pc2max, bPCAjobFinished);
            computingThread->addPCAjob(job);
        }
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
    pc1min=p1min;
    pc2min=p2min;
    pc1max=p1max;
    pc2max=p2max;
}


void Sorter::resetJobStatus()
{
    bPCAjobFinished = false;
}

bool Sorter::isPCAfinished()
{
    return bPCAjobFinished;
}
void Sorter::RePCA()
{
    bPCAcomputed = false;
    bPCAJobSubmitted = false;
    bRePCA = true;
}

void Sorter::addPCAunit(PCAUnit unit)
{
    const ScopedLock myScopedLock(mut);
    //StartCriticalSection();
    pcaUnits.push_back(unit);
    //EndCriticalSection();
}

// Adds a new unit with a single box at some default location.
// returns the unit id
int Sorter::addBoxUnit(int channel)
{
    const ScopedLock myScopedLock(mut);

    BoxUnit unit(nextUnitId++, generateLocalID());
    boxUnits.push_back(unit);
    setSelectedUnitAndBox(nextUnitId, 0);

    return nextUnitId;
}

int Sorter::addBoxUnit(int channel, Box B)
{
    const ScopedLock myScopedLock(mut);

    BoxUnit unit(B, nextUnitId++, generateLocalID());
    boxUnits.push_back(unit);
    setSelectedUnitAndBox(nextUnitId, 0);

    return nextUnitId;
}

void Sorter::getUnitColor(int UnitID, uint8& R, uint8& G, uint8& B)
{
    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == UnitID)
        {
            R = boxUnits[k].ColorRGB[0];
            G = boxUnits[k].ColorRGB[1];
            B = boxUnits[k].ColorRGB[2];
            break;
        }
    }
    for (int k = 0; k < pcaUnits.size(); k++)
    {
        if (pcaUnits[k].getUnitID() == UnitID)
        {
            R = pcaUnits[k].ColorRGB[0];
            G = pcaUnits[k].ColorRGB[1];
            B = pcaUnits[k].ColorRGB[2];
            break;
        }
    }
}

int Sorter::generateLocalID()
{
    // finds the first unused ID and return it

    int ID = 1;

    while (true)
    {
        bool used=false;
        for (int k = 0; k < boxUnits.size(); k++)
        {
            if (boxUnits[k].getLocalID() == ID)
            {
                used = true;
                break;
            }
        }
        for (int k = 0; k < pcaUnits.size(); k++)
        {
            if (pcaUnits[k].getLocalID() == ID)
            {
                used = true;
                break;
            }
        }

        if (used)
            ID++;
        else
            break;
    }
    return ID;
}

int Sorter::generateUnitID()
{

    int ID = ++nextUnitId;
    return ID;
}


void Sorter::generateNewIDs()
{
    const ScopedLock myScopedLock(mut);
    for (int k=0; k<boxUnits.size(); k++)
    {
        boxUnits[k].UnitID = generateUnitID();
    }
    for (int k=0; k<pcaUnits.size(); k++)
    {
        pcaUnits[k].UnitID = generateUnitID();
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
    //StartCriticalSection();
    for (int k=0; k<boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == unitID)
        {
            boxUnits.erase(boxUnits.begin()+k);
            //EndCriticalSection();
            return true;
        }
    }

    for (int k=0; k<pcaUnits.size(); k++)
    {
        if (pcaUnits[k].getUnitID() == unitID)
        {
            pcaUnits.erase(pcaUnits.begin()+k);
            //EndCriticalSection();
            return true;
        }
    }

    // EndCriticalSection();
    return false;

}

bool Sorter::addBoxToUnit(int channel, int unitID)
{
    const ScopedLock myScopedLock(mut);

    //StartCriticalSection();

    for (int k = 0; k < boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == unitID)
        {
            Box B = boxUnits[k].lstBoxes[boxUnits[k].lstBoxes.size() - 1];
            B.x += 100;
            B.y -= 30;
            B.channel = channel;
            boxUnits[k].addBox(B);
            setSelectedUnitAndBox(unitID, (int) boxUnits[k].lstBoxes.size() - 1);
            // EndCriticalSection();
            return true;
        }
    }
    // EndCriticalSection();
    return false;
}


bool Sorter::addBoxToUnit(int channel, int unitID, Box B)
{
    const ScopedLock myScopedLock(mut);
    //StartCriticalSection();
    for (int k=0; k<boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == unitID)
        {
            boxUnits[k].addBox(B);
            // EndCriticalSection();
            return true;
        }
    }
    //EndCriticalSection();
    return false;
}

std::vector<BoxUnit> Sorter::getBoxUnits()
{
    //StartCriticalSection();
    const ScopedLock myScopedLock(mut);
    std::vector<BoxUnit> unitsCopy = boxUnits;
    //EndCriticalSection();
    return unitsCopy;
}


std::vector<PCAUnit> Sorter::getPCAUnits()
{
    //StartCriticalSection();
    const ScopedLock myScopedLock(mut);
    std::vector<PCAUnit> unitsCopy = pcaUnits;
    //EndCriticalSection();
    return unitsCopy;
}

void Sorter::updatePCAUnits(std::vector<PCAUnit> _units)
{
    //StartCriticalSection();
    const ScopedLock myScopedLock(mut);
    pcaUnits = _units;
    //EndCriticalSection();
}

void Sorter::updateBoxUnits(std::vector<BoxUnit> _units)
{
    const ScopedLock myScopedLock(mut);
    //StartCriticalSection();
    boxUnits = _units;
    //EndCriticalSection();
}




// tests whether a candidate spike belongs to one of the defined units
bool Sorter::sortSpike(SorterSpikePtr spike, bool PCAfirst)
{
    const ScopedLock myScopedLock(mut);
    if (PCAfirst)
    {

        for (int k=0; k<pcaUnits.size(); k++)
        {
            if (pcaUnits[k].isWaveFormInsidePolygon(spike))
            {
                spike->sortedId = pcaUnits[k].getUnitID();
                spike->color[0] = pcaUnits[k].ColorRGB[0];
                spike->color[1] = pcaUnits[k].ColorRGB[1];
                spike->color[2] = pcaUnits[k].ColorRGB[2];
                return true;
            }
        }

        for (int k=0; k<boxUnits.size(); k++)
        {
            if (boxUnits[k].isWaveFormInsideAllBoxes(spike))
            {
                spike->sortedId = boxUnits[k].getUnitID();
                spike->color[0] = boxUnits[k].ColorRGB[0];
                spike->color[1] = boxUnits[k].ColorRGB[1];
                spike->color[2] = boxUnits[k].ColorRGB[2];
                boxUnits[k].updateWaveform(spike);
                return true;
            }
        }
    }
    else
    {

        for (int k=0; k<boxUnits.size(); k++)
        {
            if (boxUnits[k].isWaveFormInsideAllBoxes(spike))
            {
                spike->sortedId = boxUnits[k].getUnitID();
                spike->color[0] = boxUnits[k].ColorRGB[0];
                spike->color[1] = boxUnits[k].ColorRGB[1];
                spike->color[2] = boxUnits[k].ColorRGB[2];
                boxUnits[k].updateWaveform(spike);
                return true;
            }
        }
        for (int k=0; k<pcaUnits.size(); k++)
        {
            if (pcaUnits[k].isWaveFormInsidePolygon(spike))
            {
                spike->sortedId = pcaUnits[k].getUnitID();
                spike->color[0] = pcaUnits[k].ColorRGB[0];
                spike->color[1] = pcaUnits[k].ColorRGB[1];
                spike->color[2] = pcaUnits[k].ColorRGB[2];
                pcaUnits[k].updateWaveform(spike);
                return true;
            }
        }

    }

    return false;
}


bool  Sorter::removeBoxFromUnit(int unitID, int boxIndex)
{
    const ScopedLock myScopedLock(mut);

    for (int k=0; k<boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == unitID)
        {
            bool s= boxUnits[k].deleteBox(boxIndex);
            setSelectedUnitAndBox(-1,-1);

            return s;
        }

    }

    return false;
}

std::vector<Box> Sorter::getUnitBoxes(int unitID)
{
    std::vector<Box> boxes;
    const ScopedLock myScopedLock(mut);

    for (int k=0; k< boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == unitID)
        {

            boxes = boxUnits[k].getBoxes();

            return boxes;
        }
    }

    return boxes;
}


int Sorter::getNumBoxes(int unitID)
{
    const ScopedLock myScopedLock(mut);
    // StartCriticalSection();
    for (int k=0; k< boxUnits.size(); k++)
    {
        if (boxUnits[k].getUnitID() == unitID)
        {

            int n =boxUnits[k].getNumBoxes();
            // EndCriticalSection();
            return n;
        }
    }
    //EndCriticalSection();
    return -1;
}
