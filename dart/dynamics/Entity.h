/*
 * Copyright (c) 2014-2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Michael X. Grey <mxgrey@gatech.edu>
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

#ifndef DART_DYNAMICS_ENTITY_H_
#define DART_DYNAMICS_ENTITY_H_

#include <Eigen/Core>
#include <string>
#include <vector>

#include "dart/common/Signal.h"
#include "dart/dynamics/Shape.h"

namespace dart {
namespace renderer {
class RenderInterface;
} // namespace renderer
} // namespace dart

namespace dart {
namespace dynamics {

class Frame;
class Shape;

/// Entity class is a base class for any objects that exist in the kinematic
/// tree structure of DART.
///
/// Entities all share the following properties: they exist within a reference
/// frame, have a name, and are visualizable. However, different Entity types
/// may have different policies about how/if their reference frame or name
/// can be changed. Use the Detachable class to create an Entity whose reference
/// Frame can be changed arbitrarily.
class Entity
{
public:
  friend class Frame;

  using EntitySignal = common::Signal<void(const Entity*)>;
  using FrameChangedSignal
      = common::Signal<void(const Entity*,
                            const Frame* _oldFrame,
                            const Frame* _newFrame)>;
  using NameChangedSignal
      = common::Signal<void(const Entity*,
                            const std::string& _oldName,
                            const std::string& _newName)>;
  using VizShapeAddedSignal
      = common::Signal<void(const Entity*, const Shape* _newVisShape)>;

  /// Constructor for typical usage
  explicit Entity(Frame* _refFrame, const std::string& _name, bool _quiet);

  /// Destructor
  virtual ~Entity();

  /// Set name. Some implementations of Entity may make alterations to the name
  /// that gets passed in. The final name that this entity will use gets passed
  /// back in the return of this function.
  virtual const std::string& setName(const std::string& _name);

  /// Return the name of this Entity
  virtual const std::string& getName() const;

  /// Add a visualization shape for this Entity
  virtual void addVisualizationShape(Shape* _p);

  /// Render this Entity
  virtual void draw(renderer::RenderInterface* _ri = NULL,
                    const Eigen::Vector4d& _color = Eigen::Vector4d::Ones(),
                    bool _useDefaultColor = true, int _depth = 0) const;

  /// Get the parent (reference) frame of this Entity
  Frame* getParentFrame();

  /// Get the parent (reference) frame of this Entity
  const Frame* getParentFrame() const;

  /// True iff this Entity depends on (i.e. kinematically descends from)
  /// _someFrame. If _someFrame is NULL, this returns false.
  bool descendsFrom(const Frame* _someFrame) const;

  /// Returns true if this Entity is set to be quiet.
  ///
  /// A quiet entity is unknown to its parent Frame. It will not be tracked by
  /// its parent; it will not receive notifications from its parent, and it will
  /// not be rendered. The advantage to a quiet Entity is that it has less
  /// overhead when constructing and deconstructing, which makes it more
  /// suitable for temporary objects.
  bool isQuiet() const;

  /// Notify this Entity that its parent Frame's pose has changed
  virtual void notifyTransformUpdate();

  /// Returns true iff a transform update is needed for this Entity
  bool needsTransformUpdate() const;

  /// Notify this Entity that its parent Frame's velocity has changed
  virtual void notifyVelocityUpdate();

  /// Returns true iff a velocity update is needed for this Entity
  bool needsVelocityUpdate() const;

  /// Notify this Entity that its parent Frame's acceleration has changed
  virtual void notifyAccelerationUpdate();

  /// Returns true iff an acceleration update is needed for this Entity
  bool needsAccelerationUpdate() const;

protected:
  /// Used by derived classes to change their parent frames
  virtual void changeParentFrame(Frame* _newParentFrame);

protected:
  /// Parent frame of this Entity
  Frame* mParentFrame;

  /// Name of this Entity
  std::string mName;

  /// Vector of visualization shapes
  std::vector<Shape*> mVizShapes;

  /// Does this Entity need a Transform update
  mutable bool mNeedTransformUpdate;

  /// Does this Entity need a Velocity update
  mutable bool mNeedVelocityUpdate;

  /// Does this Entity need an Acceleration update
  mutable bool mNeedAccelerationUpdate;

  /// Frame changed signal
  FrameChangedSignal mFrameChangedSignal;

  /// Name changed signal
  NameChangedSignal mNameChangedSignal;

  /// Visualization added signal
  VizShapeAddedSignal mVizShapeAddedSignal;

  /// Transform changed signal
  EntitySignal mTransformUpdatedSignal;

  /// Velocity changed signal
  EntitySignal mVelocityChangedSignal;

  /// Acceleration changed signal
  EntitySignal mAccelerationChangedSignal;

public:
  //----------------------------------------------------------------------------
  /// \{ \name Slot registers
  //----------------------------------------------------------------------------

  /// Slot register for frame changed signal
  common::SlotRegister<FrameChangedSignal> onFrameChanged;

  /// Slot register for name changed signal
  common::SlotRegister<NameChangedSignal> onNameChanged;

  /// Slot register for visualization changed signal
  common::SlotRegister<VizShapeAddedSignal> onVizShapeAdded;

  /// Slot register for transform updated signal
  common::SlotRegister<EntitySignal> onTransformUpdated;

  /// Slot register for velocity updated signal
  common::SlotRegister<EntitySignal> onVelocityChanged;

  /// Slot register for acceleration updated signal
  common::SlotRegister<EntitySignal> onAccelerationChanged;

  /// \}

private:
  /// Whether or not this Entity is set to be quiet
  const bool mAmQuiet;

};

/// The Detachable class is a special case of the Entity base class. Detachable
/// allows the Entity's reference Frame to be changed arbitrarily by the user.
class Detachable : public virtual Entity
{
public:
  /// Constructor
  explicit Detachable(Frame* _refFrame, const std::string& _name, bool _quiet);

  /// Allows the user to change the parent Frame of this Entity
  virtual void setParentFrame(Frame* _newParentFrame);

};

} // namespace dynamics
} // namespace dart

#endif // DART_DYNAMICS_ENTITY_H_
