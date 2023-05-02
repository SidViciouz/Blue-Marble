#pragma once

namespace Maths
{
	class Quaternion;
	class Vector3;
	class Vector4;
	class Matrix3x3;

	class Matrix4x4 {
	public:
												Matrix4x4(void);
												Matrix4x4(float elements[16]);
												Matrix4x4(const Matrix3x3& m3);
												Matrix4x4(const Quaternion& quat);
												~Matrix4x4(void);

		float									array[16];

		void									ToZero();

		Vector3									GetPositionVector() const;

		void									SetPositionVector(const Vector3& in);

		Vector3									GetDiagonal() const;

		void									SetDiagonal(const Vector3& in);

		static Matrix4x4						Rotation(float degrees, const Vector3& axis);

		static Matrix4x4						Scale(const Vector3& scale);

		static Matrix4x4						Translation(const Vector3& translation);

		static Matrix4x4						Perspective(float znear, float zfar, float aspect, float fov);

		static Matrix4x4						Orthographic(float znear, float zfar, float right, float left, float top, float bottom);

		static Matrix4x4						BuildViewMatrix(const Vector3& from, const Vector3& lookingAt, const Vector3& up);

		void									Invert();
		Matrix4x4								Inverse() const;

		Matrix4x4								Transpose() const;

		Vector4									GetRow(unsigned int row) const;
		Vector4									GetColumn(unsigned int column) const;

		inline Matrix4x4						operator*(const Matrix4x4& a) const
		{
			Matrix4x4 out;
			for (unsigned int r = 0; r < 4; ++r) {
				for (unsigned int c = 0; c < 4; ++c) {
					out.array[c + (r * 4)] = 0.0f;
					for (unsigned int i = 0; i < 4; ++i) {
						out.array[c + (r * 4)] += this->array[c + (i * 4)] * a.array[(r * 4) + i];
					}
				}
			}
			return out;
		}


		inline Matrix4x4						operator*(const float a) const
		{
			Matrix4x4 out;
			for (unsigned int r = 0; r < 4; ++r) {
				for (unsigned int c = 0; c < 4; ++c) {
					out.array[c + (r * 4)] *= a;
				}
			}
			return out;
		}

		Vector3									operator*(const Vector3& v) const;
		Vector4									operator*(const Vector4& v) const;

	};
}
