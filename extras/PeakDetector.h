/*
 * PeakDetector.h
 *
 * Adapté de : 
 * Plaquette (c) 2022 Sofian Audry        :: info(@)sofianaudry(.)com
 *
 * Adaptation par Luana Belinsky 2025
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

#ifndef PEAK_H_
#define PEAK_H_

#include <stdint.h>
#include <cfloat>

/// @brief Peak modes.
enum {
  PEAK_RISING,
  PEAK_FALLING,
  PEAK_MAX,
  PEAK_MIN
};

/**
 * Emits a signals when a signal peaks.
 */
class PeakDetector {
public:
  /**
   * Constructor. Possible modes are:
   * - PEAK_RISING  : peak detected when value becomes >= triggerThreshold, then wait until it becomes < reloadThreshold (*)
   * - PEAK_FALLING : peak detected when value becomes <= triggerThreshold, then wait until it becomes > reloadThreshold (*)
   * - PEAK_MAX     : peak detected after value becomes >= triggerThreshold and then falls back after peaking; then waits until it becomes < reloadThreshold (*)
   * - PEAK_MIN     : peak detected after value becomes <= triggerThreshold and then rises back after peaking; then waits until it becomes > reloadThreshold (*)
   * @param triggerThreshold value that triggers peak detection
   * @param mode peak detection mode
   */
  PeakDetector(float triggerThreshold, uint8_t mode=PEAK_MAX);
  virtual ~PeakDetector() {}

  /// Sets triggerThreshold.
  void triggerThreshold(float triggerThreshold);

  /// Returns triggerThreshold.
  float triggerThreshold() const { return _triggerThreshold; }

  /**
   * Sets minimal threshold that "resets" peak detection in crossing
   * (rising/falling) and peak (min/max) modes.
   */
  void reloadThreshold(float reloadThreshold);

  /// Returns minimal value "drop" for reset.
  float reloadThreshold() const { return _reloadThreshold; }

  /**
   * Sets minimal relative "drop" after peak to trigger detection in peak (min/max)
   * modes, expressed as proportion (%) of peak minus triggerThreshold.
   */
  void fallbackTolerance(float fallbackTolerance);

  /// Returns minimal relative "drop" after peak to trigger detection in peak modes.
  float fallbackTolerance() const { return _fallbackTolerance; }

  /// Returns true if mode is PEAK_FALLING or PEAK_MIN.
  bool modeInverted() const;

  /// Returns true if mode is PEAK_RISING or PEAK_FALLING.
  bool modeCrossing() const;

  /// Returns true if mode is PEAK_MAX or PEAK_MIN.
  bool modeApex() const;

  /// Sets mode.
  void mode(uint8_t mode);

  /// Returns mode.
  uint8_t mode() const { return _mode; }

  /**
   * Pushes value into the unit.
   * @param value the value sent to the unit
   * @return the new value of the unit
   */
  virtual float put(float value);

  /// Returns true if the triggerThreshold is crossed.
  virtual bool isOn() { return _onValue; }

protected:
  // Resets peak detection flags.
  void _reset();

  // Threshold values.
  float _triggerThreshold;
  float _reloadThreshold;
  float _fallbackTolerance;
  float _peakValue;

  // Thresholding mode.
  bool _onValue  : 1;
  uint8_t  _mode : 2;

  // Booleans used to keep track of signal value.
  bool _isHigh   : 1;
  bool _wasLow   : 1;
  bool _crossed  : 1;
  bool _firstRun : 1;

  // Unused extra space.
  uint8_t _data  : 1;
};


#endif
