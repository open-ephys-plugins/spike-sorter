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

#include "WaveformAxes.h"

#include "SpikeSorter.h"
#include "SpikePlot.h"
#include "BoxUnit.h"

WaveformAxes::WaveformAxes(SpikePlot* plot_, Electrode* electrode_, int channelIndex) : 
    GenericDrawAxes(GenericDrawAxes::AxesType(channelIndex)),
    channel(channelIndex),
    plot(plot_),
    electrode(electrode_),
    drawGrid(true),
    displayThresholdLevel(0.0f),
    spikesReceivedSinceLastRedraw(0),
    spikeIndex(0),
    bufferSize(5),
    range(250.0f),
    isOverThresholdSlider(false),
    isDraggingThresholdSlider(false)

{
    bDragging = false;

    isOverUnit = -1;
    isOverBox = -1;

    addMouseListener(this, true);

    thresholdColour = Colours::red;

    font = Font("Small Text", 10, Font::plain);
    
    int numSamples = 40;

    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }
}

void WaveformAxes::setRange(float r)
{
    range = r;

    repaint();
}

void WaveformAxes::plotSpike(SorterSpikePtr s, Graphics& g)
{
    if (s.get() == nullptr) return;
    float h = getHeight();

    g.setColour(Colour(s->color[0], s->color[1], s->color[2]));

    //compute the spatial width for each waveform sample
    float dx = getWidth() / float(s->getChannel()->getTotalSamples());

    int spikeSamples = s->getChannel()->getTotalSamples();

    // type corresponds to channel so we need to calculate the starting
    // sample based upon which channel is getting plotted
    int offset = channel * spikeSamples;

    float x = 0.0f;

    for (int i = 0; i < spikeSamples - 1; i++)
    {
        float s1 = h - (h / 2 + s->getData()[offset + i] / (range)*h);
        float s2 = h - (h / 2 + s->getData()[offset + i + 1] / (range)*h);

        if (signalFlipped)
        {
            s1 = h - s1;
            s2 = h - s2;
        }
        g.drawLine(x,
            s1,
            x + dx,
            s2);

        x += dx;
    }

}

void WaveformAxes::drawThresholdSlider(Graphics& g)
{

    g.setColour(thresholdColour);

    if (signalFlipped)
    {
        float h = getHeight() - (getHeight() * (0.5f - displayThresholdLevel / range));
        g.drawLine(0, h, getWidth(), h);
        g.drawText(String(roundFloatToInt(displayThresholdLevel)), 2, h, 35, 10, Justification::left, false);
    }
    else
    {
        float h = getHeight() * (0.5f - displayThresholdLevel / range);
        g.drawLine(0, h, getWidth(), h);
        g.drawText(String(roundFloatToInt(displayThresholdLevel)), 2, h, 35, 10, Justification::left, false);
    }

}


void WaveformAxes::drawWaveformGrid(Graphics& g)
{

    float h = getHeight();
    float w = getWidth();

    g.setColour(Colours::darkgrey);

    for (float y = -range / 2; y < range / 2; y += 25.0f)
    {
        if (y == 0)
            g.drawLine(0, h / 2 + y / range * h, w, h / 2 + y / range * h, 2.0f);
        else
            g.drawLine(0, h / 2 + y / range * h, w, h / 2 + y / range * h);
    }

}


bool WaveformAxes::updateSpikeData(SorterSpikePtr s)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    if (spikesReceivedSinceLastRedraw < bufferSize)
    {

        spikeIndex++;
        spikeIndex %= bufferSize;

        spikeBuffer.set(spikeIndex, s);

        spikesReceivedSinceLastRedraw++;

    }

    return true;

}


void WaveformAxes::clear()
{
    spikeBuffer.clear();
    spikeIndex = 0;
    int numSamples = 40;
    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }

    repaint();
}

void WaveformAxes::mouseMove(const MouseEvent& event)
{

    float y = event.y;

    float h = getHeight() * (0.5f - displayThresholdLevel / range);

    if (signalFlipped)
    {
        h = getHeight() - h;
    }

    if (y > h - 10.0f && y < h + 10.0f && !isOverThresholdSlider)
    {
        thresholdColour = Colours::yellow;
        isOverThresholdSlider = true;
    }
    else if ((y < h - 10.0f || y > h + 10.0f) && isOverThresholdSlider)
    {
        thresholdColour = Colours::red;
        isOverThresholdSlider = false;
    }
    else
    {
        // are we inside a box ?
        isOverUnit = 0;
        isOverBox = -1;
        strOverWhere = "";
        isOverUnitBox(event.x, event.y, isOverUnit, isOverBox, strOverWhere);

    }
    repaint();

}

