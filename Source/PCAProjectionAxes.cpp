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


#include "PCAProjectionAxes.h"

#include "SpikeSorter.h"

PCAProjectionAxes::PCAProjectionAxes(Electrode* electrode_) :
    GenericDrawAxes(GenericDrawAxes::PCA),
    electrode(electrode_),
    imageDim(500),
    rangeX(250),
    rangeY(250),
    spikesReceivedSinceLastRedraw(0)
{
    projectionImage = Image(Image::RGB, imageDim, imageDim, true);
    bufferSize = 600;
    pcaMin[0] = pcaMin[1] = -5;
    pcaMax[0] = pcaMax[1] = 5;

    rangeSet = false;
    inPolygonDrawingMode = false;
    clear();
    updateProcessor = false;
    isOverUnit = -1;

    rangeUpButton = new UtilityButton("+", Font("Small Text", 10, Font::plain));
    rangeUpButton->setRadius(3.0f);
    rangeUpButton->addListener(this);
    rangeUpButton->setBounds(35, 10, 20, 15);
    addAndMakeVisible(rangeUpButton);

    rangeDownButton = new UtilityButton("-", Font("Small Text", 10, Font::plain));
    rangeDownButton->setRadius(3.0f);
    rangeDownButton->addListener(this);
    rangeDownButton->setBounds(10, 10, 20, 15);
    addAndMakeVisible(rangeDownButton);

    redrawSpikes = true;

}


void PCAProjectionAxes::setPolygonDrawingMode(bool on)
{
    if (on)
    {
        inPolygonDrawingMode = true;
        setMouseCursor(MouseCursor::CrosshairCursor);

    }
    else
    {
        inPolygonDrawingMode = false;
        setMouseCursor(MouseCursor::NormalCursor);
    }
}

void PCAProjectionAxes::updateUnits(std::vector<PCAUnit> _units)
{
    units = _units;
}

void PCAProjectionAxes::drawUnit(Graphics& g, PCAUnit unit)
{
    float w = getWidth();
    float h = getHeight();

    int selectedUnitId, selectedBoxId;

    electrode->sorter->getSelectedUnitAndBox(selectedUnitId, selectedBoxId);

    g.setColour(Colour(unit.colorRGB[0], unit.colorRGB[1], unit.colorRGB[2]));
    
    if (unit.poly.pts.size() > 2)
    {
        float thickness;
        if (unit.getUnitId() == selectedUnitId)
            thickness = 3;
        else if (unit.getUnitId() == isOverUnit)
            thickness = 2;
        else
            thickness = 1;

        double cx = 0, cy = 0;

        for (int k = 0; k < unit.poly.pts.size() - 1; k++)
        {
            // convert projection coordinates to screen coordinates.
            float x1 = (unit.poly.offset.X + unit.poly.pts[k].X - pcaMin[0]) / (pcaMax[0] - pcaMin[0]) * w;
            float y1 = (unit.poly.offset.Y + unit.poly.pts[k].Y - pcaMin[1]) / (pcaMax[1] - pcaMin[1]) * h;
            float x2 = (unit.poly.offset.X + unit.poly.pts[k + 1].X - pcaMin[0]) / (pcaMax[0] - pcaMin[0]) * w;
            float y2 = (unit.poly.offset.Y + unit.poly.pts[k + 1].Y - pcaMin[1]) / (pcaMax[1] - pcaMin[1]) * h;
            cx += x1;
            cy += y1;
            g.drawLine(x1, y1, x2, y2, thickness);
        }
        
        float x1 = (unit.poly.offset.X + unit.poly.pts[0].X - pcaMin[0]) / (pcaMax[0] - pcaMin[0]) * w;
        float y1 = (unit.poly.offset.Y + unit.poly.pts[0].Y - pcaMin[1]) / (pcaMax[1] - pcaMin[1]) * h;
        float x2 = (unit.poly.offset.X + unit.poly.pts[unit.poly.pts.size() - 1].X - pcaMin[0]) / (pcaMax[0] - pcaMin[0]) * w;
        float y2 = (unit.poly.offset.Y + unit.poly.pts[unit.poly.pts.size() - 1].Y - pcaMin[1]) / (pcaMax[1] - pcaMin[1]) * h;
        
        g.drawLine(x1, y1, x2, y2, thickness);

        cx += x2;
        cy += y2;

        g.drawText(String(unit.unitId), 
                   (cx / unit.poly.pts.size()) - 10, 
                   (cy / unit.poly.pts.size()) - 10, 
                   20, 15, juce::Justification::centred, 
                   false);
    }
}

