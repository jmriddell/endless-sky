#include "es-test.hpp"

#include "../../../source/SaveGameManager.h"
#include "../../../source/Files.h"
#include "../../../source/DataNode.h"
#include "../../../source/DataWriter.h"

#include <git2.h>
#include <filesystem>
#include <string>
#include <fstream>

using namespace std;

namespace {

SCENARIO("SaveGameManager git integration", "[SaveGameManager][git]") {
	// Setup a temporary directory for testing
	string pilotName = "GitTestPilot";
	string pilotLast = "Tester";
	string folderName = pilotName + " " + pilotLast;
	filesystem::path savePath = Files::Saves() / folderName;
	filesystem::path saveFile = savePath / (folderName + ".txt");

	// Ensure clean state
	if (Files::Exists(savePath)) {
		Files::Delete(savePath);
	}

	GIVEN("A new pilot save") {
		SaveGameManager manager((Files::Saves() / folderName / (folderName + ".txt")).string());
		DataNode root;
		DataNode pilot;
		pilot.AddToken("pilot");
		pilot.AddToken(pilotName);
		pilot.AddToken(pilotLast);
		root.AddChild(pilot);

		// Ensure directory is created
		SaveGameManager::GeneratePath(pilotName, pilotLast);

		WHEN("Saving the game") {
			manager.Save(root, "01-01-3000");

			THEN("A git repository should be initialized") {
				CHECK(Files::Exists(savePath / ".git"));
			}

			AND_THEN("A commit should be created") {
				git_libgit2_init();
				git_repository *repo = nullptr;
				CHECK(git_repository_open(&repo, savePath.string().c_str()) == 0);

				if(repo) {
					git_oid oid;
					git_reference_name_to_id(&oid, repo, "HEAD");
					git_commit *commit = nullptr;
					CHECK(git_commit_lookup(&commit, repo, &oid) == 0);

					if(commit) {
						CHECK(string(git_commit_message(commit)) == "01-01-3000");
						git_commit_free(commit);
					}
					git_repository_free(repo);
				}
				git_libgit2_shutdown();
			}
		}

		WHEN("Saving multiple times") {
			manager.Save(root, "01-01-3000");
			manager.Save(root, "02-01-3000");

			THEN("Multiple commits should exist") {
				git_libgit2_init();
				git_repository *repo = nullptr;
				CHECK(git_repository_open(&repo, savePath.string().c_str()) == 0);

				if(repo) {
					git_revwalk *walker = nullptr;
					git_revwalk_new(&walker, repo);
					git_revwalk_push_head(walker);

					int count = 0;
					git_oid oid;
					while(git_revwalk_next(&oid, walker) == 0) {
						count++;
					}
					CHECK(count == 2);

					git_revwalk_free(walker);
					git_repository_free(repo);
				}
				git_libgit2_shutdown();
			}
		}
	}

	// Cleanup
	if (Files::Exists(savePath)) {
		// Recursive delete for directory
		std::filesystem::remove_all(savePath);
	}
}

SCENARIO("Multiple pilots have separate repos", "[SaveGameManager][git][multi]") {
	string pilot1 = "PilotOne";
	string pilot2 = "PilotTwo";

	filesystem::path path1 = Files::Saves() / (pilot1 + " Test");
	filesystem::path path2 = Files::Saves() / (pilot2 + " Test");

	// Cleanup
	std::filesystem::remove_all(path1);
	std::filesystem::remove_all(path2);

	SaveGameManager::GeneratePath(pilot1, "Test");
	SaveGameManager::GeneratePath(pilot2, "Test");

	SaveGameManager man1((path1 / (pilot1 + " Test.txt")).string());
	SaveGameManager man2((path2 / (pilot2 + " Test.txt")).string());

	DataNode root;

	man1.Save(root, "Date1");
	man2.Save(root, "Date2");

	CHECK(Files::Exists(path1 / ".git"));
	CHECK(Files::Exists(path2 / ".git"));

	// Verify repo 1 has 1 commit with "Date1"
	{
		git_libgit2_init();
		git_repository *repo = nullptr;
		CHECK(git_repository_open(&repo, path1.string().c_str()) == 0);
		if(repo) {
			git_oid oid;
			git_reference_name_to_id(&oid, repo, "HEAD");
			git_commit *commit = nullptr;
			CHECK(git_commit_lookup(&commit, repo, &oid) == 0);
			if(commit) {
				CHECK(string(git_commit_message(commit)) == "Date1");
				git_commit_free(commit);
			}
			git_repository_free(repo);
		}
		git_libgit2_shutdown();
	}

	// Cleanup
	std::filesystem::remove_all(path1);
	std::filesystem::remove_all(path2);
}

} // namespace
