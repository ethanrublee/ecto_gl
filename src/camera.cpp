// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#include "camera.h"

//#include "gpuhelper.h"
//#include <GL/glu.h>

#include "Eigen/LU"
using namespace Eigen;
namespace ecto_gl
{
  Camera::Camera()
      :
        mVpX(),
        mVpY(),
        mVpWidth(),
        mVpHeight(),
        mViewIsUptodate(false),
        mProjIsUptodate(false),
        mFovY(M_PI / 3.f),
        mNearDist(0.1f),
        mFarDist(100.f)
  {
    setPosition(Vector3f::Constant(100.f));
    setTarget(Vector3f::Zero());
  }

  Camera&
  Camera::operator=(const Camera& other)
  {
    if(this == &other)
      return *this;

    mViewIsUptodate = false;
    mProjIsUptodate = false;

    mVpX = other.mVpX;
    mVpY = other.mVpY;
    mVpWidth = other.mVpWidth;
    mVpHeight = other.mVpHeight;

    mFrame = other.mFrame;

    mTarget = other.mTarget;
    mFovY = other.mFovY;
    mNearDist = other.mNearDist;
    mFarDist = other.mFarDist;

    mViewMatrix = other.mViewMatrix;
    mProjectionMatrix = other.mProjectionMatrix;
    return *this;
  }

  Camera::Camera(const Camera& rhs)
  :
          mVpX(rhs.mVpX),
          mVpY(rhs.mVpY),
          mVpWidth(rhs.mVpWidth),
          mVpHeight(rhs.mVpHeight),
          mFrame(rhs.mFrame),
          mViewIsUptodate(rhs.mViewIsUptodate),
          mProjIsUptodate(rhs.mProjIsUptodate),
          mFovY(rhs.mFovY),
          mNearDist(rhs.mNearDist),
          mFarDist(rhs.mFarDist),
          mViewMatrix(rhs.mViewMatrix),
          mProjectionMatrix(rhs.mProjectionMatrix)
  {
  }

  Camera::~Camera()
  {
  }

  void
  Camera::setViewport(uint offsetx, uint offsety, uint width, uint height)
  {
    mVpX = offsetx;
    mVpY = offsety;
    mVpWidth = width;
    mVpHeight = height;

    mProjIsUptodate = false;
  }

  void
  Camera::setViewport(uint width, uint height)
  {
    mVpWidth = width;
    mVpHeight = height;

    mProjIsUptodate = false;
  }

  void
  Camera::setFovY(float value)
  {
    mFovY = value;
    mProjIsUptodate = false;
  }

  Vector3f
  Camera::direction(void) const
  {
    return -(orientation() * Vector3f::UnitZ());
  }
  Vector3f
  Camera::up(void) const
  {
    return orientation() * Vector3f::UnitY();
  }
  Vector3f
  Camera::right(void) const
  {
    return orientation() * Vector3f::UnitX();
  }

  void
  Camera::setDirection(const Vector3f& newDirection)
  {
    // TODO implement it computing the rotation between newDirection and current dir ?
    Vector3f up = this->up();

    Matrix3f camAxes;

    camAxes.col(2) = (-newDirection).normalized();
    camAxes.col(0) = up.cross(camAxes.col(2)).normalized();
    camAxes.col(1) = camAxes.col(2).cross(camAxes.col(0)).normalized();
    setOrientation(Quaternionf(camAxes));

    mViewIsUptodate = false;
  }

  void
  Camera::setTarget(const Vector3f& target)
  {
    mTarget = target;
    if (!mTarget.isApprox(position()))
    {
      Vector3f newDirection = mTarget - position();
      setDirection(newDirection.normalized());
    }
  }

  void
  Camera::setPosition(const Vector3f& p)
  {
    mFrame.position = p;
    mViewIsUptodate = false;
  }

  void
  Camera::setOrientation(const Quaternionf& q)
  {
    mFrame.orientation = q;
    mViewIsUptodate = false;
  }

