/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_MATH_TRIANGLE
#define CHECKHEADER_SLIB_MATH_TRIANGLE

#include "definition.h"

#include "point.h"
#include "matrix3.h"

namespace slib
{
	
	template <class T>
	class SLIB_EXPORT TriangleT
	{
	public:
		PointT<T> point1;
		PointT<T> point2;
		PointT<T> point3;

	public:
		TriangleT() = default;

		TriangleT(const TriangleT<T>& other) = default;

		template <class O>
		TriangleT(const TriangleT<O>& other);

		TriangleT(const PointT<T>& point1, const PointT<T>& point2, const PointT<T>& point3);

	public:
		static T getCross(const PointT<T>& point1, const PointT<T>& point2, const PointT<T>& point3);

		T getCross() const;

		T getSize() const;

		void transform(Matrix3T<T>& mat);

	public:
		TriangleT<T>& operator=(const TriangleT<T>& other) = default;

		template <class O>
		TriangleT<T>& operator=(const TriangleT<O>& other);
	
	};
	
	extern template class TriangleT<float>;
	extern template class TriangleT<double>;
	typedef TriangleT<sl_real> Triangle;
	typedef TriangleT<float> Trianglef;
	typedef TriangleT<double> Trianglelf;

}

#include "detail/triangle.inc"

#endif
