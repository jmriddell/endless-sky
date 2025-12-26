#include "es-test.hpp"
#include "../../../source/Files.h"
#include "../../../source/git/GitHelper.h"
// #include "LoadPanel.h" // Avoid including LoadPanel as it pulls in UI/OpenGL
// #include "PlayerInfo.h"
// #include "UI.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

// Mock LoadPanel to test UpdateLists logic
// We can't easily instantiate LoadPanel without a window, but we can copy the logic
// or use a helper function if we refactored.
// Since I can't easily refactor the whole class now without breaking things,
// I will verify the components: Files::ListDirectories, GitHelper, and the logic
// I added.

// Actually, I can instantiate LoadPanel if I mock UI and PlayerInfo, but UI interacts with OpenGL.
// So unit testing LoadPanel::UpdateLists directly is hard.

// Instead, I will write a test that verifies the *ingredients* of the fix:
// 1. Files::ListDirectories returns directories.
// 2. GitHelper::IsRepo detects the repo.
// 3. GitHelper::ListCommits returns the commits.
// 4. And I'll simulate the integration logic.

TEST_CASE("Save Loading Logic", "[save][git]") {
    // Setup temporary saves directory
    filesystem::path testSaves = "test_saves_load";
    if (filesystem::exists(testSaves)) filesystem::remove_all(testSaves);
    filesystem::create_directories(testSaves);

    // Mock Files::Saves() by changing the config path or using relative paths
    // But Files::Saves() is static.
    // I can't easily swap Files::Saves().
    // However, I can test the logic that I *added* if I extract it or just test the primitives.

    // Let's test the GitHelper part primarily, as that's the new backend logic.
    // And verify Files::ListDirectories works.

    SECTION("Files::ListDirectories works") {
        filesystem::path dir1 = testSaves / "PilotDir";
        filesystem::create_directory(dir1);
        filesystem::path file1 = testSaves / "PilotFile.txt";
        ofstream(file1) << "content";

        vector<filesystem::path> dirs = Files::ListDirectories(testSaves);
        bool found = false;
        for (const auto& p : dirs) {
            if (p.filename() == "PilotDir") found = true;
        }
        CHECK(found);

        // Ensure file is NOT in dirs
        for (const auto& p : dirs) {
            CHECK(p.filename() != "PilotFile.txt");
        }
    }

    SECTION("Git Logic Simulation") {
        filesystem::path repoPath = testSaves / "GitPilot";
        filesystem::create_directories(repoPath);

        GitHelper::Init();

        // Init repo
        // We need to use GitHelper::CreateCommit or manually init.
        // GitHelper::CreateCommit initializes if needed.
        string filename = "GitPilot.txt";
        ofstream(repoPath / filename) << "save data";

        GitHelper::CreateCommit(repoPath, filename, "First Snapshot");

        CHECK(GitHelper::IsRepo(repoPath));

        auto commits = GitHelper::ListCommits(repoPath);
        CHECK(commits.size() == 1);
        CHECK(commits[0].first.find("First Snapshot") != string::npos);

        // Add another commit
        ofstream(repoPath / filename) << "save data 2";
        GitHelper::CreateCommit(repoPath, filename, "Second Snapshot");

        commits = GitHelper::ListCommits(repoPath);
        CHECK(commits.size() == 2);

        // Verify content retrieval
        string id = commits[0].first; // Newest is first usually? ListCommits sorts by time?
        // GitHelper::ListCommits sorts GIT_SORT_TIME.
        // Let's check the hash part.
        string hash = id.substr(7, 40);
        string content = GitHelper::GetFileContent(repoPath, hash, filename);
        // The newest commit should be "save data 2".
        // The older one "save data".

        // Wait, "save data 2" was just written.
        // Let's see which one is which.
        // They are sorted by time. If created instantly, order might be ambiguous if resolution is low?
        // But usually creating a commit takes > 0ms.

        // We can check messages.
        bool foundFirst = false;
        bool foundSecond = false;
        for(auto& c : commits) {
            if(c.first.find("First Snapshot") != string::npos) {
                string h = c.first.substr(7, 40);
                string text = GitHelper::GetFileContent(repoPath, h, filename);
                CHECK(text == "save data");
                foundFirst = true;
            }
            if(c.first.find("Second Snapshot") != string::npos) {
                 string h = c.first.substr(7, 40);
                string text = GitHelper::GetFileContent(repoPath, h, filename);
                CHECK(text == "save data 2");
                foundSecond = true;
            }
        }
        CHECK(foundFirst);
        CHECK(foundSecond);

        GitHelper::Shutdown();
    }

    filesystem::remove_all(testSaves);
}
