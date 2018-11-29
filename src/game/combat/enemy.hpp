#pragma once

#include "../../engine/core.hpp"
#include "../../math/vec.hpp"
#include "../../engine/animatedSprite.hpp"

#include "unit.hpp"
#include "attack.hpp"

#define ENEMY_DEFAULT_MOVE_COUNTER		20
#define ENEMY_DEFAULT_ATTACK_COUNTER	20

class Enemy : public Unit {

public:

	Enemy();
	Enemy(UnitType type, const std::string& spritePath);
	~Enemy();

	virtual void render() override;
	virtual void update(int delta);

	void takeTurn();

protected:
	
	// Helper method to handle the movement portion of an enemy turn
	virtual void handleMovement();
	// Helper method to handle the attack portion of an enemy turn
	virtual void handleAttack();

	// Override callback function to customize functionality
	void takeDamageCallback(int damage) override;

	// Enemy sprite
	AnimatedSprite sprite;

private:

	// Basic bite attack of enemy
	Attack bite;
 
};