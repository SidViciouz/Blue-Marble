#pragma once

namespace Physics
{
	class Constraint {
	public:
		Constraint() {}
		virtual ~Constraint() {}

		virtual void UpdateConstraint(float dt) = 0;
	};
}