int WaveformAxes::findUnitIndexById(int id)
{
    for (int k = 0; k < units.size(); k++)
        if (units[k].unitId == id)
            return k;
    return -1;
}

void WaveformAxes::mouseDown(const juce::MouseEvent& event)
{

    if (event.mods.isRightButtonDown())
    {
        clear();
    }

    float h = getHeight();
    float w = getWidth();
    float microsec_span = 40.0 / 30000.0 * 1e6;
    float microvolt_span = range / 2;
    
    mouseDownX = event.x / w * microsec_span;
    
    if (signalFlipped)
        mouseDownY = (h / 2 - (h - event.y)) / (h / 2) * microvolt_span;
    else
        mouseDownY = (h / 2 - event.y) / (h / 2) * microvolt_span;

    if (isOverUnit > 0)
    {
        electrode->sorter->setSelectedUnitAndBox(isOverUnit, isOverBox);
        int indx = findUnitIndexById(isOverUnit);
        jassert(indx >= 0);
        mouseOffsetX = mouseDownX - units[indx].lstBoxes[isOverBox].x;
        mouseOffsetY = mouseDownY - units[indx].lstBoxes[isOverBox].y;
    }
    else
    {
        electrode->sorter->setSelectedUnitAndBox(-1, -1);

    }
}


void WaveformAxes::mouseUp(const MouseEvent& event)
{
    if (bDragging)
    {
        bDragging = false;
        electrode->sorter->updateBoxUnits(units);
    }
}

void WaveformAxes::mouseDrag(const MouseEvent& event)
{
    bDragging = true;

    if (isOverUnit > 0)
    {
        // dragging a box
        // convert position to metric coordinates.
        float h = getHeight();
        float w = getWidth();
        float microsec_span = 40.0 / 30000.0 * 1e6;
        float microvolt_span = range / 2;
        float x = event.x / w * microsec_span;

        float y;
        if (signalFlipped)
            y = (h / 2 - (h - event.y)) / (h / 2) * microvolt_span;
        else
            y = (h / 2 - event.y) / (h / 2) * microvolt_span;

        // update units position....

        for (int k = 0; k < units.size(); k++)
        {
            if (units[k].getUnitId() == isOverUnit)
            {
                float oldx = units[k].lstBoxes[isOverBox].x;
                float oldy = units[k].lstBoxes[isOverBox].y;

                float dx = x - oldx;
                float dy = y - oldy;

                if (strOverWhere == "right")
                {
                    units[k].lstBoxes[isOverBox].w = x - oldx;
                }
                else if (strOverWhere == "left")
                {
                    units[k].lstBoxes[isOverBox].w += -dx;
                    units[k].lstBoxes[isOverBox].x = x;
                }
                else if ((!signalFlipped && strOverWhere == "top") || (signalFlipped && strOverWhere == "bottom"))
                {
                    units[k].lstBoxes[isOverBox].y += dy;
                    units[k].lstBoxes[isOverBox].h += dy;
                }
                else if ((!signalFlipped && strOverWhere == "bottom") || (signalFlipped && strOverWhere == "top"))
                {
                    units[k].lstBoxes[isOverBox].h = -dy;
                }
                else if ((!signalFlipped && strOverWhere == "bottomright") || (signalFlipped && strOverWhere == "topright"))
                {
                    units[k].lstBoxes[isOverBox].w = x - oldx;
                    units[k].lstBoxes[isOverBox].h = -dy;

                }
                else if ((!signalFlipped && strOverWhere == "bottomleft") || (signalFlipped && strOverWhere == "topleft"))
                {
                    units[k].lstBoxes[isOverBox].w += -dx;
                    units[k].lstBoxes[isOverBox].x = x;
                    units[k].lstBoxes[isOverBox].h = -dy;
                }
                else if ((!signalFlipped && strOverWhere == "topright") || (signalFlipped && strOverWhere == "bottomright"))
                {
                    units[k].lstBoxes[isOverBox].y += dy;
                    units[k].lstBoxes[isOverBox].h += dy;
                    units[k].lstBoxes[isOverBox].w = x - oldx;

                }
                else if ((!signalFlipped && strOverWhere == "topleft") || (signalFlipped && strOverWhere == "bottomleft"))
                {
                    units[k].lstBoxes[isOverBox].w += -dx;
                    units[k].lstBoxes[isOverBox].x = x;
                    units[k].lstBoxes[isOverBox].y += dy;
                    units[k].lstBoxes[isOverBox].h += dy;

                }
                else if (strOverWhere == "inside")
                {
                    units[k].lstBoxes[isOverBox].x = x - mouseOffsetX;
                    units[k].lstBoxes[isOverBox].y = y - mouseOffsetY;
                }

                if (units[k].lstBoxes[isOverBox].h < 0)
                {
                    units[k].lstBoxes[isOverBox].y -= units[k].lstBoxes[isOverBox].h;
                    units[k].lstBoxes[isOverBox].h *= -1;
                    if (strOverWhere == "top")
                        strOverWhere = "bottom";
                    else if (strOverWhere == "bottom")
                        strOverWhere = "top";
                    else if (strOverWhere == "topleft")
                        strOverWhere = "bottomleft";
                    else if (strOverWhere == "topright")
                        strOverWhere = "bottomright";
                    else if (strOverWhere == "bottomleft")
                        strOverWhere = "topleft";
                    else if (strOverWhere == "bottomright")
                        strOverWhere = "topright";
                }
                if (units[k].lstBoxes[isOverBox].w < 0)
                {
                    units[k].lstBoxes[isOverBox].x += units[k].lstBoxes[isOverBox].w;
                    units[k].lstBoxes[isOverBox].w *= -1;
                    if (strOverWhere == "left")
                        strOverWhere = "right";
                    else if (strOverWhere == "right")
                        strOverWhere = "left";
                    else if (strOverWhere == "topleft")
                        strOverWhere = "topright";
                    else if (strOverWhere == "topright")
                        strOverWhere = "topleft";
                    else if (strOverWhere == "bottomleft")
                        strOverWhere = "bottomright";
                    else if (strOverWhere == "bottomright")
                        strOverWhere = "bottomleft";
                }

            }

        }

    }
    else  if (isOverThresholdSlider)
    {

        float thresholdSliderPosition;
        if (signalFlipped)
            thresholdSliderPosition = (getHeight() - float(event.y)) / float(getHeight());
        else
            thresholdSliderPosition = float(event.y) / float(getHeight());

        if (thresholdSliderPosition > 1)
            thresholdSliderPosition = 1;
        else if (thresholdSliderPosition < -1) // Modified to allow negative thresholds.
            thresholdSliderPosition = -1;


        displayThresholdLevel = (0.5f - thresholdSliderPosition) * range;

        plot->setDisplayThresholdForChannel(channel, displayThresholdLevel);

    }

    repaint();

}


