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
}

void PhysicsWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
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

}
