#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Maths
{
	class Vector3
	{
	public:
		Vector3()
		{
			this->v.x = 0.0f;
			this->v.y = 0.0f;
			this->v.z = 0.0f;
		}
		Vector3(const float& x, const float& y, const float& z)
		{
			this->v.x = x;
			this->v.y = y;
			this->v.z = z;
		}
		Vector3(const XMFLOAT3& value)
		{
			this->v.x = value.x;
			this->v.y = value.y;
			this->v.z = value.z;
		}

		Vector3										operator+(const Vector3& other) const
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMLoadFloat3(&v) + XMLoadFloat3(&other.v));

			return result;
		}
		Vector3										operator-(const Vector3& other) const
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMLoadFloat3(&v) - XMLoadFloat3(&other.v));

			return result;
		}
		Vector3										operator-() const
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMLoadFloat3(&v) * -1.0f);

			return result;
		}
		Vector3										operator*(const float& c) const
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMLoadFloat3(&v) * c);

			return result;
		}
		float										operator*(const Vector3& other) const
		{
			float result;

			result = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&v), XMLoadFloat3(&other.v)));

			return result;
		}
		Vector3										operator*(const XMFLOAT3X3& m) const
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMVector3Transform(XMLoadFloat3(&v), XMLoadFloat3x3(&m)));

			return result;
		}
		Vector3										operator/(const float& c) const
		{
			if (fabs(c) < 0.00001f)
				return Vector3(0.0f, 0.0f, 0.0f);

			Vector3 result;

			XMStoreFloat3(&result.v, XMLoadFloat3(&v) / c);

			return result;
		}
		/*
		* cross product
		*/
		Vector3										operator^(const Vector3& other) const
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMVector3Cross(XMLoadFloat3(&v), XMLoadFloat3(&other.v)));

			return result;
		}
		float										length() const
		{
			return sqrt((v.x * v.x )+ (v.y * v.y) + (v.z * v.z));
		}
		Vector3										normalize()
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMVector3Normalize(XMLoadFloat3(&v)));

			return result;
		}
		bool										Zero()
		{
			if (v.x == 0 && v.y == 0 && v.z == 0)
				return true;
			else
				return false;
		}
		bool										operator==(const Vector3& other)
		{
			if (v.x == other.v.x && v.y == other.v.y && v.z == other.v.z)
				return true;
			else
				return false;
		}
		static float								Dot(const Vector3& a, const Vector3& b)
		{
			float result;

			result = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&a.v), XMLoadFloat3(&b.v)));

			return result;
		}
		static Vector3								Cross(const Vector3& a, const Vector3& b)
		{
			Vector3 result;

			XMStoreFloat3(&result.v, XMVector3Cross(XMLoadFloat3(&a.v), XMLoadFloat3(&b.v)));

			return result;
		}
		inline void operator-=(const Vector3& a) {
			v.x -= a.v.x;
			v.y -= a.v.y;
			v.z -= a.v.z;
		}

		XMFLOAT3									v;
	};

}
