/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "location.h"
#include "types.h"

namespace Ms {
class EngravingItem;
class Score;
class EngravingObject;
class XmlReader;
class XmlWriter;

//---------------------------------------------------------
//   @@ ConnectorInfo
///    Stores a general information on various connecting
///    elements (currently only spanners) including their
///    endpoints locations.
///    Base class of helper classes used to read and write
///    such elements.
//---------------------------------------------------------

class ConnectorInfo
{
    const EngravingItem* _current    { 0 };
    bool _currentUpdated       { false };
    const Score* _score;

    bool finishedLeft() const;
    bool finishedRight() const;

    static int orderedConnectionDistance(const ConnectorInfo& c1, const ConnectorInfo& c2);

protected:
    ElementType _type       { ElementType::INVALID };
    Location _currentLoc;
    Location _prevLoc       { Location::absolute() };
    Location _nextLoc       { Location::absolute() };

    ConnectorInfo* _prev    { 0 };
    ConnectorInfo* _next    { 0 };

    void updateLocation(const EngravingItem* e, Location& i, bool clipboardmode);
    void updateCurrentInfo(bool clipboardmode);
    bool currentUpdated() const { return _currentUpdated; }
    void setCurrentUpdated(bool v) { _currentUpdated = v; }

    ConnectorInfo* findFirst();
    const ConnectorInfo* findFirst() const;
    ConnectorInfo* findLast();
    const ConnectorInfo* findLast() const;

public:
    ConnectorInfo(const EngravingItem* current, int track = -1, Fraction = { -1, 1 });
    ConnectorInfo(const Score* score, const Location& currentLocation);

    ConnectorInfo* prev() const { return _prev; }
    ConnectorInfo* next() const { return _next; }
    ConnectorInfo* start();
    ConnectorInfo* end();

    ElementType type() const { return _type; }
    const Location& location() const { return _currentLoc; }

    bool connect(ConnectorInfo* other);
    bool finished() const;

    // for reconnection of broken connectors
    int connectionDistance(const ConnectorInfo& c2) const;
    void forceConnect(ConnectorInfo* c2);

    bool hasPrevious() const { return _prevLoc.measure() != INT_MIN; }
    bool hasNext() const { return _nextLoc.measure() != INT_MIN; }
    bool isStart() const { return !hasPrevious() && hasNext(); }
    bool isMiddle() const { return hasPrevious() && hasNext(); }
    bool isEnd() const { return hasPrevious() && !hasNext(); }
};

//---------------------------------------------------------
//   @@ ConnectorInfoReader
///    Helper class for reading beams, tuplets and spanners.
//---------------------------------------------------------

class ConnectorInfoReader final : public ConnectorInfo
{
    XmlReader* _reader;
    EngravingItem* _connector;
    EngravingObject* _connectorReceiver;

    void readEndpointLocation(Location& l);

public:
    ConnectorInfoReader(XmlReader& e, EngravingItem* current, int track = -1);
    ConnectorInfoReader(XmlReader& e, Score* current, int track = -1);

    ConnectorInfoReader* prev() const { return static_cast<ConnectorInfoReader*>(_prev); }
    ConnectorInfoReader* next() const { return static_cast<ConnectorInfoReader*>(_next); }

    EngravingItem* connector();
    const EngravingItem* connector() const;
    EngravingItem* releaseConnector();   // returns connector and "forgets" it by
    // setting an internal pointer to it to zero

    bool read();
    void update();
    void addToScore(bool pasteMode);

    static void readConnector(std::unique_ptr<ConnectorInfoReader> info, XmlReader& e);
};

//---------------------------------------------------------
//   @@ ConnectorInfoWriter
///    Helper class for writing connecting elements.
///    Subclasses should fill _prevInfo and _nextInfo with
///    the proper information on the connector's endpoints.
//---------------------------------------------------------

class ConnectorInfoWriter : public ConnectorInfo
{
    XmlWriter* _xml;

protected:
    const EngravingItem* _connector;

    virtual const char* tagName() const = 0;

public:
    ConnectorInfoWriter(XmlWriter& xml, const EngravingItem* current, const EngravingItem* connector, int track = -1, Fraction = { -1, 1 });

    ConnectorInfoWriter* prev() const { return static_cast<ConnectorInfoWriter*>(_prev); }
    ConnectorInfoWriter* next() const { return static_cast<ConnectorInfoWriter*>(_next); }

    const EngravingItem* connector() const { return _connector; }

    void write();
};
}     // namespace Ms
#endif