void PCAProjectionAxes::paint(Graphics& g)
{

    spikesReceivedSinceLastRedraw = 0;

    g.drawImage(projectionImage,
        0, 0, getWidth(), getHeight(),
        0, 0, rangeX, rangeY);

    // draw pca units polygons
    for (int k = 0; k < units.size(); k++)
    {
        drawUnit(g, units[k]);
    }

    if (inPolygonDrawingMode)
    {
        setMouseCursor(MouseCursor::CrosshairCursor);
        // draw polygon
        bool first = true;
        PointD prev;

        if (drawnPolygon.size() > 0)
        {
            g.setColour(Colour(drawnUnit.colorRGB[0], drawnUnit.colorRGB[1], drawnUnit.colorRGB[2]));

            for (std::list<PointD>::iterator it = drawnPolygon.begin(); it != drawnPolygon.end(); it++)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    g.drawLine((*it).X, (*it).Y, prev.X, prev.Y);
                }
                prev = *it;
            }

            g.drawLine(drawnPolygon.front().X, drawnPolygon.front().Y, drawnPolygon.back().X, drawnPolygon.back().Y);
        }
    }

    if (redrawSpikes)
    {
        projectionImage.clear(juce::Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
            Colours::black);

        bool subsample = false;
        int dk = (subsample) ? 5 : 1;

        for (int k = 0; k < bufferSize; k += dk)
        {
            drawProjectedSpike(spikeBuffer[k]);
        }
        redrawSpikes = false;
    }

}


void PCAProjectionAxes::drawProjectedSpike(SorterSpikePtr s)
{
    if (s != nullptr && rangeSet)
    {
        Graphics g(projectionImage);

        g.setColour(Colour(s->color[0], s->color[1], s->color[2]));

        float x = (s->pcProj[0] - pcaMin[0]) / (pcaMax[0] - pcaMin[0]) * rangeX;
        float y = (s->pcProj[1] - pcaMin[1]) / (pcaMax[1] - pcaMin[1]) * rangeY;
        if (x >= 0 & y >= 0 & x <= rangeX & y <= rangeY)
            g.fillEllipse(x, y, 2, 2);
    }
}

void PCAProjectionAxes::redraw(bool subsample)
{
    Graphics g(projectionImage);

    projectionImage.clear(juce::Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
        Colours::black);

    int dk = (subsample) ? 5 : 1;

    for (int k = 0; k < bufferSize; k += dk)
    {
        drawProjectedSpike(spikeBuffer[k]);
    }

}

void PCAProjectionAxes::setPCARange(float p1min, float p2min, float p1max, float p2max)
{

    pcaMin[0] = p1min;
    pcaMin[1] = p2min;
    pcaMax[0] = p1max;
    pcaMax[1] = p2max;
    rangeSet = true;
    redrawSpikes = true;
    electrode->sorter->setPCArange(p1min, p2min, p1max, p2max);

}

bool PCAProjectionAxes::updateSpikeData(SorterSpikePtr s)
{

    if (spikesReceivedSinceLastRedraw < bufferSize)
    {

        spikeIndex++;
        spikeIndex %= bufferSize;

        spikeBuffer.set(spikeIndex, s);

        spikesReceivedSinceLastRedraw++;
        //drawProjectedSpike(newSpike);
        redrawSpikes = true;

    }
    return true;
}


void PCAProjectionAxes::clear()
{
    projectionImage.clear(juce::Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
        Colours::black);

    spikeBuffer.clear();
    spikeIndex = 0;

    redrawSpikes = true;
}


void PCAProjectionAxes::mouseDrag(const juce::MouseEvent& event)
{

    if (!inPolygonDrawingMode)
    {

        setMouseCursor(MouseCursor::DraggingHandCursor);
        int selectedUnitID, selectedBoxID;
        electrode->sorter->getSelectedUnitAndBox(selectedUnitID, selectedBoxID);

        if (isOverUnit > 0 && selectedUnitID == isOverUnit)
        {
            // pan unit
            int unitindex = -1;

            for (int k = 0; k < units.size(); k++)
            {
                if (units[k].getUnitId() == selectedUnitID)
                {
                    unitindex = k;
                    break;
                }
            }
            jassert(unitindex >= 0);

            int w = getWidth();
            int h = getHeight();
            float range0 = pcaMax[0] - pcaMin[0];
            float range1 = pcaMax[1] - pcaMin[1];

            float dx = float(event.x - prevx) / w * range0;
            float dy = float(event.y - prevy) / h * range1;


            units[unitindex].poly.offset.X += dx;
            units[unitindex].poly.offset.Y += dy;
            updateProcessor = true;
            // draw polygon
            prevx = event.x;
            prevy = event.y;

        }
        else
        {
            // Pan PCA space
            int w = getWidth();
            int h = getHeight();
            float range0 = pcaMax[0] - pcaMin[0];
            float range1 = pcaMax[1] - pcaMin[1];

            float dx = -float(event.x - prevx) / w * range0;
            float dy = -float(event.y - prevy) / h * range1;

            pcaMin[0] += dx;
            pcaMin[1] += dy;
            pcaMax[0] += dx;
            pcaMax[1] += dy;
            electrode->sorter->setPCArange(pcaMin[0], pcaMin[1], pcaMax[0], pcaMax[1]);

            // draw polygon
            prevx = event.x;
            prevy = event.y;

            redrawSpikes = true;
        }

    }
    else
    {
        int pixel_quantizer = 6;
        float distance = float(event.x - prevx) * float(event.x - prevx) +
            float(event.y - prevy) * float(event.y - prevy);
        if (distance > pixel_quantizer * pixel_quantizer)  // add a point every n pixels.
        {
            drawnPolygon.push_back(PointD(event.x, event.y));
            // draw polygon
            prevx = event.x;
            prevy = event.y;

            repaint();
        }
    }

}

