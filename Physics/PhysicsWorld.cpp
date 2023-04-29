#include "PhysicsWorld.h"
#include <random>

using namespace Physics;

PhysicsWorld::PhysicsWorld() {

	shuffleConstraints = false;
	shuffleObjects = false;
	worldIDCounter = 0;
}

PhysicsWorld::~PhysicsWorld() {
}

void PhysicsWorld::Clear() {
	gameObjects.clear();
	constraints.clear();
}

void PhysicsWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	for (auto& i : constraints) {
		delete i;
	}
	Clear();
}

void PhysicsWorld::AddGameObject(PhysicsObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
}

void PhysicsWorld::RemoveGameObject(PhysicsObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete) {
		delete o;
	}
}

void PhysicsWorld::GetObjectIterators(
	PhysicsObjectIterator& first,
	PhysicsObjectIterator& last) const {

	first = gameObjects.begin();
	last = gameObjects.end();
}

void PhysicsWorld::OperateOnContents(PhysicsObjectFunc f) {
	for (PhysicsObject* g : gameObjects) {
		f(g);
	}
}

void PhysicsWorld::UpdateWorld(float dt) {
	if (shuffleObjects) {
		std::random_shuffle(gameObjects.begin(), gameObjects.end());
	}

	if (shuffleConstraints) {
		std::random_shuffle(constraints.begin(), constraints.end());
	}
}

/*
Constraint Tutorial Stuff
*/

void PhysicsWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void PhysicsWorld::RemoveConstraint(Constraint* c, bool andDelete) {
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
	if (andDelete) {
		delete c;
	}
}

void PhysicsWorld::GetConstraintIterators(
	std::vector<Constraint*>::const_iterator& first,
	std::vector<Constraint*>::const_iterator& last) const {
	first = constraints.begin();
	last = constraints.end();
}