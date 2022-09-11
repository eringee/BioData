/******************************************************

 * This file is part of the BioData project
 * (c) 2018 Erin Gee   http://www.eringee.net
 *
 * It's a high-pass filter.
 *
 * Contributing authors:
 * (c) 2018 Erin Gee
 * (c) 2018 Sofian Audry
 * (c) 2017 Thomas Ouellet Fredericks
 * (c) 2017 Martin Peach
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


class Hip {
  float value;
  float previousInput;
  float hip;

 public:
  Hip(float hip ) {
    this->hip = hip;
    };
    
  float filter( float input) {
    value =  hip * ( value + input - previousInput);
    previousInput = input;
    return value;
  }
};

