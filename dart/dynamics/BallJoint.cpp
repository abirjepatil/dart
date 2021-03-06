/*
 * Copyright (c) 2013-2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Jeongseok Lee <jslee02@gmail.com>
 *
 * Georgia Tech Graphics Lab and Humanoid Robotics Lab
 *
 * Directed by Prof. C. Karen Liu and Prof. Mike Stilman
 * <karenliu@cc.gatech.edu> <mstilman@cc.gatech.edu>
 *
 * This file is provided under the following "BSD-style" License:
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

#include "dart/dynamics/BallJoint.h"

#include <string>

#include "dart/math/Helpers.h"
#include "dart/math/Geometry.h"

namespace dart {
namespace dynamics {

//==============================================================================
BallJoint::BallJoint(const std::string& _name)
  : MultiDofJoint(_name),
    mR(Eigen::Isometry3d::Identity())
{
  updateDegreeOfFreedomNames();
}

//==============================================================================
BallJoint::~BallJoint()
{
}

//==============================================================================
Eigen::Isometry3d BallJoint::convertToTransform(
    const Eigen::Vector3d& _positions)
{
  return Eigen::Isometry3d(convertToRotation(_positions));
}

//==============================================================================
Eigen::Matrix3d BallJoint::convertToRotation(const Eigen::Vector3d& _positions)
{
  return math::expMapRot(_positions);
}

//==============================================================================
void BallJoint::setPositionsStatic(const Eigen::Vector3d& _positions)
{
  mR = convertToRotation(_positions);

  MultiDofJoint::setPositionsStatic(_positions);
}

//==============================================================================
Eigen::Vector3d BallJoint::getPositionDifferencesStatic(
    const Eigen::Vector3d& _q0, const Eigen::Vector3d& _q1) const
{
  Eigen::Vector3d dq;

  const Eigen::Matrix3d Jw  = getLocalJacobianStatic(_q0).topRows<3>();
  const Eigen::Matrix3d R0T = math::expMapRot(-_q0);
  const Eigen::Matrix3d R1  = math::expMapRot( _q1);

  dq = Jw.inverse() * math::logMap(R0T * R1);

  return dq;
}

//==============================================================================
Eigen::Matrix<double, 6, 3> BallJoint::getLocalJacobianStatic(
    const Eigen::Vector3d& _positions) const
{
  // Jacobian expressed in the Joint frame
  Eigen::Matrix<double, 6, 3> J;
  J.topRows<3>()    = math::expMapJac(-_positions);
  J.bottomRows<3>() = Eigen::Matrix3d::Zero();

  // Transform the reference frame to the child BodyNode frame
  J = math::AdTJacFixed(mT_ChildBodyToJoint, J);

  assert(!math::isNan(J));

  return J;
}

//==============================================================================
void BallJoint::integratePositions(double _dt)
{
  mR.linear() = mR.linear()
      * convertToRotation(getLocalJacobianStatic().topRows<3>()
                          * getVelocitiesStatic() * _dt);

  setPositionsStatic(convertToPositions(mR.linear()));
}

//==============================================================================
void BallJoint::updateDegreeOfFreedomNames()
{
  if(!mDofs[0]->isNamePreserved())
    mDofs[0]->setName(mName + "_x", false);
  if(!mDofs[1]->isNamePreserved())
    mDofs[1]->setName(mName + "_y", false);
  if(!mDofs[2]->isNamePreserved())
    mDofs[2]->setName(mName + "_z", false);
}

//==============================================================================
void BallJoint::updateLocalTransform() const
{
  mR.linear() = convertToRotation(getPositionsStatic());

  mT = mT_ParentBodyToJoint * mR * mT_ChildBodyToJoint.inverse();

  assert(math::verifyTransform(mT));
}

//==============================================================================
void BallJoint::updateLocalJacobian(bool) const
{
  mJacobian = getLocalJacobian(getPositionsStatic());
}

//==============================================================================
void BallJoint::updateLocalJacobianTimeDeriv() const
{
  Eigen::Matrix<double, 6, 3> dJ;
  dJ.topRows<3>()    = math::expMapJacDot(getPositionsStatic(),
                                          getVelocitiesStatic()).transpose();
  dJ.bottomRows<3>() = Eigen::Matrix3d::Zero();

  mJacobianDeriv = math::AdTJacFixed(mT_ChildBodyToJoint, dJ);

  assert(!math::isNan(mJacobianDeriv));
}

}  // namespace dynamics
}  // namespace dart

