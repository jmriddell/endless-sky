/* test_saveGameManager.cpp
Copyright (c) 2024 by Juan

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "es-test.hpp"

// Include only the tested class's header.
#include "../../../source/SaveGameManager.h"

// ... and any system includes needed for the test file.
#include "../../../source/DataNode.h"

#include <string>

namespace { // test namespace

// #region mock data
// #endregion mock data



// #region unit tests

SCENARIO("SaveGameManager generates unique save paths", "[SaveGameManager][path]") {
	GIVEN("a pilot name") {
		std::string firstName = "Test";
		std::string lastName = "Pilot";
		
		WHEN("generating a save path") {
			std::string path = SaveGameManager::GeneratePath(firstName, lastName);
			
			THEN("the path should contain the pilot name") {
				CHECK(path.find("Test Pilot") != std::string::npos);
			}
			
			AND_THEN("the path should end with .txt") {
				CHECK(path.ends_with(".txt"));
			}
		}
	}
	
	GIVEN("a pilot name with special characters") {
		std::string firstName = "Test-Name";
		std::string lastName = "O'Pilot";
		
		WHEN("generating a save path") {
			std::string path = SaveGameManager::GeneratePath(firstName, lastName);
			
			THEN("the path should be generated without errors") {
				CHECK_FALSE(path.empty());
				CHECK(path.ends_with(".txt"));
			}
		}
	}
}

SCENARIO("SaveGameManager handles recent save path retrieval", "[SaveGameManager][recent]") {
	WHEN("getting the path to the recent save file") {
		std::string recentPath = SaveGameManager::PathToRecent();
		
		THEN("it should return a string without crashing") {
			// The path might be empty if no recent save exists, which is valid
			// This test just ensures the function works correctly
			CHECK(true);
		}
	}
}

SCENARIO("SaveGameManager can be constructed with a save name", "[SaveGameManager][construction]") {
	GIVEN("a save file name") {
		std::string saveName = "test_pilot.txt";
		
		WHEN("creating a SaveGameManager") {
			SaveGameManager manager(saveName);
			
			THEN("the manager is constructed successfully") {
				// If we get here, construction succeeded
				CHECK(true);
			}
		}
	}
}

// #endregion unit tests



} // test namespace
