#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Maths
{
	class Vector3;
	class Quaternion
	{

	public:
													Quaternion();
													Quaternion(const float& x, const float& y, const float& z, const float& w);
													Quaternion(const Vector3& vector, float w);
		Quaternion									operator*(const Quaternion& other) const;

		void										Set(const XMFLOAT4& quaternion);
		void										Set(const Quaternion& quaternion);
		void										Set(const float& x, const float& y, const float& z, const float& w);

		/*
		* mQuaternion�� ���� �����ϰ� �״��� quaternion�� �����Ѵ�.
		* ���� mul�� quaternion * mQuaternion���� ����Ѵ�.
		*/
		void										operator*=(const XMFLOAT4& quaternion);
		void										operator*=(const Quaternion& quaternion);
		void										Mul(const float& x, const float& y, const float& z, const float& w);

		Quaternion									operator+(const Quaternion& other);

		void										Normalize();

		Quaternion									Conjugate() const;

		const XMFLOAT4&								Get() const;

	protected:
		XMFLOAT4									mQuaternion;
	};
}
