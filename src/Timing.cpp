/*
 * Timers.cpp
 *
 * This file defines general timer functions that do not depend on Arduino framework
 * 
 * This file is part of the BioData project
 * (c) 2018 Erin Gee
 *
 * Contributing authors:
 * (c) 2024 Luana Belinsky
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
#include "Timing.h"

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <iostream>
#include <chrono>
#endif

Timing::Timing() : start_time(0) {}

void Timing::start() {
    start_time = current_time();
}

unsigned long Timing::elapsedMicros() {
    return current_time() - start_time;
}

bool Timing::hasElapsed(unsigned long duration) {
    return elapsedMicros() >= duration;
}

unsigned long Timing::current_time() {
#if defined(ARDUINO)
    return micros();
#else
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
#endif
}

unsigned long Timing::getMicros() {
    return elapsedMicros();
}

unsigned long Timing::getMillis() {
    return getMicros() / 1000;
}