void PCAProjectionAxes::mouseUp(const juce::MouseEvent& event)
{
    repaint();

    setMouseCursor(MouseCursor::NormalCursor);

    if (updateProcessor)
    {
        electrode->sorter->updatePCAUnits(units);
        updateProcessor = false;

    }

    if (inPolygonDrawingMode)
    {
        inPolygonDrawingMode = false;

        // convert pixel coordinates to pca space coordinates and update unit
        cPolygon poly;
        poly.pts.resize(drawnPolygon.size());
        int k = 0;

        float w = getWidth();
        float h = getHeight();
        float range0 = pcaMax[0] - pcaMin[0];
        float range1 = pcaMax[1] - pcaMin[1];

        for (std::list<PointD>::iterator it = drawnPolygon.begin(); it != drawnPolygon.end(); it++, k++)
        {
            poly.pts[k].X = (*it).X / w * range0 + pcaMin[0];
            poly.pts[k].Y = (*it).Y / h * range1 + pcaMin[1];
        }
        
        drawnUnit.poly = poly;
        units.push_back(drawnUnit);

        // add a new PCA unit
        electrode->sorter->addPCAunit(drawnUnit);

        uint8 r, g, b;
        electrode->sorter->getUnitColor(drawnUnit.getUnitId(), r, g, b);

        drawnPolygon.clear();
    }
}


void PCAProjectionAxes::mouseMove(const juce::MouseEvent& event)
{
    isOverUnit = -1;
    float w = getWidth();
    float h = getHeight();

    for (int k = 0; k < units.size(); k++)
    {
        // convert projection coordinates to screen coordinates.
        float x1 = ((float)event.x / w) * (pcaMax[0] - pcaMin[0]) + pcaMin[0];
        float y1 = ((float)event.y / h) * (pcaMax[1] - pcaMin[1]) + pcaMin[1];
        if (units[k].isPointInsidePolygon(PointD(x1, y1)))
        {
            isOverUnit = units[k].getUnitId();
            break;
        }

    }
}


void PCAProjectionAxes::mouseDown(const juce::MouseEvent& event)
{
    prevx = event.x;
    prevy = event.y;
    if (event.mods.isRightButtonDown())
    {
        clear();
    }
    if (inPolygonDrawingMode)
    {
        drawnUnit = PCAUnit(Sorter::generateUnitId());
        drawnPolygon.push_back(PointD(event.x, event.y));
    }
    else
    {
        if (isOverUnit > 0)
            electrode->sorter->setSelectedUnitAndBox(isOverUnit, -1);
        else
            electrode->sorter->setSelectedUnitAndBox(-1, -1);
    }
}


bool PCAProjectionAxes::keyPressed(const KeyPress& key)
{
    KeyPress e = KeyPress::createFromDescription("escape");

    if (key.isKeyCode(e.getKeyCode()) && inPolygonDrawingMode) // C
    {
        inPolygonDrawingMode = false;
        setMouseCursor(MouseCursor::NormalCursor);
        return true;
    }
    return false;
}

void PCAProjectionAxes::rangeDown()
{
    float range0 = pcaMax[0] - pcaMin[0];
    float range1 = pcaMax[1] - pcaMin[1];
    pcaMin[0] = pcaMin[0] - 0.1 * range0;
    pcaMax[0] = pcaMax[0] + 0.1 * range0;
    pcaMin[1] = pcaMin[1] - 0.1 * range1;
    pcaMax[1] = pcaMax[1] + 0.1 * range1;
    setPCARange(pcaMin[0], pcaMin[1], pcaMax[0], pcaMax[1]);
}

void PCAProjectionAxes::rangeUp()
{
    float range0 = pcaMax[0] - pcaMin[0];
    float range1 = pcaMax[1] - pcaMin[1];
    pcaMin[0] = pcaMin[0] + 0.1 * range0;
    pcaMax[0] = pcaMax[0] - 0.1 * range0;
    pcaMin[1] = pcaMin[1] + 0.1 * range1;
    pcaMax[1] = pcaMax[1] - 0.1 * range1;

    setPCARange(pcaMin[0], pcaMin[1], pcaMax[0], pcaMax[1]);

}

void PCAProjectionAxes::buttonClicked(Button* button)
{
    if (button == rangeDownButton)
    {
        rangeDown();
    }

    else if (button == rangeUpButton)
    {
        rangeUp();
    }

}

void PCAProjectionAxes::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
    if (wheel.deltaY > 0)
        rangeDown();
    else
        rangeUp();
}
