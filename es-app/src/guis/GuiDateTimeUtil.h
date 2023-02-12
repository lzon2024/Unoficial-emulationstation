// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2022-present kkoshelev

#pragma once

#include "GuiSettings.h"

class GuiDateTimeUtil : public GuiSettings
{
public:
	static void show(Window* window);

protected:
  GuiDateTimeUtil(Window* window);
};
