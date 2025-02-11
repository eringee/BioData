/*
 * PeakDetector.cpp
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

#include "PeakDetector.h"
#include "utils.h"

PeakDetector::PeakDetector(float triggerThreshold_, uint8_t mode_)
  : 
    _triggerThreshold(triggerThreshold_),
    _reloadThreshold(triggerThreshold_),
    _fallbackTolerance(0.1f),
    _mode(PEAK_RISING) // will be reset properly when calling mode(mode_)
{
  // Assign mode.
  mode(mode_);

  // Assign triggerThreshold (flip if necessary).
  triggerThreshold(triggerThreshold_);

  // Set default values.
  reloadThreshold(triggerThreshold_);
  fallbackTolerance(0.1f);

  // Reset detector.
  _reset();
}

void PeakDetector::triggerThreshold(float triggerThreshold) {
  triggerThreshold = modeInverted() ? -triggerThreshold : triggerThreshold;

  if (_triggerThreshold != triggerThreshold) {
    _triggerThreshold = triggerThreshold;
    _reset();
  }
}

void PeakDetector::reloadThreshold(float reloadThreshold) {
  if (modeInverted()) reloadThreshold = -reloadThreshold;
  reloadThreshold = min(reloadThreshold, _triggerThreshold);

  if (_reloadThreshold != reloadThreshold) {
    _reloadThreshold = reloadThreshold;
    _reset();
  }
}

void PeakDetector::fallbackTolerance(float fallbackTolerance) {
  _fallbackTolerance = clamp(fallbackTolerance, 0.0f, 1.0f);
}

void PeakDetector::mode(uint8_t mode) {
  // Save current state.
  bool wasInverted = modeInverted();

  // Change mode.
  _mode = clamp(mode, (uint8_t)PEAK_RISING, (uint8_t)PEAK_MIN);

  // If mode inversion was changed, adjust triggerThresholds.
  if (modeInverted() != wasInverted) {
    // Flip.
    _triggerThreshold = -_triggerThreshold;
    _reloadThreshold = -_reloadThreshold;
  }
}

bool PeakDetector::modeInverted() const {
  return (_mode == PEAK_FALLING || _mode == PEAK_MIN);
}

bool PeakDetector::modeCrossing() const {
  return (_mode == PEAK_RISING || _mode == PEAK_FALLING);
}

bool PeakDetector::modeApex() const {
  return !modeCrossing();
}

float PeakDetector::put(float value) {
  // Flip value.
  if (modeInverted())
    value = -value;
  
  // Check if value is above triggerThreshold ("high" flag).
  bool high     = (value >= _triggerThreshold); // value is high if above triggerThreshold

  // Initialize _wasLow on first run.
  if (_firstRun) {
    _wasLow = !high;
    _firstRun = false;
  }
  
  else {

    bool crossing = (high && _wasLow);            // value is crossing if just crossed triggerThreshold
    bool isMax    = (value > _peakValue);         // value is new max if higher than current peak value

    // At the moment of crossing, reset flags.
    if (crossing) {
      _wasLow  = false;
      _crossed = true;
    }

    // Check if value is below reloadThreshold.
    else if (value <= _reloadThreshold)
      _wasLow = true;

    // Perform fallback detection operations.
    bool fallingBack = false;
    if (_crossed) {
      // Set peak value.
      if (isMax) {
        _peakValue = value;
      }


      // Check for fallback (only if value is below peak ie. !isMax).
      // Fallback detected after crossing and falling below maximum and either:
      // (1) drops by % tolerance between peak and triggerThreshold OR
      // (2) falls below triggerThreshold (!high)
      else if ((map(value, _peakValue, _triggerThreshold,0,1) >= _fallbackTolerance &&
                          _peakValue != _triggerThreshold) // deal with special case where mapTo01(...) would return 0.5 by default
                  || !high) {

        // Fallback detected.
        fallingBack = true;

        // Reset.
        _crossed = false;
        _peakValue = -FLT_MAX;
      }
    }

    // Assign value depending on mode.
    _onValue = (modeCrossing() ? crossing : fallingBack);
  }

  return _onValue;
}

void PeakDetector::_reset() {
  // Init peak value to -inf.
  _peakValue = -FLT_MAX;

  // Init all flags.
  _onValue = _isHigh = _crossed = false;
  _wasLow = true;

  // Set first run flag.
  _firstRun = true;
}


