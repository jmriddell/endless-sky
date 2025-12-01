/* SavedGame.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "DataFile.h"
#include "DataNode.h"

#include <filesystem>
#include <functional>
#include <string>

class DataWriter;

class SaveGameManager {
public:
	explicit SaveGameManager(const std::string &savename);

	void Save(const DataNode &root, const std::string &dateString);
	DataNode Load();

	static std::string PathToRecent();
	static std::string GeneratePath(const std::string &first, const std::string &last);

private:
	std::string savename;

	void RotateBackups(const std::string &dateString);
	void UpdateRecentSave();
};
