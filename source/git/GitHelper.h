#pragma once

#include <git2.h>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

class GitHelper {
public:
	static void Init();
	static void Shutdown();

	static bool IsRepo(const std::filesystem::path &path);
	static void CreateCommit(const std::filesystem::path &repoPath, const std::string &filename, const std::string &message);
	static std::string GetFileContent(const std::filesystem::path &repoPath, const std::string &commitHash, const std::string &filename);
	static std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> ListCommits(const std::filesystem::path &repoPath);
	static std::string GetLatestCommitMessage(const std::filesystem::path &repoPath);
};
