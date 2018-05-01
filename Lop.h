/* This file is part of the BioData project
* (c) 2018 Erin Gee   http://www.eringee.net
*
* It's a low-pass filter.
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

#ifndef LOP_H_
#define LOP_H_

class Lop {

  // Low-pass smoothing factor.
  float alpha;

  // Current value.
  float value;

  // N. samples seen thus far.
  unsigned int n;

  // N. samples in calibration phase.
  unsigned int nCalibration;

public:

  /// Constructor.
  Lop(float alpha_=0.01) {
    setSmoothing(alpha_);
    reset();
  };

  /// Resets filter.
  void reset() {
    value = 0;
    n     = 0;
  }

  /// Sets smoothing factor to value in [0, 1] (lower value = smoother).
  void setSmoothing(float alpha_) {
    // Constrains the smoothing factor in [0, 1].
    alpha = constrain(alpha_, 0, 1);

    // Rule of thumb that maps the smoothing factor to number of samples.
    nCalibration = int(2 / alpha - 1);
  }

  /// Filters sample and returns smoothed value.
  float filter(float input) {
    // For the first #nCalibration# samples just compute the average.
    if (n < nCalibration) {
      n++;
      value = (value * (n-1) + input) / n;
    }
    // After that: switch back to exponential moving average.
    else {
      value += (input - value) * alpha;
    }
    return value;
  }

};

#endif
