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
#include "elastic.hpp"

/*! \brief For running a simulation with a time scale set by the shear
 * rate.
 *
 * Scientifically there is no reason for this class and a simulation
 * can be performed using hard sphere units; however, this class is
 * useful for debugging with a certain energy scale. The unit of this
 * scale can then be set to one by adjusting the time scale, which is
 * the function of this class.
 *
 * Although this class is called USquareWell, this class essentially
 * supports any system where there is an inherent energy scale you
 * want the simulation to run in. This usually means the unit energy
 * is set equal to 1.
 */
class UShear: public UHardSphere
{
public:
  static double ShearRate() { return  1; }

  UShear(const DYNAMO::SimData* tmp): 
    UHardSphere(1.0, tmp)
  {
    I_cout() << "Shearing units loaded";
  }
  
  UShear(const double& length, const DYNAMO::SimData* tmp):
    UHardSphere(length, tmp)
  {
    I_cout() << "Shearing units loaded";
  }
  
  UShear(const magnet::xml::Node& XML, const DYNAMO::SimData* tmp):
    UHardSphere(1.0, tmp)
  { 
    operator<<(XML); 
    I_cout() << "Shearing units loaded";
  }
  
  virtual ~UShear() {}
  
  virtual double unitTime() const
  { return 1.0 / ShearRate(); }
  
  virtual Units* Clone() const
  { return new UShear(*this); }
  
  void operator<<(const magnet::xml::Node&);

private:
  void outputXML(xml::XmlStream&) const;
};
