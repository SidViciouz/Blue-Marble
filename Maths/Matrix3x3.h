#pragma once

#include <cstdlib>

namespace Maths
{
	class Vector3;
	class Quaternion;

	class Matrix3x3
	{
	public:
												Matrix3x3(void);
												Matrix3x3(float elements[9]);
												Matrix3x3(const Quaternion& quat);

												~Matrix3x3(void);

		//Set all matrix values to zero
		void									ToZero();

		Vector3									GetRow(unsigned int row) const;
		void									SetRow(unsigned int row, const Vector3& val);

		Vector3									GetColumn(unsigned int column) const;
		void									SetColumn(unsigned int column, const Vector3& val);

		Vector3									GetDiagonal() const;
		void									SetDiagonal(const Vector3& in);

		Vector3									ToEuler() const;

		inline Matrix3x3						Absolute() const
		{
			Matrix3x3 m;

			for (int i = 0; i < 9; ++i) {
				m.array[i] = std::abs(array[i]);
			}

			return m;
		}

		inline Matrix3x3						Transposed() const
		{
			Matrix3x3 temp = *this;
			temp.Transpose();
			return temp;
		}

		inline void								Transpose()
		{
			float tempValues[3];

			tempValues[0] = array[3];
			tempValues[1] = array[6];
			tempValues[2] = array[7];

			array[3] = array[1];
			array[6] = array[2];
			array[7] = array[5];

			array[1] = tempValues[0];
			array[2] = tempValues[1];
			array[5] = tempValues[2];
		}

		Vector3									operator*(const Vector3& v) const;

		inline Matrix3x3						operator*(const Matrix3x3& a) const
		{
			Matrix3x3 out;
			for (unsigned int r = 0; r < 3; ++r) {
				for (unsigned int c = 0; c < 3; ++c) {
					out.array[c + (r * 3)] = 0.0f;
					for (unsigned int i = 0; i < 3; ++i) {
						out.array[c + (r * 3)] += this->array[c + (i * 3)] * a.array[(r * 3) + i];
					}
				}
			}
			return out;
		}


		inline Matrix3x3						operator*(const float a) const
		{
			Matrix3x3 out;
			for (unsigned int r = 0; r < 3; ++r) {
				for (unsigned int c = 0; c < 3; ++c) {
					out.array[c + (r * 3)] *= a;
				}
			}
			return out;
		}

		static Matrix3x3						Rotation(float degrees, const Vector3& axis);


		static Matrix3x3						Scale(const Vector3& scale);

		static Matrix3x3						FromEuler(const Vector3& euler);

		Matrix3x3								Inverse()
		{
			float a = array[0];
			float b = array[1];
			float c = array[2];
			float d = array[3];
			float e = array[4];
			float f = array[5];
			float g = array[6];
			float h = array[7];
			float i = array[8];
			

			float D = 1.0f/(a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h);

			float element[9] = 
			{
				D*(e*i-f*h), -D*(b*i -c*h), D*(b*f-c*e),
				-D*(d*i-f*g),D*(a*i-c*g),-D*(a*f-c*d),
				D*(d*h-e*g),-D*(a*h-b*g),D*(a*e-b*d)
			};

			Matrix3x3 inv(element);

			return inv;
		}

	public:
		float									array[9];
	};
}