  void
  Camera::setFrame(const Frame& f)
  {
    mFrame = f;
    mViewIsUptodate = false;
  }

  void
  Camera::rotateAroundTarget(const Quaternionf& q)
  {
    Matrix4f mrot, mt, mtm;

    // update the transform matrix
    updateViewMatrix();
    Vector3f t = mViewMatrix * mTarget;

    mViewMatrix = Translation3f(t) * q * Translation3f(-t) * mViewMatrix;

    Quaternionf qa(mViewMatrix.linear());
    qa = qa.conjugate();
    setOrientation(qa);
    setPosition(-(qa * mViewMatrix.translation()));

    mViewIsUptodate = true;
  }

  void
  Camera::localRotate(const Quaternionf& q)
  {
    float dist = (position() - mTarget).norm();
    setOrientation(orientation() * q);
    mTarget = position() + dist * direction();
    mViewIsUptodate = false;
  }

  void
  Camera::zoom(float d)
  {
    float dist = (position() - mTarget).norm();
    if (dist > d)
    {
      setPosition(position() + direction() * d);
      mViewIsUptodate = false;
    }
  }

  void
  Camera::localTranslate(const Vector3f& t)
  {
    Vector3f trans = orientation() * t;
    setPosition(position() + trans);
    setTarget(mTarget + trans);

    mViewIsUptodate = false;
  }

  void
  Camera::updateViewMatrix(void) const
  {
    if (!mViewIsUptodate)
    {
      Quaternionf q = orientation().conjugate();
      mViewMatrix.linear() = q.toRotationMatrix();
      mViewMatrix.translation() = -(mViewMatrix.linear() * position());

      mViewIsUptodate = true;
    }
  }

  const Affine3f&
  Camera::viewMatrix(void) const
  {
    updateViewMatrix();
    return mViewMatrix;
  }

  void
  Camera::updateProjectionMatrix(void) const
  {
    if (!mProjIsUptodate)
    {
      mProjectionMatrix.setIdentity();
      float aspect = float(mVpWidth) / float(mVpHeight);
      float theta = mFovY * 0.5;
      float range = mFarDist - mNearDist;
      float invtan = 1. / tan(theta);

      mProjectionMatrix(0, 0) = invtan / aspect;
      mProjectionMatrix(1, 1) = invtan;
      mProjectionMatrix(2, 2) = -(mNearDist + mFarDist) / range;
      mProjectionMatrix(3, 2) = -1;
      mProjectionMatrix(2, 3) = -2 * mNearDist * mFarDist / range;
      mProjectionMatrix(3, 3) = 0;

      mProjIsUptodate = true;
    }
  }

  const Matrix4f&
  Camera::projectionMatrix(void) const
  {
    updateProjectionMatrix();
    return mProjectionMatrix;
  }
#if 0
  void Camera::activateGL(void)
  {
    glViewport(vpX(), vpY(), vpWidth(), vpHeight());
    gpu.loadMatrix(projectionMatrix(),GL_PROJECTION);
    gpu.loadMatrix(viewMatrix().matrix(),GL_MODELVIEW);
  }
#endif

  Vector3f
  Camera::unProject(const Vector2f& uv, float depth) const
  {
    Matrix4f inv = mViewMatrix.inverse().matrix();
    return unProject(uv, depth, inv);
  }

  Vector3f
  Camera::unProject(const Vector2f& uv, float depth, const Matrix4f& invModelview) const
  {
    updateViewMatrix();
    updateProjectionMatrix();

    Vector3f a(2. * uv.x() / float(mVpWidth) - 1., 2. * uv.y() / float(mVpHeight) - 1., 1.);
    a.x() *= depth / mProjectionMatrix(0, 0);
    a.y() *= depth / mProjectionMatrix(1, 1);
    a.z() = -depth;
    // FIXME /\/|
    Vector4f b = invModelview * Vector4f(a.x(), a.y(), a.z(), 1.);
    return Vector3f(b.x(), b.y(), b.z());
  }
}
