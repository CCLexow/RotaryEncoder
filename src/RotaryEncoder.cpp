// -----
// RotaryEncoder.cpp - Library for using rotary encoders.
// This class is implemented for use with the Arduino environment.
//
// Copyright (c) by Matthias Hertel, http://www.mathertel.de
//
// This work is licensed under a BSD 3-Clause style license,
// https://www.mathertel.de/License.aspx.
//
// More information on: http://www.mathertel.de/Arduino
// -----
// Changelog: see RotaryEncoder.h
// -----

#include "RotaryEncoder.h"
#include "Arduino.h"

#define LATCH0 0 // input state at position 0
#define LATCH3 3 // input state at position 3


// The array holds the values �1 for the entries where a position was decremented,
// a 1 for the entries where the position was incremented
// and 0 in all the other (no change or not valid) cases.

const int8_t ci08KNOBDIR[] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0};


// positions: [3] 1 0 2 [3] 1 0 2 [3]
// [3] is the positions where my rotary switch detends
// ==> right, count up
// <== left,  count down


// ----- Initialization and Default Values -----

RotaryEncoder::RotaryEncoder(uint8_t u08Pin1, uint8_t u08Pin2, LatchMode mode)
{
  // Remember Hardware Setup
  _u08Pin1 = u08Pin1;
  _u08Pin2 = u08Pin2;
  _eMode = mode;

  // Setup the input pins and turn on pullup resistor
  pinMode(u08Pin1, INPUT_PULLUP);
  pinMode(u08Pin2, INPUT_PULLUP);

  // when not started in motion, the current state of the encoder should be 3
  _i08OldState = 3;

  // start with position 0;
  _i32Position = 0;
  _i32PositionExt = 0;
  _i32PositionExtPrev = 0;
} // RotaryEncoder()


int32_t RotaryEncoder::getPosition()
{
  return _i32PositionExt;
} // getPosition()


RotaryEncoder::Direction RotaryEncoder::getDirection()
{
  RotaryEncoder::Direction ret = Direction::NOROTATION;

  if (_i32PositionExtPrev > _i32PositionExt)
  {
    ret = Direction::COUNTERCLOCKWISE;
    _i32PositionExtPrev = _i32PositionExt;
  }
  else if (_i32PositionExtPrev < _i32PositionExt)
  {
    ret = Direction::CLOCKWISE;
    _i32PositionExtPrev = _i32PositionExt;
  }
  else
  {
    ret = Direction::NOROTATION;
    _i32PositionExtPrev = _i32PositionExt;
  }

  return ret;
}


void RotaryEncoder::setPosition(int32_t newPosition)
{
  switch (_eMode)
  {
    case LatchMode::FOUR3:
    case LatchMode::FOUR0:
    {
      // only adjust the external part of the position.
      _i32Position = ((newPosition << 2) | (_i32Position & 0x03L));
      _i32PositionExt = newPosition;
      _i32PositionExtPrev = newPosition;
      break;
    }

    case LatchMode::TWO03:
    {
      // only adjust the external part of the position.
      _i32Position = ((newPosition << 1) | (_i32Position & 0x01L));
      _i32PositionExt = newPosition;
      _i32PositionExtPrev = newPosition;
      break;
    }
  } // switch

} // setPosition()


void RotaryEncoder::tick(void)
{
  int sig1 = digitalRead(_u08Pin1);
  int sig2 = digitalRead(_u08Pin2);
  int8_t thisState = sig1 | (sig2 << 1);

  if (_i08OldState != thisState)
  {
    _i32Position += ci08KNOBDIR[thisState | (_i08OldState << 2)];

    switch (_eMode)
    {
      case LatchMode::FOUR3:
      {
        if (thisState == LATCH3) {
          // The hardware has 4 steps with a latch on the input state 3
          _i32PositionExt = _i32Position >> 2;
          _u32PositionExtTimePrev = _u32PositionExtTime;
          _u32PositionExtTime = millis();
        }
        break;
      }

      case LatchMode::FOUR0:
      {
        if (thisState == LATCH0)
        {
          // The hardware has 4 steps with a latch on the input state 0
          _i32PositionExt = _i32Position >> 2;
          _u32PositionExtTimePrev = _u32PositionExtTime;
          _u32PositionExtTime = millis();
        }
        break;
      }

      case LatchMode::TWO03:
      {
        if ((thisState == LATCH0) || (thisState == LATCH3))
        {
          // The hardware has 2 steps with a latch on the input state 0 and 3
          _i32PositionExt = _i32Position >> 1;
          _u32PositionExtTimePrev = _u32PositionExtTime;
          _u32PositionExtTime = millis();
        }
        break;
      }
    } // switch

    _i08OldState = thisState;
  } // if
} // tick()


uint32_t RotaryEncoder::getMillisBetweenRotations() const
{
  return (_u32PositionExtTime - _u32PositionExtTimePrev);
}

uint32_t RotaryEncoder::getRPM()
{
  // calculate max of difference in time between last position changes or last change and now.
  uint32_t timeBetweenLastPositions = _u32PositionExtTime - _u32PositionExtTimePrev;
  uint32_t timeToLastPosition = millis() - _u32PositionExtTime;
  uint32_t t = max(timeBetweenLastPositions, timeToLastPosition);
  return 60000.0 / ((float)(t * 20));
}


// End