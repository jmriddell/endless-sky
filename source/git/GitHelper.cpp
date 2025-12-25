#include "GitHelper.h"
#include <iostream>

using namespace std;

void GitHelper::Init() {
	git_libgit2_init();
}

void GitHelper::Shutdown() {
	git_libgit2_shutdown();
}

bool GitHelper::IsRepo(const filesystem::path &path) {
	git_repository *repo = nullptr;
	if (git_repository_open(&repo, path.string().c_str()) == 0) {
		git_repository_free(repo);
		return true;
	}
	return false;
}

void GitHelper::CreateCommit(const filesystem::path &repoPath, const string &filename, const string &message) {
	git_repository *repo = nullptr;
	if (git_repository_open(&repo, repoPath.string().c_str()) != 0) {
		if (git_repository_init(&repo, repoPath.string().c_str(), 0) != 0)
			return;
	}

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
		git_commit_create_v(
			&commit_oid, repo, "HEAD", sig, sig,
			NULL, message.c_str(), tree, parent ? 1 : 0, parent);

		if (parent) git_commit_free(parent);
		git_signature_free(sig);
		git_tree_free(tree);
		git_index_free(index);
	}
	git_repository_free(repo);
}

string GitHelper::GetFileContent(const filesystem::path &repoPath, const string &commitHash, const string &filename) {
	string content;
	git_repository *repo = nullptr;
	if (git_repository_open(&repo, repoPath.string().c_str()) == 0) {
		git_oid oid;
		git_oid_fromstr(&oid, commitHash.c_str());
		git_commit *commit = nullptr;
		if (git_commit_lookup(&commit, repo, &oid) == 0) {
			git_tree *tree = nullptr;
			git_commit_tree(&tree, commit);

			git_tree_entry *entry = nullptr;
			if (git_tree_entry_bypath(&entry, tree, filename.c_str()) == 0) {
				const git_oid *blob_oid = git_tree_entry_id(entry);
				git_blob *blob = nullptr;
				if (git_blob_lookup(&blob, repo, blob_oid) == 0) {
					const char *raw = (const char *)git_blob_rawcontent(blob);
					size_t size = git_blob_rawsize(blob);
					content.assign(raw, size);
					git_blob_free(blob);
				}
				git_tree_entry_free(entry);
			}
			git_tree_free(tree);
			git_commit_free(commit);
		}
		git_repository_free(repo);
	}
	return content;
}

vector<pair<string, chrono::system_clock::time_point>> GitHelper::ListCommits(const filesystem::path &repoPath) {
	vector<pair<string, chrono::system_clock::time_point>> commits;
	git_repository *repo = nullptr;
	if (git_repository_open(&repo, repoPath.string().c_str()) == 0) {
		git_revwalk *walker = nullptr;
		git_revwalk_new(&walker, repo);
		git_revwalk_push_head(walker);
		git_revwalk_sorting(walker, GIT_SORT_TIME);

		git_oid oid;
		while (git_revwalk_next(&oid, walker) == 0) {
			git_commit *commit = nullptr;
			if (git_commit_lookup(&commit, repo, &oid) == 0) {
				git_time_t time = git_commit_time(commit);
				const char *message = git_commit_message(commit);
				char oid_str[GIT_OID_HEXSZ + 1];
				git_oid_tostr(oid_str, sizeof(oid_str), &oid);

				string msg(message);
				if (!msg.empty() && msg.back() == '\n') msg.pop_back();

				commits.emplace_back("commit:" + string(oid_str) + "~" + msg, chrono::system_clock::from_time_t(time));
				git_commit_free(commit);
			}
		}
		git_revwalk_free(walker);
		git_repository_free(repo);
	}
	return commits;
}

string GitHelper::GetLatestCommitMessage(const filesystem::path &repoPath) {
	string msg;
	git_repository *repo = nullptr;
	if (git_repository_open(&repo, repoPath.string().c_str()) == 0) {
		git_oid oid;
		if (git_reference_name_to_id(&oid, repo, "HEAD") == 0) {
			git_commit *commit = nullptr;
			if (git_commit_lookup(&commit, repo, &oid) == 0) {
				const char *message = git_commit_message(commit);
				msg = message;
				if (!msg.empty() && msg.back() == '\n') msg.pop_back();
				git_commit_free(commit);
			}
		}
		git_repository_free(repo);
	}
	return msg;
}
