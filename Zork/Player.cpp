#include "pch.h"
#include "Player.h"
#include "Weapon.h"
#include "BlessedAppleAction.h"
#include "GameManager.h"

void Player::spawnPlayer() {
	lifeCounter = 3;
	hasEquippedWeapon = false;
}

string Player::lookAround() {
	return currentRoom->getDescription();
}

bool Player::canGoToDirection(common_defs::tokens direction) {
	return currentRoom->isAccessibleConnection(direction);
}

string Player::lookAtDirection(common_defs::tokens direction) {
	return currentRoom->getDirectionInfo(direction);
}

void Player::moveToDirection(common_defs::tokens direction) {
	Room *nextRoom = currentRoom->getRoomInDirection(direction);
	if (nextRoom->triggersEvent()) {
		GameManager::instance().setTriggeredEvent(nextRoom->getTriggeredEvent());
		nextRoom->setTriggeredEvent(NULL);
	}
	moveToRoom(nextRoom);
}

void Player::moveToRoom(Room* nextRoom) {
	// TODO: Possible memory leak? Need to check
	currentRoom = nextRoom; 
}

bool Player::canFindItem(common_defs::tokens item) {
	return currentRoom->containsItem(item) | currentRoom->containsWeapon(item);
}

string Player::pickItem(common_defs::tokens item) {
	Item pickedItem = currentRoom->takeItem(item);
	inventory.push_back(pickedItem);

	return pickedItem.getName();
}

void Player::pickItem(Item item) {
	inventory.push_back(item);
}

bool Player::isInInventory(common_defs::tokens item) {
	for (Item i : inventory) {
		if (i.getAssociatedToken() == item) {
			return true;
		}
	}

	if (hasEquippedWeapon) {
		return currentWeaponIs(item);
	}
	return false;
	
}

string Player::dropItem(common_defs::tokens item) {
	std::list<Item>::iterator it;
	for (it = inventory.begin(); it != inventory.end(); ++it) {
		if (it->getAssociatedToken() == item) {
			Item droppedItem = *it;
			currentRoom->putItem(droppedItem);
			inventory.erase(it);
			return droppedItem.getName();
		}
	}

	if (hasEquippedWeapon & currentWeaponIs(item)) {
		dropCurrentWeapon();
		return equippedWeapon.getName();
	}

	throw "Item not found in inventory, this should never happen.";
}

string Player::describeSelf() {
	string description = "--STATS--\n- HEALTH: " + to_string(lifeCounter) + "\n- ATTACK: ";
	if (!hasEquippedWeapon) {
		description = description + "0\n\n--EQUIP--\nYou have no equipped weapon.";
	}
	else {
		description = description + to_string(equippedWeapon.getAttack()) + "\n\n--EQUIP--\nYou are equipped with a " + equippedWeapon.getName() + ".";
	}

	description = description + "\n\n--INVENTORY--";

	if (inventory.size() == 0) {
		return description + "\nYour inventory is empty.";
	}

	for (Item i : inventory) {
		description = description + "\n- " + i.getName();
	}

	return description;
}

string Player::describeItemFromInventory(common_defs::tokens item) {
	std::list<Item>::iterator it;
	for (it = inventory.begin(); it != inventory.end(); ++it) {
		if (it->getAssociatedToken() == item) {
			return it->getDescription();
		}
	}

	if (hasEquippedWeapon & currentWeaponIs(item)) {
		return equippedWeapon.getDescription();
	}

	throw "Item not found in inventory, this should never happen.";

}

bool Player::canFindEnemy(common_defs::tokens enemy) {
	return currentRoom->containsEnemy(enemy);
}

string Player::attack(common_defs::tokens enemy) {
	Enemy e = currentRoom->getEnemy(enemy);
	string battleResult;

	int weaponAttack;
	if (hasEquippedWeapon) {
		weaponAttack = equippedWeapon.getAttack();
	}
	else {
		weaponAttack = 0;
	}

	if (weaponAttack >= e.getDefense()) {
		currentRoom->dropEnemyLoot(enemy);
		currentRoom->killEnemy(enemy);
		battleResult =  "You killed the " + e.getName() + "! It dropped some loot.";
	}
	else {
		--lifeCounter;
		if (lifeCounter == 0) {
			GameManager::instance().triggerGameEnding(GameManager::end_game_reason::DEATH);
			battleResult =  "You couldn't defeat the " + e.getName() + "! It attacks you and takes your last life.";
		}
		else {
			battleResult =  "You couldn't defeat the " + e.getName() + "! It attacks you and takes one of your lifes.";
		}
	}

	return battleResult;
}

bool Player::currentWeaponIs(common_defs::tokens weapon) {
	return equippedWeapon.getAssociatedToken() == weapon;
}

bool Player::hasWeaponEquipped() {
	return hasEquippedWeapon;
}

string Player::pickWeapon(common_defs::tokens item) {
	Weapon pickedWeapon = currentRoom->takeWeapon(item);

	if (hasEquippedWeapon) {
		dropCurrentWeapon();
	}

	hasEquippedWeapon = true;
	equippedWeapon = pickedWeapon;
	return pickedWeapon.getName();
}

void Player::pickWeapon(Weapon weapon) {

	if (hasEquippedWeapon) {
		dropCurrentWeapon();
	}

	hasEquippedWeapon = true;
	equippedWeapon = weapon;
}

void Player::dropCurrentWeapon() {
	currentRoom->putWeapon(equippedWeapon);
	hasEquippedWeapon = false;
}

bool Player::canSeeLockedDoor() {
	return currentRoom->hasLockedDoor();
}

void Player::openLockedDoor() {
	currentRoom->openLockedDoor();

	list<Item>::iterator it;
	for (it = inventory.begin(); it != inventory.end(); ++it) {
		if (it->getAssociatedToken() == common_defs::KEY) {
			inventory.erase(it);
			return;
		}
	}
}

bool Player::canFindContainer(common_defs::tokens container) {
	return currentRoom->containsContainer(container);
}

string Player::putItemIntoContainer(common_defs::tokens item, common_defs::tokens container) {
	Item requestedItem;
	std::list<Item>::iterator it;
	for (it = inventory.begin(); it != inventory.end(); ++it) {
		if (it->getAssociatedToken() == item) {
			requestedItem = *it;
			inventory.erase(it);

			if (item == common_defs::CURSED_APPLE & container == common_defs::CHALICE) {
				BlessedAppleAction* blessedAppleEvent = new BlessedAppleAction();
				blessedAppleEvent->execute();
				string output = blessedAppleEvent->getGeneratedOutput();
				delete blessedAppleEvent;
				return output;
			}
			else {
				Container *c = currentRoom->getContainer(container);
				c->insertItem(requestedItem);
				return "You put the " + requestedItem.getName() + " into the " + c->getName() + ".";
			}
		}
	}

}