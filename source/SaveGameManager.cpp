/* SaveGameManager.cpp
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

#include "SaveGameManager.h"

#include "ConditionsStore.h"
#include "DataFile.h"
#include "DataWriter.h"
#include "Files.h"
#include "GameData.h"
#include "Preferences.h"
#include "SavedGame.h"

using namespace std;

SaveGameManager::SaveGameManager(const string &savename)
	: savename(savename)
{
}

void SaveGameManager::Save(const DataNode &root, const string &dateString)
{
	UpdateRecentSave();
	RotateBackups(dateString);

	DataWriter out(savename);
	out.Write(root);

	// Save global conditions:
	DataWriter globalConditions(Files::Config() / "global conditions.txt");
	GameData::GlobalConditions().Save(globalConditions);
}

void SaveGameManager::RotateBackups(const string &dateString)
{
	if(savename.rfind(".txt") != savename.length() - 4)
		return;

	// Only update the backups if this save will have a newer date.
	SavedGame saved(savename);
	if(saved.GetDate() == dateString)
		return;

	string root = savename.substr(0, savename.length() - 4);
	const int previousCount = Preferences::GetPreviousSaveCount();
	const string rootPrevious = root + "~~previous-";
	for(int i = previousCount - 1; i > 0; --i)
	{
		const string toMove = rootPrevious + to_string(i) + ".txt";
		if(Files::Exists(toMove))
			Files::Move(toMove, rootPrevious + to_string(i + 1) + ".txt");
	}
	if(Files::Exists(savename))
		Files::Move(savename, rootPrevious + "1.txt");
}

void SaveGameManager::UpdateRecentSave()
{
	// Remember that this was the most recently saved player.
	Files::Write(Files::Config() / "recent.txt", savename + '\n');
}

DataNode SaveGameManager::Load()
{
	DataFile file(savename);
	// We return the root node of the file. Since DataNode has children,
	// we need to make sure we are returning a node that contains the children of the file.
	// DataFile has a 'root' member which is a DataNode.
	// However, DataFile::root is private. We can iterate over DataFile to get children.
	// But we want to return a single DataNode that acts as the root.
	// Let's check DataFile.h again. It has begin() and end() which iterate over root.children.
	
	// Actually, DataFile IS NOT a DataNode, but it HAS a root DataNode.
	// But the root node of DataFile is usually empty and just holds children.
	// So we can construct a DataNode and add all children of DataFile to it.
	// Or better, if we can access the root, we can return it.
	// But we can't access it directly.
	
	DataNode root;
	for(const DataNode &child : file)
		root.AddChild(child);
	
	return root;
}

string SaveGameManager::PathToRecent()
{
	string recentPath = Files::Read(Files::Config() / "recent.txt");
	// Trim trailing whitespace (including newlines) from the path.
	while(!recentPath.empty() && recentPath.back() <= ' ')
		recentPath.pop_back();

	return recentPath;
}

string SaveGameManager::GeneratePath(const string &first, const string &last)
{
	string fileName = first + " " + last;

	// If there are multiple pilots with the same name, append a number to the
	// pilot name to generate a unique file name.
	string filePath = (Files::Saves() / fileName).string();
	int index = 0;
	while(true)
	{
		string path = filePath;
		if(index++)
			path += " " + to_string(index);
		path += ".txt";

		if(!Files::Exists(path))
			return path;
	}
}