/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_MATH_VECTOR2
#define CHECKHEADER_SLIB_MATH_VECTOR2

#include "definition.h"

#include "../core/interpolation.h"

namespace slib
{
	
	template <class T, class FT = T>
	class SLIB_EXPORT Vector2T
	{
	public:
		T x;
		T y;
	
	public:
		SLIB_INLINE Vector2T() = default;

		constexpr Vector2T(const Vector2T<T, FT>& other) : x(other.x), y(other.y) {}
	
		template <class O, class FO>
		constexpr Vector2T(const Vector2T<O, FO>& other) : x((T)(other.x)), y((T)(other.y)) {}
	
		constexpr Vector2T(T _x, T _y) : x(_x), y(_y) {}
	
	public:
		static const Vector2T<T, FT>& zero();

		static const Vector2T<T, FT>& fromArray(const T arr[2]);

		static Vector2T<T, FT>& fromArray(T arr[2]);

		T dot(const Vector2T<T, FT>& other) const;

		T cross(const Vector2T<T, FT>& other) const;

		T getLength2p() const;

		FT getLength() const;

		T getLength2p(const Vector2T<T, FT>& other) const;

		FT getLength(const Vector2T<T, FT>& other) const;

		void normalize();

		Vector2T<T, FT> getNormalized();

		FT getCosBetween(const Vector2T<T, FT>& other) const;

		FT getAbsAngleBetween(const Vector2T<T, FT>& other) const;

		FT getAngleBetween(const Vector2T<T, FT>& other) const;

		sl_bool equals(const Vector2T<T, FT>& other) const;

		sl_bool isAlmostEqual(const Vector2T<T, FT>& other) const;

		Vector2T<T, FT> lerp(const Vector2T<T, FT>& target, float factor) const;
	
	public:
		Vector2T<T, FT>& operator=(const Vector2T<T, FT>& other) = default;

		template <class O, class FO>
		Vector2T<T, FT>& operator=(const Vector2T<O, FO>& other);

		Vector2T<T, FT> operator+(const Vector2T<T, FT>& other) const;

		Vector2T<T, FT>& operator+=(const Vector2T<T, FT>& other);

		Vector2T<T, FT> operator-(const Vector2T<T, FT>& other) const;

		Vector2T<T, FT>& operator-=(const Vector2T<T, FT>& other);

		Vector2T<T, FT> operator*(T f) const;

		Vector2T<T, FT>& operator*=(T f);

		Vector2T<T, FT> operator*(const Vector2T<T, FT>& other) const;

		Vector2T<T, FT>& operator*=(const Vector2T<T, FT>& other);

		Vector2T<T, FT> operator/(T f) const;

		Vector2T<T, FT>& operator/=(T f);

		Vector2T<T, FT> operator/(const Vector2T<T, FT>& other) const;

		Vector2T<T, FT>& operator/(const Vector2T<T, FT>& other);

		Vector2T<T, FT> operator-() const;

		sl_bool operator==(const Vector2T<T, FT>& other) const;

		sl_bool operator!=(const Vector2T<T, FT>& other) const;

	private:
		static T _zero[2];
	
	};
	
	
	extern template class Vector2T<float>;
	extern template class Vector2T<double>;
	extern template class Vector2T<sl_int32, float>;
	extern template class Vector2T<sl_int64, double>;
	typedef Vector2T<sl_real> Vector2;
	typedef Vector2T<float> Vector2f;
	typedef Vector2T<double> Vector2lf;
	typedef Vector2T<sl_int32, float> Vector2i;
	typedef Vector2T<sl_int64, double> Vector2li;
	
	template <class T, class FT>
	Vector2T<T, FT> operator*(T f, const Vector2T<T, FT>& v);
	
	template <class T, class FT>
	Vector2T<T, FT> operator/(T f, const Vector2T<T, FT>& v);
	
	template <class T, class FT>
	class Interpolation< Vector2T<T, FT> >
	{
	public:
		static Vector2T<T, FT> interpolate(const Vector2T<T, FT>& a, const Vector2T<T, FT>& b, float factor);
	};

}

#include "detail/vector2.inc"

#endif
