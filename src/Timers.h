/*
 * Timers.h
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

#include <chrono>

#ifndef TIMERS_H_
#define TIMERS_H_

unsigned long getMicros();
unsigned long getMillis();

#endif