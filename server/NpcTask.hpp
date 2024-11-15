#pragma once

#include <variant>

enum NpcMoveMode {
  NpcMoveMode_Walk,
  NpcMoveMode_Run,
  NpcMoveMode_Sprint
};

template <int TskId>
struct NpcTask {
  static constexpr const int TaskId = TskId;

  void writeInternal(NetworkBitStream& bs) const {
    bs.writeUINT8(TaskId);
    write(bs);
  }

  virtual void write(NetworkBitStream& bs) const = 0;
  virtual bool operator==(const NpcTask& other) const = 0;
  bool operator!=(const NpcTask& other) const {
    return !(*this == other);
  }
};

struct NpcTaskStandStill final : NpcTask<0> {
  void write(NetworkBitStream& bs) const override {
    // Nothing to do
  }

  bool operator==(const NpcTask& other) const override {
    const auto other_ = dynamic_cast<const NpcTaskStandStill*>(&other);
    return other_ != nullptr;
  }
};

struct NpcTaskAttackPlayer final : NpcTask<1> {
  const IPlayer* target = nullptr;
  bool aggressive = false;

  void write(NetworkBitStream& bs) const override {
    bs.writeUINT16(target->getID());
    bs.writeUINT8(aggressive ? 1 : 0);
  }

  bool operator==(const NpcTask& other) const override {
    const auto other_ = dynamic_cast<const NpcTaskAttackPlayer*>(&other);
    return other_ != nullptr && target == other_->target && aggressive == other_->aggressive;
  }
};

struct NpcTaskGoToPoint final : NpcTask<2> {
  Vector3 destination;
  NpcMoveMode mode;

  void write(NetworkBitStream& bs) const override {
    bs.writeVEC3(destination);
    bs.writeUINT8(int(mode));
  }

  bool operator==(const NpcTask& other) const override {
    const auto other_ = dynamic_cast<const NpcTaskGoToPoint*>(&other);
    return other_ != nullptr && destination == other_->destination && mode == other_->mode;
  }
};

struct NpcTaskFollowPlayer final : NpcTask<3> {
  const IPlayer* target = nullptr;

  void write(NetworkBitStream& bs) const override {
    bs.writeUINT16(target->getID());
  }

  bool operator==(const NpcTask& other) const override {
    const auto other_ = dynamic_cast<const NpcTaskFollowPlayer*>(&other);
    return other_ != nullptr && target == other_->target;
  }
};

struct NpcTaskPlayAnimation final : NpcTask<4> {
  AnimationData data;

  void write(NetworkBitStream& bs) const override {
    bs.writeDynStr8(data.lib);
    bs.writeDynStr8(data.name);
    bs.writeFLOAT(data.delta);
    bs.writeUINT8(data.loop);
    bs.writeUINT8(data.lockX);
    bs.writeUINT8(data.lockY);
    bs.writeUINT8(data.freeze);
    bs.writeUINT32(data.time);
  }

  bool operator==(const NpcTask& other) const override {
    const auto other_ = dynamic_cast<const NpcTaskPlayAnimation*>(&other);
    return other_ != nullptr
      && data.delta == other_->data.delta
      && data.loop == other_->data.loop
      && data.lockX == other_->data.lockX
      && data.lockY == other_->data.lockY
      && data.freeze == other_->data.freeze
      && data.time == other_->data.time
      && data.lib == other_->data.lib
      && data.name == other_->data.name;
  }
};

using NpcTasksSet = std::variant<
    NpcTaskStandStill,
    NpcTaskAttackPlayer,
    NpcTaskGoToPoint,
    NpcTaskFollowPlayer,
    NpcTaskPlayAnimation
>;
