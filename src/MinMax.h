/******************************************************
   This file is part of the BioData project
   (c) 2018 Erin Gee   http://www.eringee.net

   This file normalizes values using their minimum and maximum values.

   Contributing authors:
   (c) 2018 Erin Gee
   (c) 2018 Sofian Audry
   (c) 2017 Thomas Ouellet Fredericks
   (c) 2017 Martin Peach

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

******************************************************/

#ifndef MIN_MAX_H_
#define MIN_MAX_H_

class MinMax {
    float input;
    float min;
    float max;
    float value;
    bool firstPass;

  public:
    MinMax() {
      reset();
    }

    void reset() {
      input = min = max = value = 0;
      firstPass = true;
    }

    void adapt(float lop) {
      lop = constrain(lop, 0, 1);
      lop = lop * lop;

      min += (input - min) * lop;
      max += (input - max) * lop;
    }
    
    float filter(float f) {

      input = f;

      if ( firstPass ) {
        firstPass = false;
        min = f;
        max = f;
      } else {
        if ( f > max ) max = f;
        if ( f < min ) min = f;
      }

      if ( max == min ) {
        value = 0.5;
      } else {
        value = (f - min) / ( max - min);
      }

      return value;
    }

    float getMax() {
      return max;
    }

    float getMin() {
      return min;
    }


};

#endif
