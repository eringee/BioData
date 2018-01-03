/******************************************************
copyright Erin Gee 2017

Authors Erin Gee // Martin Peach // Thomas Ouellet-Fredericks

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as published by
the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For more details: <http://www.gnu.org/licenses/>.

******************************************************/

class MinMax {
    float input= 0;
    float min = 0;
    float max = 0;
    float value = 0;
    bool reset = true;

  public:
    MinMax() {

    }


    void adapt(float lop) {
      lop = constrain(lop, 0, 1);
      lop = lop * lop;

      min = (input - min) * lop + min;
      max = (input - max) * lop + max;
    }

    float filter(float f) {

      input = f;
      
      if ( reset == true ) {
        reset = false;
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

