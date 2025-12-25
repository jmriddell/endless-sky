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

	DataWriter out(savename);
	out.Write(root);

	// Save global conditions:
	DataWriter globalConditions(Files::Config() / "global conditions.txt");
	GameData::GlobalConditions().Save(globalConditions);

	// Initialize git and commit the changes
	git_libgit2_init();

	filesystem::path savePath(savename);
	filesystem::path repoPath = savePath.parent_path();
	string filename = savePath.filename().string();

	git_repository *repo = nullptr;
	if (git_repository_open(&repo, repoPath.string().c_str()) != 0) {
		git_repository_init(&repo, repoPath.string().c_str(), 0);
	}

	if (repo) {
		git_index *index = nullptr;
		if (git_repository_index(&index, repo) == 0) {
			git_index_add_bypath(index, filename.c_str());
			git_index_write(index);

			git_oid tree_oid;
			git_index_write_tree(&tree_oid, index);
			git_tree *tree = nullptr;
			git_tree_lookup(&tree, repo, &tree_oid);

			git_signature *sig = nullptr;
			git_signature_now(&sig, "Endless Sky", "endless-sky@localhost");

			git_oid parent_oid;
			git_commit *parent = nullptr;
			if (git_reference_name_to_id(&parent_oid, repo, "HEAD") == 0) {
				git_commit_lookup(&parent, repo, &parent_oid);
			}

			git_oid commit_oid;
			// Use the date string as the commit message
			git_commit_create_v(
				&commit_oid, repo, "HEAD", sig, sig,
				NULL, dateString.c_str(), tree, parent ? 1 : 0, parent);

			git_commit_free(parent);
			git_signature_free(sig);
			git_tree_free(tree);
			git_index_free(index);
		}
		git_repository_free(repo);
	}

	git_libgit2_shutdown();
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
	string folderName = first + " " + last;

	// If there are multiple pilots with the same name, append a number to the
	// pilot name to generate a unique folder name.
	string folderPath = (Files::Saves() / folderName).string();
	int index = 0;
	while(true)
	{
		string path = folderPath;
		if(index++)
			path += " " + to_string(index);

		if(!Files::Exists(path)) {
			// Create the directory
			filesystem::create_directories(path);
			// Return the path to the save file within that directory
			return (filesystem::path(path) / (folderName + ".txt")).string();
		}
	}
}