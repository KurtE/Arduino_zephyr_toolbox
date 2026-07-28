/*
 * Copyright (c) Arduino s.r.l. and/or its affiliated companies
 * Copyright (c) 2022 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This is an extended version that adds Analog Frequency...
 */


#pragma once
void XanalogWriteFrequency(pin_size_t pin, float freq);
float XanalogWriteFrequency(pin_size_t pin);
int XanalogWriteLastStatus();
void XanalogWrite(pin_size_t pinNumber, int value);