void WaveformAxes::mouseExit(const MouseEvent& event)
{
    if (isOverThresholdSlider)
    {
        isOverThresholdSlider = false;
        thresholdColour = Colours::red;
        repaint();
    }
}

float WaveformAxes::getDisplayThreshold()
{
    return displayThresholdLevel;
}

void WaveformAxes::setDetectorThreshold(float t)
{
    displayThresholdLevel = t;
}



void WaveformAxes::isOverUnitBox(float x, float y, int& UnitID, int& BoxID, String& where)
{

    float h = getHeight();
    float w = getWidth();
    // Map box coordinates to screen coordinates.
    // Assume time span is 40 samples at 30 Khz?
    float microsec_span = 40.0 / 30000.0 * 1e6;
    float microvolt_span = range / 2;

    // Typical spike is 40 samples, at 30kHz ~ 1.3 ms or 1300 usecs.
    for (int k = 0; k < units.size(); k++)
    {
        for (int boxiter = 0; boxiter < units[k].lstBoxes.size(); boxiter++)
        {
            Box B = units[k].lstBoxes[boxiter];
            float rectx1 = B.x / microsec_span * w;
            float recty1 = h / 2 - (B.y / microvolt_span * h / 2);
            float rectx2 = (B.x + B.w) / microsec_span * w;
            float recty2 = h / 2 - ((B.y - B.h) / microvolt_span * h / 2);

            if (signalFlipped)
            {
                recty1 = h - recty1;
                recty2 = h - recty2;
            }

            if (rectx1 > rectx2)
                std::swap(rectx1, rectx2);
            if (recty1 > recty2)
                std::swap(recty1, recty2);

            if (x >= rectx1 - 10 & y >= recty1 - 10 & x <= rectx2 + 10 & y <= recty2 + 10)
            {

                UnitID = units[k].unitId;
                BoxID = boxiter;
                if (x >= rectx1 - 10 & x <= rectx1 + 10 && y >= recty1 - 10 & y <= recty1 + 10)
                {
                    where = "topleft";
                    setMouseCursor(MouseCursor::TopLeftCornerResizeCursor);
                }
                else if (x >= rectx2 - 10 & x <= rectx2 + 10 && y >= recty1 - 10 & y <= recty1 + 10)
                {
                    where = "topright";
                    setMouseCursor(MouseCursor::TopRightCornerResizeCursor);
                }
                else if (x >= rectx1 - 10 & x <= rectx1 + 10 && y >= recty2 - 10 & y <= recty2 + 10)
                {
                    where = "bottomleft";
                    setMouseCursor(MouseCursor::BottomLeftCornerResizeCursor);
                }
                else if (x >= rectx2 - 10 & x <= rectx2 + 10 && y >= recty2 - 10 & y <= recty2 + 10)
                {
                    where = "bottomright";
                    setMouseCursor(MouseCursor::BottomRightCornerResizeCursor);
                }
                else if (x >= rectx1 - 10 & x <= rectx1 + 10)
                {
                    where = "left";
                    setMouseCursor(MouseCursor::LeftEdgeResizeCursor);
                }
                else if (x >= rectx2 - 10 & x <= rectx2 + 10)
                {
                    where = "right";
                    setMouseCursor(MouseCursor::RightEdgeResizeCursor);
                }
                else if (y >= recty1 - 10 & y <= recty1 + 10)
                {
                    setMouseCursor(MouseCursor::TopEdgeResizeCursor);
                    where = "top";
                }
                else if (y >= recty2 - 10 & y <= recty2 + 10)
                {
                    where = "bottom";
                    setMouseCursor(MouseCursor::BottomEdgeResizeCursor);
                }
                else
                {
                    setMouseCursor(MouseCursor::DraggingHandCursor);
                    where = "inside";
                }
                return;
            }
        }
    }

    setMouseCursor(MouseCursor::NormalCursor); // not inside any boxes
}

