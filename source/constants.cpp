/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

#include <string>

#include "constants.hpp"

const std::string hajime_version = "0.2.0";

std::string hajimePath = "hajime.d/";

std::string serverSubpath = "servers/";

std::string defaultServerConfFile = hajimePath + serverSubpath + "default.json";

std::string hajDefaultConfFile = hajimePath + "hajime.json";

std::string logFile = hajimePath + "hajime.log";

long long defaultCounterInterval = 3;

long long defaultCounterMax = 20 * 60 * 24 * 14;

bool useTUI = false;