/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
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

#include "interaction.hpp"

class ISquareBond: public Interaction
{
public:
  ISquareBond(DYNAMO::SimData*, double, double, double, C2Range*);

  ISquareBond(const magnet::xml::Node&, DYNAMO::SimData*);

  void operator<<(const magnet::xml::Node&);

  virtual Interaction* Clone() const;

  virtual double hardCoreDiam() const;

  virtual double maxIntDist() const;

  virtual void rescaleLengths(double);

  virtual double getCaptureEnergy() const;

  virtual void initialise(size_t);

  virtual bool captureTest(const Particle&, const Particle&) const;

  virtual void checkOverlaps(const Particle&, const Particle&) const;

  virtual IntEvent getEvent(const Particle&, const Particle&) const;
  
  virtual void runEvent(const Particle&, const Particle&, const IntEvent&) const;
    
  virtual void outputXML(xml::XmlStream&) const;

  virtual double getInternalEnergy() const { return 0.0; }

  virtual void 
  write_povray_info(std::ostream&) const;

protected:

  double diameter,d2;
  double lambda, ld2;
  double elasticity;
};
