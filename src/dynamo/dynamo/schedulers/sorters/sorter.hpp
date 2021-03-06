/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <dynamo/schedulers/sorters/event.hpp>
#include <dynamo/base.hpp>
#include <dynamo/eventtypes.hpp>

namespace magnet { namespace xml { class Node; } } 
namespace xml { class XmlStream; } 

namespace dynamo {
  /*! \brief Future Event Lists (FEL) sort the Particle Event Lists
      (PEL) to determine the next event to occur.

      Classes Derived from this base class provide a mechanism to sort
      \ref Event s. These events are first pre-sorted using a Particle
      Event List before being sorted by these classes.
   */
  class FEL
  {
  public:
    virtual ~FEL() {}
    virtual void   resize(const size_t&) = 0;
    virtual void   clear() = 0;
    virtual void   init() = 0;
    virtual bool   empty() const = 0;
    virtual void   rebuild() = 0;
    virtual void   stream(const double&) = 0;
    virtual void   push(const Event&, const size_t&) = 0;
    virtual void   update(const size_t&) = 0;
    virtual std::pair<size_t, Event> next() const = 0;
    virtual void   sort() = 0;
    virtual void   rescaleTimes(const double&) = 0;
    virtual void   clearPEL(const size_t&) = 0;
    virtual void   popNextPELEvent(const size_t&) = 0;
    virtual void   popNextEvent() = 0;

    static shared_ptr<FEL> getClass(const magnet::xml::Node&);

    friend magnet::xml::XmlStream& operator<<(magnet::xml::XmlStream&, const FEL&);

  private:
    virtual void outputXML(magnet::xml::XmlStream&) const = 0;
  
  };
}
