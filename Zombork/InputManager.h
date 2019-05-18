#pragma once
#include "Action.h"
#include "pch.h"

class InputManager
{
public:
	static InputManager &instance()
	{
		static InputManager instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	};

	Action* getActionFromInput();

private:
	struct InvalidInputException : public exception {
		const char * what() const throw () {
			return "Invalid input.";
		}
	};

	vector<common_defs::tokens> tokenize(string input);
	bool isValidToken(string token);

	map<string, common_defs::tokens> tokens_mapping = {
		// ACTIONS
		{"ATTACK", common_defs::ATTACK},
		{"CHECK", common_defs::CHECK},
		{"DROP", common_defs::DROP},
		{"GO", common_defs::GO},
		{"HELP", common_defs::HELP},
		{"LOOK", common_defs::LOOK},
		{"OPEN", common_defs::OPEN},
		{"PUT", common_defs::PUT},
		{"TAKE", common_defs::TAKE},

		// DIRECTIONS
		{"NORTH", common_defs::NORTH},
		{"SOUTH", common_defs::SOUTH},
		{"EAST", common_defs::EAST},
		{"WEST", common_defs::WEST},
		{"UP", common_defs::UP},
		{"DOWN", common_defs::DOWN},

		// ITEMS
		{"CURE", common_defs::CURE},
		{"DOOR", common_defs::DOOR},
		{"REVOLVER", common_defs::REVOLVER}
	};


};
