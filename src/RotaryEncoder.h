// -----
// RotaryEncoder.h - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
//
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
//
// This work is licensed under a BSD 3-Clause style license,
// https://www.mathertel.de/License.aspx.
//
// More information on: http://www.mathertel.de/Arduino
// -----
// 18.01.2014 created by Matthias Hertel
// 16.06.2019 pin initialization using INPUT_PULLUP
// 10.11.2020 Added the ability to obtain the encoder RPM
// 29.01.2021 Options for using rotary encoders with 2 state changes per latch.
// -----

#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "Arduino.h"

class RotaryEncoder
{
public:
  enum class Direction {
    NOROTATION = 0,
    CLOCKWISE = 1,
    COUNTERCLOCKWISE = -1
  };

  enum class LatchMode {
    FOUR3 = 1, // 4 steps, Latch at position 3 only (compatible to older versions)
    FOUR0 = 2, // 4 steps, Latch at position 0 (reverse wirings)
    TWO03 = 3  // 2 steps, Latch at position 0 and 3
  };

  // ----- Constructor -----
  RotaryEncoder(uint8_t u08Pin1, uint8_t u08Pin2, LatchMode mode = LatchMode::FOUR0);

  // retrieve the current position
  int32_t getPosition();

  // simple retrieve of the direction the knob was rotated at. 0 = No rotation, 1 = Clockwise, -1 = Counter Clockwise
  Direction getDirection();

  // adjust the current position
  void setPosition(int32_t newPosition);

  // call this function every some milliseconds or by using an interrupt for handling state changes of the rotary encoder.
  void tick(void);

  // Returns the time in milliseconds between the current observed
  uint32_t getMillisBetweenRotations() const;

  // Returns the RPM
  uint32_t getRPM();

private:
  uint8_t _u08Pin1, _u08Pin2; // Arduino pins used for the encoder.

  LatchMode _eMode; // Latch mode from initialization

  volatile int8_t _i08OldState;

  volatile int32_t _i32Position;        // Internal position (4 times _i32PositionExt)
  volatile int32_t _i32PositionExt;     // External position
  volatile int32_t _i32PositionExtPrev; // External position (used only for direction checking)

  uint32_t _u32PositionExtTime;     // The time the last position change was detected.
  uint32_t _u32PositionExtTimePrev; // The time the previous position change was detected.
};

#endif

// End