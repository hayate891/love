/**
 * Copyright (c) 2006-2016 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Transform.h"

namespace love
{
namespace math
{

Transform::Transform()
	: matrix()
	, inverseDirty(true)
	, inverseMatrix()
{
}

Transform::Transform(const Matrix4 &m)
	: matrix(m)
	, inverseDirty(true)
	, inverseMatrix()
{
}

Transform::Transform(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky)
	: matrix(x, y, a, sx, sy, ox, oy, kx, ky)
	, inverseDirty(true)
	, inverseMatrix()
{
}

Transform::~Transform()
{
}

Transform *Transform::clone()
{
	return new Transform(*this);
}

Transform *Transform::inverse()
{
	return new Transform(getInverseMatrix());
}

void Transform::apply(Transform *other)
{
	matrix *= other->getMatrix();
	inverseDirty = true;
}

void Transform::translate(float x, float y)
{
	matrix.translate(x, y);
	inverseDirty = true;
}

void Transform::rotate(float angle)
{
	matrix.rotate(angle);
	inverseDirty = true;
}

void Transform::scale(float x, float y)
{
	matrix.scale(x, y);
	inverseDirty = true;
}

void Transform::shear(float x, float y)
{
	matrix.shear(x, y);
	inverseDirty = true;
}

void Transform::reset()
{
	matrix.setIdentity();
	inverseDirty = true;
}

void Transform::setTransformation(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky)
{
	matrix.setTransformation(x, y, a, sx, sy, ox, oy, kx, ky);
	inverseDirty = true;
}

love::Vector Transform::transformPoint(love::Vector p) const
{
	love::Vector result;
	matrix.transform(&result, &p, 1);
	return result;
}

love::Vector Transform::inverseTransformPoint(love::Vector p)
{
	love::Vector result;
	getInverseMatrix().transform(&result, &p, 1);
	return result;
}

const Matrix4 &Transform::getMatrix() const
{
	return matrix;
}

void Transform::setMatrix(const Matrix4 &m)
{
	matrix = m;
	inverseDirty = true;
}

} // math
} // love