void WaveformAxes::drawBoxes(Graphics& g)
{
    // y and h are given in micro volts.
    // x and w and given in micro seconds.

    float h = getHeight();
    float w = getWidth();
    
    // Map box coordinates to screen coordinates.
    // Assume time span is 40 samples at 30 Khz?
    float microsec_span = 40.0 / 30000.0 * 1e6;
    float microvolt_span = range / 2;

    int selectedUnitID, selectedBoxID;
    electrode->sorter->getSelectedUnitAndBox(selectedUnitID, selectedBoxID);

    // Typical spike is 40 samples, at 30kHz ~ 1.3 ms or 1300 usecs.
    for (int k = 0; k < units.size(); k++)
    {
        g.setColour(Colour(units[k].colorRGB[0], units[k].colorRGB[1], units[k].colorRGB[2]));

        for (int boxiter = 0; boxiter < units[k].lstBoxes.size(); boxiter++)
        {
            Box B = units[k].lstBoxes[boxiter];

            float thickness = 2;
            if (units[k].getUnitId() == selectedUnitID && boxiter == selectedBoxID)
                thickness = 3;
            else if (units[k].getUnitId() == isOverUnit && boxiter == isOverBox)
                thickness = 2;
            else
                thickness = 1;

            float rectx1 = B.x / microsec_span * w;
            float recty1 = (h / 2 - (B.y / microvolt_span * h / 2));

            float rectx2 = (B.x + B.w) / microsec_span * w;
            float recty2 = (h / 2 - ((B.y - B.h) / microvolt_span * h / 2));

            float drawRecty1, drawRecty2;
            if (signalFlipped)
            {
                drawRecty2 = h - recty1;
                drawRecty1 = h - recty2;
            }
            else
            {
                drawRecty1 = recty1;
                drawRecty2 = recty2;
            }
            g.drawRect(rectx1, drawRecty1, rectx2 - rectx1, drawRecty2 - drawRecty1, thickness);
            g.drawText(String(units[k].unitId), rectx1, drawRecty1 - 15, rectx2 - rectx1, 15, juce::Justification::centred, false);

        }
    }
}



void WaveformAxes::updateUnits(std::vector<BoxUnit> _units)
{
    units = _units;
}

void WaveformAxes::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.fillRect(0, 0, getWidth(), getHeight());

    if (drawGrid)
        drawWaveformGrid(g);

    // draw the threshold line and labels
    drawThresholdSlider(g);
    drawBoxes(g);

    // if no spikes have been received then don't plot anything
    if (!gotFirstSpike)
    {
        return;
    }

    for (int spikeNum = 0; spikeNum < bufferSize; spikeNum++)
    {

        if (spikeNum != spikeIndex && spikeBuffer[spikeNum] != nullptr)
        {
            g.setColour(Colours::grey);
            plotSpike(spikeBuffer[spikeNum], g);
        }

    }

    g.setColour(Colours::white);
    if (spikeBuffer[spikeIndex] != nullptr)
        plotSpike(spikeBuffer[spikeIndex], g);

    spikesReceivedSinceLastRedraw = 0;

}
