/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// WebAssembly AST visitor. Useful for anything that wants to do something
// different for each AST node type, like printing, interpreting, etc.
//
// This class is specifically designed as a template to avoid virtual function
// call overhead. To write a visitor, derive from this class as follows:
//
//   struct MyVisitor : public WasmVisitor<MyVisitor> { .. }
//

#ifndef wasm_wasm_traversal_h
#define wasm_wasm_traversal_h

#include "support/small_vector.h"
#include "support/threads.h"
#include "wasm.h"

namespace wasm {

// A generic visitor, defaulting to doing nothing on each visit

template<typename SubType, typename ReturnType = void> struct Visitor {
  // Expression visitors
#include "wasm-visitors.generated.h"

  // Module-level visitors
  ReturnType visitExport(Export* curr) { return ReturnType(); }
  ReturnType visitGlobal(Global* curr) { return ReturnType(); }
  ReturnType visitFunction(Function* curr) { return ReturnType(); }
  ReturnType visitTable(Table* curr) { return ReturnType(); }
  ReturnType visitMemory(Memory* curr) { return ReturnType(); }
  ReturnType visitEvent(Event* curr) { return ReturnType(); }
  ReturnType visitModule(Module* curr) { return ReturnType(); }

  ReturnType visit(Expression* curr) {
    assert(curr);

#define DELEGATE(CLASS_TO_VISIT)                                               \
  return static_cast<SubType*>(this)->visit##CLASS_TO_VISIT(                   \
    static_cast<CLASS_TO_VISIT*>(curr))

    switch (curr->_id) {
      case Expression::Id::BlockId:
        DELEGATE(Block);
      case Expression::Id::IfId:
        DELEGATE(If);
      case Expression::Id::LoopId:
        DELEGATE(Loop);
      case Expression::Id::BreakId:
        DELEGATE(Break);
      case Expression::Id::SwitchId:
        DELEGATE(Switch);
      case Expression::Id::CallId:
        DELEGATE(Call);
      case Expression::Id::CallIndirectId:
        DELEGATE(CallIndirect);
      case Expression::Id::LocalGetId:
        DELEGATE(LocalGet);
      case Expression::Id::LocalSetId:
        DELEGATE(LocalSet);
      case Expression::Id::GlobalGetId:
        DELEGATE(GlobalGet);
      case Expression::Id::GlobalSetId:
        DELEGATE(GlobalSet);
      case Expression::Id::LoadId:
        DELEGATE(Load);
      case Expression::Id::StoreId:
        DELEGATE(Store);
      case Expression::Id::AtomicRMWId:
        DELEGATE(AtomicRMW);
      case Expression::Id::AtomicCmpxchgId:
        DELEGATE(AtomicCmpxchg);
      case Expression::Id::AtomicWaitId:
        DELEGATE(AtomicWait);
      case Expression::Id::AtomicNotifyId:
        DELEGATE(AtomicNotify);
      case Expression::Id::AtomicFenceId:
        DELEGATE(AtomicFence);
      case Expression::Id::SIMDExtractId:
        DELEGATE(SIMDExtract);
      case Expression::Id::SIMDReplaceId:
        DELEGATE(SIMDReplace);
      case Expression::Id::SIMDShuffleId:
        DELEGATE(SIMDShuffle);
      case Expression::Id::SIMDTernaryId:
        DELEGATE(SIMDTernary);
      case Expression::Id::SIMDShiftId:
        DELEGATE(SIMDShift);
      case Expression::Id::SIMDLoadId:
        DELEGATE(SIMDLoad);
      case Expression::Id::MemoryInitId:
        DELEGATE(MemoryInit);
      case Expression::Id::DataDropId:
        DELEGATE(DataDrop);
      case Expression::Id::MemoryCopyId:
        DELEGATE(MemoryCopy);
      case Expression::Id::MemoryFillId:
        DELEGATE(MemoryFill);
      case Expression::Id::ConstId:
        DELEGATE(Const);
      case Expression::Id::UnaryId:
        DELEGATE(Unary);
      case Expression::Id::BinaryId:
        DELEGATE(Binary);
      case Expression::Id::SelectId:
        DELEGATE(Select);
      case Expression::Id::DropId:
        DELEGATE(Drop);
      case Expression::Id::ReturnId:
        DELEGATE(Return);
      case Expression::Id::MemorySizeId:
        DELEGATE(MemorySize);
      case Expression::Id::MemoryGrowId:
        DELEGATE(MemoryGrow);
      case Expression::Id::RefNullId:
        DELEGATE(RefNull);
      case Expression::Id::RefIsNullId:
        DELEGATE(RefIsNull);
      case Expression::Id::RefFuncId:
        DELEGATE(RefFunc);
      case Expression::Id::RefEqId:
        DELEGATE(RefEq);
      case Expression::Id::TryId:
        DELEGATE(Try);
      case Expression::Id::ThrowId:
        DELEGATE(Throw);
      case Expression::Id::RethrowId:
        DELEGATE(Rethrow);
      case Expression::Id::BrOnExnId:
        DELEGATE(BrOnExn);
      case Expression::Id::NopId:
        DELEGATE(Nop);
      case Expression::Id::UnreachableId:
        DELEGATE(Unreachable);
      case Expression::Id::PopId:
        DELEGATE(Pop);
      case Expression::Id::TupleMakeId:
        DELEGATE(TupleMake);
      case Expression::Id::TupleExtractId:
        DELEGATE(TupleExtract);
      case Expression::Id::I31NewId:
        DELEGATE(I31New);
      case Expression::Id::I31GetId:
        DELEGATE(I31Get);
      case Expression::Id::RefTestId:
        DELEGATE(RefTest);
      case Expression::Id::RefCastId:
        DELEGATE(RefCast);
      case Expression::Id::BrOnCastId:
        DELEGATE(BrOnCast);
      case Expression::Id::RttCanonId:
        DELEGATE(RttCanon);
      case Expression::Id::RttSubId:
        DELEGATE(RttSub);
      case Expression::Id::StructNewId:
        DELEGATE(StructNew);
      case Expression::Id::StructGetId:
        DELEGATE(StructGet);
      case Expression::Id::StructSetId:
        DELEGATE(StructSet);
      case Expression::Id::ArrayNewId:
        DELEGATE(ArrayNew);
      case Expression::Id::ArrayGetId:
        DELEGATE(ArrayGet);
      case Expression::Id::ArraySetId:
        DELEGATE(ArraySet);
      case Expression::Id::ArrayLenId:
        DELEGATE(ArrayLen);
      case Expression::Id::InvalidId:
      default:
        WASM_UNREACHABLE("unexpected expression type");
    }

#undef DELEGATE
  }
};

// A visitor which must be overridden for each visitor that is reached.

template<typename SubType, typename ReturnType = void>
struct OverriddenVisitor {
// Expression visitors, which must be overridden
#define UNIMPLEMENTED(CLASS_TO_VISIT)                                          \
  ReturnType visit##CLASS_TO_VISIT(CLASS_TO_VISIT* curr) {                     \
    static_assert(                                                             \
      &SubType::visit##CLASS_TO_VISIT !=                                       \
        &OverriddenVisitor<SubType, ReturnType>::visit##CLASS_TO_VISIT,        \
      "Derived class must implement visit" #CLASS_TO_VISIT);                   \
    WASM_UNREACHABLE("Derived class must implement visit" #CLASS_TO_VISIT);    \
  }

  UNIMPLEMENTED(Block);
  UNIMPLEMENTED(If);
  UNIMPLEMENTED(Loop);
  UNIMPLEMENTED(Break);
  UNIMPLEMENTED(Switch);
  UNIMPLEMENTED(Call);
  UNIMPLEMENTED(CallIndirect);
  UNIMPLEMENTED(LocalGet);
  UNIMPLEMENTED(LocalSet);
  UNIMPLEMENTED(GlobalGet);
  UNIMPLEMENTED(GlobalSet);
  UNIMPLEMENTED(Load);
  UNIMPLEMENTED(Store);
  UNIMPLEMENTED(AtomicRMW);
  UNIMPLEMENTED(AtomicCmpxchg);
  UNIMPLEMENTED(AtomicWait);
  UNIMPLEMENTED(AtomicNotify);
  UNIMPLEMENTED(AtomicFence);
  UNIMPLEMENTED(SIMDExtract);
  UNIMPLEMENTED(SIMDReplace);
  UNIMPLEMENTED(SIMDShuffle);
  UNIMPLEMENTED(SIMDTernary);
  UNIMPLEMENTED(SIMDShift);
  UNIMPLEMENTED(SIMDLoad);
  UNIMPLEMENTED(MemoryInit);
  UNIMPLEMENTED(DataDrop);
  UNIMPLEMENTED(MemoryCopy);
  UNIMPLEMENTED(MemoryFill);
  UNIMPLEMENTED(Const);
  UNIMPLEMENTED(Unary);
  UNIMPLEMENTED(Binary);
  UNIMPLEMENTED(Select);
  UNIMPLEMENTED(Drop);
  UNIMPLEMENTED(Return);
  UNIMPLEMENTED(MemorySize);
  UNIMPLEMENTED(MemoryGrow);
  UNIMPLEMENTED(RefNull);
  UNIMPLEMENTED(RefIsNull);
  UNIMPLEMENTED(RefFunc);
  UNIMPLEMENTED(RefEq);
  UNIMPLEMENTED(Try);
  UNIMPLEMENTED(Throw);
  UNIMPLEMENTED(Rethrow);
  UNIMPLEMENTED(BrOnExn);
  UNIMPLEMENTED(Nop);
  UNIMPLEMENTED(Unreachable);
  UNIMPLEMENTED(Pop);
  UNIMPLEMENTED(TupleMake);
  UNIMPLEMENTED(TupleExtract);
  UNIMPLEMENTED(I31New);
  UNIMPLEMENTED(I31Get);
  UNIMPLEMENTED(RefTest);
  UNIMPLEMENTED(RefCast);
  UNIMPLEMENTED(BrOnCast);
  UNIMPLEMENTED(RttCanon);
  UNIMPLEMENTED(RttSub);
  UNIMPLEMENTED(StructNew);
  UNIMPLEMENTED(StructGet);
  UNIMPLEMENTED(StructSet);
  UNIMPLEMENTED(ArrayNew);
  UNIMPLEMENTED(ArrayGet);
  UNIMPLEMENTED(ArraySet);
  UNIMPLEMENTED(ArrayLen);
  UNIMPLEMENTED(Export);
  UNIMPLEMENTED(Global);
  UNIMPLEMENTED(Function);
  UNIMPLEMENTED(Table);
  UNIMPLEMENTED(Memory);
  UNIMPLEMENTED(Event);
  UNIMPLEMENTED(Module);

#undef UNIMPLEMENTED

  ReturnType visit(Expression* curr) {
    assert(curr);

#define DELEGATE(CLASS_TO_VISIT)                                               \
  return static_cast<SubType*>(this)->visit##CLASS_TO_VISIT(                   \
    static_cast<CLASS_TO_VISIT*>(curr))

    switch (curr->_id) {
      case Expression::Id::BlockId:
        DELEGATE(Block);
      case Expression::Id::IfId:
        DELEGATE(If);
      case Expression::Id::LoopId:
        DELEGATE(Loop);
      case Expression::Id::BreakId:
        DELEGATE(Break);
      case Expression::Id::SwitchId:
        DELEGATE(Switch);
      case Expression::Id::CallId:
        DELEGATE(Call);
      case Expression::Id::CallIndirectId:
        DELEGATE(CallIndirect);
      case Expression::Id::LocalGetId:
        DELEGATE(LocalGet);
      case Expression::Id::LocalSetId:
        DELEGATE(LocalSet);
      case Expression::Id::GlobalGetId:
        DELEGATE(GlobalGet);
      case Expression::Id::GlobalSetId:
        DELEGATE(GlobalSet);
      case Expression::Id::LoadId:
        DELEGATE(Load);
      case Expression::Id::StoreId:
        DELEGATE(Store);
      case Expression::Id::AtomicRMWId:
        DELEGATE(AtomicRMW);
      case Expression::Id::AtomicCmpxchgId:
        DELEGATE(AtomicCmpxchg);
      case Expression::Id::AtomicWaitId:
        DELEGATE(AtomicWait);
      case Expression::Id::AtomicNotifyId:
        DELEGATE(AtomicNotify);
      case Expression::Id::AtomicFenceId:
        DELEGATE(AtomicFence);
      case Expression::Id::SIMDExtractId:
        DELEGATE(SIMDExtract);
      case Expression::Id::SIMDReplaceId:
        DELEGATE(SIMDReplace);
      case Expression::Id::SIMDShuffleId:
        DELEGATE(SIMDShuffle);
      case Expression::Id::SIMDTernaryId:
        DELEGATE(SIMDTernary);
      case Expression::Id::SIMDShiftId:
        DELEGATE(SIMDShift);
      case Expression::Id::SIMDLoadId:
        DELEGATE(SIMDLoad);
      case Expression::Id::MemoryInitId:
        DELEGATE(MemoryInit);
      case Expression::Id::DataDropId:
        DELEGATE(DataDrop);
      case Expression::Id::MemoryCopyId:
        DELEGATE(MemoryCopy);
      case Expression::Id::MemoryFillId:
        DELEGATE(MemoryFill);
      case Expression::Id::ConstId:
        DELEGATE(Const);
      case Expression::Id::UnaryId:
        DELEGATE(Unary);
      case Expression::Id::BinaryId:
        DELEGATE(Binary);
      case Expression::Id::SelectId:
        DELEGATE(Select);
      case Expression::Id::DropId:
        DELEGATE(Drop);
      case Expression::Id::ReturnId:
        DELEGATE(Return);
      case Expression::Id::MemorySizeId:
        DELEGATE(MemorySize);
      case Expression::Id::MemoryGrowId:
        DELEGATE(MemoryGrow);
      case Expression::Id::RefNullId:
        DELEGATE(RefNull);
      case Expression::Id::RefIsNullId:
        DELEGATE(RefIsNull);
      case Expression::Id::RefFuncId:
        DELEGATE(RefFunc);
      case Expression::Id::RefEqId:
        DELEGATE(RefEq);
      case Expression::Id::TryId:
        DELEGATE(Try);
      case Expression::Id::ThrowId:
        DELEGATE(Throw);
      case Expression::Id::RethrowId:
        DELEGATE(Rethrow);
      case Expression::Id::BrOnExnId:
        DELEGATE(BrOnExn);
      case Expression::Id::NopId:
        DELEGATE(Nop);
      case Expression::Id::UnreachableId:
        DELEGATE(Unreachable);
      case Expression::Id::PopId:
        DELEGATE(Pop);
      case Expression::Id::TupleMakeId:
        DELEGATE(TupleMake);
      case Expression::Id::TupleExtractId:
        DELEGATE(TupleExtract);
      case Expression::Id::I31NewId:
        DELEGATE(I31New);
      case Expression::Id::I31GetId:
        DELEGATE(I31Get);
      case Expression::Id::RefTestId:
        DELEGATE(RefTest);
      case Expression::Id::RefCastId:
        DELEGATE(RefCast);
      case Expression::Id::BrOnCastId:
        DELEGATE(BrOnCast);
      case Expression::Id::RttCanonId:
        DELEGATE(RttCanon);
      case Expression::Id::RttSubId:
        DELEGATE(RttSub);
      case Expression::Id::StructNewId:
        DELEGATE(StructNew);
      case Expression::Id::StructGetId:
        DELEGATE(StructGet);
      case Expression::Id::StructSetId:
        DELEGATE(StructSet);
      case Expression::Id::ArrayNewId:
        DELEGATE(ArrayNew);
      case Expression::Id::ArrayGetId:
        DELEGATE(ArrayGet);
      case Expression::Id::ArraySetId:
        DELEGATE(ArraySet);
      case Expression::Id::ArrayLenId:
        DELEGATE(ArrayLen);
      case Expression::Id::InvalidId:
      default:
        WASM_UNREACHABLE("unexpected expression type");
    }

#undef DELEGATE
  }
};

// Visit with a single unified visitor, called on every node, instead of
// separate visit* per node

template<typename SubType, typename ReturnType = void>
struct UnifiedExpressionVisitor : public Visitor<SubType, ReturnType> {
  // called on each node
  ReturnType visitExpression(Expression* curr) { return ReturnType(); }

  // redirects
  ReturnType visitBlock(Block* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitIf(If* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitLoop(Loop* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitBreak(Break* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSwitch(Switch* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitCall(Call* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitCallIndirect(CallIndirect* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitLocalGet(LocalGet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitLocalSet(LocalSet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitGlobalGet(GlobalGet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitGlobalSet(GlobalSet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitLoad(Load* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitStore(Store* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitAtomicRMW(AtomicRMW* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitAtomicCmpxchg(AtomicCmpxchg* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitAtomicWait(AtomicWait* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitAtomicNotify(AtomicNotify* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitAtomicFence(AtomicFence* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSIMDExtract(SIMDExtract* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSIMDReplace(SIMDReplace* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSIMDShuffle(SIMDShuffle* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSIMDTernary(SIMDTernary* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSIMDShift(SIMDShift* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSIMDLoad(SIMDLoad* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitMemoryInit(MemoryInit* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitDataDrop(DataDrop* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitMemoryCopy(MemoryCopy* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitMemoryFill(MemoryFill* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitConst(Const* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitUnary(Unary* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitBinary(Binary* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitSelect(Select* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitDrop(Drop* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitReturn(Return* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitMemorySize(MemorySize* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitMemoryGrow(MemoryGrow* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRefNull(RefNull* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRefIsNull(RefIsNull* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRefFunc(RefFunc* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRefEq(RefEq* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitTry(Try* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitThrow(Throw* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRethrow(Rethrow* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitBrOnExn(BrOnExn* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitNop(Nop* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitUnreachable(Unreachable* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitPop(Pop* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitTupleMake(TupleMake* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitTupleExtract(TupleExtract* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitI31New(I31New* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitI31Get(I31Get* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRefTest(RefTest* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRefCast(RefCast* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitBrOnCast(BrOnCast* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRttCanon(RttCanon* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitRttSub(RttSub* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitStructNew(StructNew* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitStructGet(StructGet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitStructSet(StructSet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitArrayNew(ArrayNew* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitArrayGet(ArrayGet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitArraySet(ArraySet* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
  ReturnType visitArrayLen(ArrayLen* curr) {
    return static_cast<SubType*>(this)->visitExpression(curr);
  }
};

//
// Base class for all WasmWalkers, which can traverse an AST
// and provide the option to replace nodes while doing so.
//
// Subclass and implement the visit*()
// calls to run code on different node types.
//
template<typename SubType, typename VisitorType>
struct Walker : public VisitorType {
  // Useful methods for visitor implementions

  // Replace the current node. You can call this in your visit*() methods.
  // Note that the visit*() for the result node is not called for you (i.e.,
  // just one visit*() method is called by the traversal; if you replace a node,
  // and you want to process the output, you must do that explicitly).
  Expression* replaceCurrent(Expression* expression) {
    // Copy debug info, if present.
    if (currFunction) {
      auto& debugLocations = currFunction->debugLocations;
      if (!debugLocations.empty()) {
        auto* curr = getCurrent();
        auto iter = debugLocations.find(curr);
        if (iter != debugLocations.end()) {
          auto location = iter->second;
          debugLocations.erase(iter);
          debugLocations[expression] = location;
        }
      }
    }
    return *replacep = expression;
  }

  Expression* getCurrent() { return *replacep; }

  Expression** getCurrentPointer() { return replacep; }

  // Get the current module
  Module* getModule() { return currModule; }

  // Get the current function
  Function* getFunction() { return currFunction; }

  // Walk starting

  void walkGlobal(Global* global) {
    walk(global->init);
    static_cast<SubType*>(this)->visitGlobal(global);
  }

  void walkFunction(Function* func) {
    setFunction(func);
    static_cast<SubType*>(this)->doWalkFunction(func);
    static_cast<SubType*>(this)->visitFunction(func);
    setFunction(nullptr);
  }

  void walkEvent(Event* event) {
    static_cast<SubType*>(this)->visitEvent(event);
  }

  void walkFunctionInModule(Function* func, Module* module) {
    setModule(module);
    setFunction(func);
    static_cast<SubType*>(this)->doWalkFunction(func);
    static_cast<SubType*>(this)->visitFunction(func);
    setFunction(nullptr);
    setModule(nullptr);
  }

  // override this to provide custom functionality
  void doWalkFunction(Function* func) { walk(func->body); }

  void walkTable(Table* table) {
    for (auto& segment : table->segments) {
      walk(segment.offset);
    }
    static_cast<SubType*>(this)->visitTable(table);
  }

  void walkMemory(Memory* memory) {
    for (auto& segment : memory->segments) {
      if (!segment.isPassive) {
        walk(segment.offset);
      }
    }
    static_cast<SubType*>(this)->visitMemory(memory);
  }

  void walkModule(Module* module) {
    setModule(module);
    static_cast<SubType*>(this)->doWalkModule(module);
    static_cast<SubType*>(this)->visitModule(module);
    setModule(nullptr);
  }

  // override this to provide custom functionality
  void doWalkModule(Module* module) {
    // Dispatch statically through the SubType.
    SubType* self = static_cast<SubType*>(this);
    for (auto& curr : module->exports) {
      self->visitExport(curr.get());
    }
    for (auto& curr : module->globals) {
      if (curr->imported()) {
        self->visitGlobal(curr.get());
      } else {
        self->walkGlobal(curr.get());
      }
    }
    for (auto& curr : module->functions) {
      if (curr->imported()) {
        self->visitFunction(curr.get());
      } else {
        self->walkFunction(curr.get());
      }
    }
    for (auto& curr : module->events) {
      if (curr->imported()) {
        self->visitEvent(curr.get());
      } else {
        self->walkEvent(curr.get());
      }
    }
    self->walkTable(&module->table);
    self->walkMemory(&module->memory);
  }

  // Walk implementation. We don't use recursion as ASTs may be highly
  // nested.

  // Tasks receive the this pointer and a pointer to the pointer to operate on
  typedef void (*TaskFunc)(SubType*, Expression**);

  struct Task {
    TaskFunc func;
    Expression** currp;
    Task() {}
    Task(TaskFunc func, Expression** currp) : func(func), currp(currp) {}
  };

  void pushTask(TaskFunc func, Expression** currp) {
    assert(*currp);
    stack.emplace_back(func, currp);
  }
  void maybePushTask(TaskFunc func, Expression** currp) {
    if (*currp) {
      stack.emplace_back(func, currp);
    }
  }
  Task popTask() {
    auto ret = stack.back();
    stack.pop_back();
    return ret;
  }

  void walk(Expression*& root) {
    assert(stack.size() == 0);
    pushTask(SubType::scan, &root);
    while (stack.size() > 0) {
      auto task = popTask();
      replacep = task.currp;
      assert(*task.currp);
      task.func(static_cast<SubType*>(this), task.currp);
    }
  }

  // subclasses implement this to define the proper order of execution
  static void scan(SubType* self, Expression** currp) { abort(); }

  // task hooks to call visitors

  static void doVisitBlock(SubType* self, Expression** currp) {
    self->visitBlock((*currp)->cast<Block>());
  }
  static void doVisitIf(SubType* self, Expression** currp) {
    self->visitIf((*currp)->cast<If>());
  }
  static void doVisitLoop(SubType* self, Expression** currp) {
    self->visitLoop((*currp)->cast<Loop>());
  }
  static void doVisitBreak(SubType* self, Expression** currp) {
    self->visitBreak((*currp)->cast<Break>());
  }
  static void doVisitSwitch(SubType* self, Expression** currp) {
    self->visitSwitch((*currp)->cast<Switch>());
  }
  static void doVisitCall(SubType* self, Expression** currp) {
    self->visitCall((*currp)->cast<Call>());
  }
  static void doVisitCallIndirect(SubType* self, Expression** currp) {
    self->visitCallIndirect((*currp)->cast<CallIndirect>());
  }
  static void doVisitLocalGet(SubType* self, Expression** currp) {
    self->visitLocalGet((*currp)->cast<LocalGet>());
  }
  static void doVisitLocalSet(SubType* self, Expression** currp) {
    self->visitLocalSet((*currp)->cast<LocalSet>());
  }
  static void doVisitGlobalGet(SubType* self, Expression** currp) {
    self->visitGlobalGet((*currp)->cast<GlobalGet>());
  }
  static void doVisitGlobalSet(SubType* self, Expression** currp) {
    self->visitGlobalSet((*currp)->cast<GlobalSet>());
  }
  static void doVisitLoad(SubType* self, Expression** currp) {
    self->visitLoad((*currp)->cast<Load>());
  }
  static void doVisitStore(SubType* self, Expression** currp) {
    self->visitStore((*currp)->cast<Store>());
  }
  static void doVisitAtomicRMW(SubType* self, Expression** currp) {
    self->visitAtomicRMW((*currp)->cast<AtomicRMW>());
  }
  static void doVisitAtomicCmpxchg(SubType* self, Expression** currp) {
    self->visitAtomicCmpxchg((*currp)->cast<AtomicCmpxchg>());
  }
  static void doVisitAtomicWait(SubType* self, Expression** currp) {
    self->visitAtomicWait((*currp)->cast<AtomicWait>());
  }
  static void doVisitAtomicNotify(SubType* self, Expression** currp) {
    self->visitAtomicNotify((*currp)->cast<AtomicNotify>());
  }
  static void doVisitAtomicFence(SubType* self, Expression** currp) {
    self->visitAtomicFence((*currp)->cast<AtomicFence>());
  }
  static void doVisitSIMDExtract(SubType* self, Expression** currp) {
    self->visitSIMDExtract((*currp)->cast<SIMDExtract>());
  }
  static void doVisitSIMDReplace(SubType* self, Expression** currp) {
    self->visitSIMDReplace((*currp)->cast<SIMDReplace>());
  }
  static void doVisitSIMDShuffle(SubType* self, Expression** currp) {
    self->visitSIMDShuffle((*currp)->cast<SIMDShuffle>());
  }
  static void doVisitSIMDTernary(SubType* self, Expression** currp) {
    self->visitSIMDTernary((*currp)->cast<SIMDTernary>());
  }
  static void doVisitSIMDShift(SubType* self, Expression** currp) {
    self->visitSIMDShift((*currp)->cast<SIMDShift>());
  }
  static void doVisitSIMDLoad(SubType* self, Expression** currp) {
    self->visitSIMDLoad((*currp)->cast<SIMDLoad>());
  }
  static void doVisitMemoryInit(SubType* self, Expression** currp) {
    self->visitMemoryInit((*currp)->cast<MemoryInit>());
  }
  static void doVisitDataDrop(SubType* self, Expression** currp) {
    self->visitDataDrop((*currp)->cast<DataDrop>());
  }
  static void doVisitMemoryCopy(SubType* self, Expression** currp) {
    self->visitMemoryCopy((*currp)->cast<MemoryCopy>());
  }
  static void doVisitMemoryFill(SubType* self, Expression** currp) {
    self->visitMemoryFill((*currp)->cast<MemoryFill>());
  }
  static void doVisitConst(SubType* self, Expression** currp) {
    self->visitConst((*currp)->cast<Const>());
  }
  static void doVisitUnary(SubType* self, Expression** currp) {
    self->visitUnary((*currp)->cast<Unary>());
  }
  static void doVisitBinary(SubType* self, Expression** currp) {
    self->visitBinary((*currp)->cast<Binary>());
  }
  static void doVisitSelect(SubType* self, Expression** currp) {
    self->visitSelect((*currp)->cast<Select>());
  }
  static void doVisitDrop(SubType* self, Expression** currp) {
    self->visitDrop((*currp)->cast<Drop>());
  }
  static void doVisitReturn(SubType* self, Expression** currp) {
    self->visitReturn((*currp)->cast<Return>());
  }
  static void doVisitMemorySize(SubType* self, Expression** currp) {
    self->visitMemorySize((*currp)->cast<MemorySize>());
  }
  static void doVisitMemoryGrow(SubType* self, Expression** currp) {
    self->visitMemoryGrow((*currp)->cast<MemoryGrow>());
  }
  static void doVisitRefNull(SubType* self, Expression** currp) {
    self->visitRefNull((*currp)->cast<RefNull>());
  }
  static void doVisitRefIsNull(SubType* self, Expression** currp) {
    self->visitRefIsNull((*currp)->cast<RefIsNull>());
  }
  static void doVisitRefFunc(SubType* self, Expression** currp) {
    self->visitRefFunc((*currp)->cast<RefFunc>());
  }
  static void doVisitRefEq(SubType* self, Expression** currp) {
    self->visitRefEq((*currp)->cast<RefEq>());
  }
  static void doVisitTry(SubType* self, Expression** currp) {
    self->visitTry((*currp)->cast<Try>());
  }
  static void doVisitThrow(SubType* self, Expression** currp) {
    self->visitThrow((*currp)->cast<Throw>());
  }
  static void doVisitRethrow(SubType* self, Expression** currp) {
    self->visitRethrow((*currp)->cast<Rethrow>());
  }
  static void doVisitBrOnExn(SubType* self, Expression** currp) {
    self->visitBrOnExn((*currp)->cast<BrOnExn>());
  }
  static void doVisitNop(SubType* self, Expression** currp) {
    self->visitNop((*currp)->cast<Nop>());
  }
  static void doVisitUnreachable(SubType* self, Expression** currp) {
    self->visitUnreachable((*currp)->cast<Unreachable>());
  }
  static void doVisitPop(SubType* self, Expression** currp) {
    self->visitPop((*currp)->cast<Pop>());
  }
  static void doVisitTupleMake(SubType* self, Expression** currp) {
    self->visitTupleMake((*currp)->cast<TupleMake>());
  }
  static void doVisitTupleExtract(SubType* self, Expression** currp) {
    self->visitTupleExtract((*currp)->cast<TupleExtract>());
  }
  static void doVisitI31New(SubType* self, Expression** currp) {
    self->visitI31New((*currp)->cast<I31New>());
  }
  static void doVisitI31Get(SubType* self, Expression** currp) {
    self->visitI31Get((*currp)->cast<I31Get>());
  }
  static void doVisitRefTest(SubType* self, Expression** currp) {
    self->visitRefTest((*currp)->cast<RefTest>());
  }
  static void doVisitRefCast(SubType* self, Expression** currp) {
    self->visitRefCast((*currp)->cast<RefCast>());
  }
  static void doVisitBrOnCast(SubType* self, Expression** currp) {
    self->visitBrOnCast((*currp)->cast<BrOnCast>());
  }
  static void doVisitRttCanon(SubType* self, Expression** currp) {
    self->visitRttCanon((*currp)->cast<RttCanon>());
  }
  static void doVisitRttSub(SubType* self, Expression** currp) {
    self->visitRttSub((*currp)->cast<RttSub>());
  }
  static void doVisitStructNew(SubType* self, Expression** currp) {
    self->visitStructNew((*currp)->cast<StructNew>());
  }
  static void doVisitStructGet(SubType* self, Expression** currp) {
    self->visitStructGet((*currp)->cast<StructGet>());
  }
  static void doVisitStructSet(SubType* self, Expression** currp) {
    self->visitStructSet((*currp)->cast<StructSet>());
  }
  static void doVisitArrayNew(SubType* self, Expression** currp) {
    self->visitArrayNew((*currp)->cast<ArrayNew>());
  }
  static void doVisitArrayGet(SubType* self, Expression** currp) {
    self->visitArrayGet((*currp)->cast<ArrayGet>());
  }
  static void doVisitArraySet(SubType* self, Expression** currp) {
    self->visitArraySet((*currp)->cast<ArraySet>());
  }
  static void doVisitArrayLen(SubType* self, Expression** currp) {
    self->visitArrayLen((*currp)->cast<ArrayLen>());
  }

  void setModule(Module* module) { currModule = module; }

  void setFunction(Function* func) { currFunction = func; }

private:
  // the address of the current node, used to replace it
  Expression** replacep = nullptr;
  SmallVector<Task, 10> stack;      // stack of tasks
  Function* currFunction = nullptr; // current function being processed
  Module* currModule = nullptr;     // current module being processed
};

// Walks in post-order, i.e., children first. When there isn't an obvious
// order to operands, we follow them in order of execution.

template<typename SubType, typename VisitorType = Visitor<SubType>>
struct PostWalker : public Walker<SubType, VisitorType> {

  static void scan(SubType* self, Expression** currp) {
    Expression* curr = *currp;
    switch (curr->_id) {
      case Expression::Id::InvalidId:
        abort();
      case Expression::Id::BlockId: {
        self->pushTask(SubType::doVisitBlock, currp);
        auto& list = curr->cast<Block>()->list;
        for (int i = int(list.size()) - 1; i >= 0; i--) {
          self->pushTask(SubType::scan, &list[i]);
        }
        break;
      }
      case Expression::Id::IfId: {
        self->pushTask(SubType::doVisitIf, currp);
        self->maybePushTask(SubType::scan, &curr->cast<If>()->ifFalse);
        self->pushTask(SubType::scan, &curr->cast<If>()->ifTrue);
        self->pushTask(SubType::scan, &curr->cast<If>()->condition);
        break;
      }
      case Expression::Id::LoopId: {
        self->pushTask(SubType::doVisitLoop, currp);
        self->pushTask(SubType::scan, &curr->cast<Loop>()->body);
        break;
      }
      case Expression::Id::BreakId: {
        self->pushTask(SubType::doVisitBreak, currp);
        self->maybePushTask(SubType::scan, &curr->cast<Break>()->condition);
        self->maybePushTask(SubType::scan, &curr->cast<Break>()->value);
        break;
      }
      case Expression::Id::SwitchId: {
        self->pushTask(SubType::doVisitSwitch, currp);
        self->pushTask(SubType::scan, &curr->cast<Switch>()->condition);
        self->maybePushTask(SubType::scan, &curr->cast<Switch>()->value);
        break;
      }
      case Expression::Id::CallId: {
        self->pushTask(SubType::doVisitCall, currp);
        auto& list = curr->cast<Call>()->operands;
        for (int i = int(list.size()) - 1; i >= 0; i--) {
          self->pushTask(SubType::scan, &list[i]);
        }
        break;
      }
      case Expression::Id::CallIndirectId: {
        self->pushTask(SubType::doVisitCallIndirect, currp);
        auto& list = curr->cast<CallIndirect>()->operands;
        self->pushTask(SubType::scan, &curr->cast<CallIndirect>()->target);
        for (int i = int(list.size()) - 1; i >= 0; i--) {
          self->pushTask(SubType::scan, &list[i]);
        }
        break;
      }
      case Expression::Id::LocalGetId: {
        // TODO: optimize leaves with a direct call?
        self->pushTask(SubType::doVisitLocalGet, currp);
        break;
      }
      case Expression::Id::LocalSetId: {
        self->pushTask(SubType::doVisitLocalSet, currp);
        self->pushTask(SubType::scan, &curr->cast<LocalSet>()->value);
        break;
      }
      case Expression::Id::GlobalGetId: {
        self->pushTask(SubType::doVisitGlobalGet, currp);
        break;
      }
      case Expression::Id::GlobalSetId: {
        self->pushTask(SubType::doVisitGlobalSet, currp);
        self->pushTask(SubType::scan, &curr->cast<GlobalSet>()->value);
        break;
      }
      case Expression::Id::LoadId: {
        self->pushTask(SubType::doVisitLoad, currp);
        self->pushTask(SubType::scan, &curr->cast<Load>()->ptr);
        break;
      }
      case Expression::Id::StoreId: {
        self->pushTask(SubType::doVisitStore, currp);
        self->pushTask(SubType::scan, &curr->cast<Store>()->value);
        self->pushTask(SubType::scan, &curr->cast<Store>()->ptr);
        break;
      }
      case Expression::Id::AtomicRMWId: {
        self->pushTask(SubType::doVisitAtomicRMW, currp);
        self->pushTask(SubType::scan, &curr->cast<AtomicRMW>()->value);
        self->pushTask(SubType::scan, &curr->cast<AtomicRMW>()->ptr);
        break;
      }
      case Expression::Id::AtomicCmpxchgId: {
        self->pushTask(SubType::doVisitAtomicCmpxchg, currp);
        self->pushTask(SubType::scan,
                       &curr->cast<AtomicCmpxchg>()->replacement);
        self->pushTask(SubType::scan, &curr->cast<AtomicCmpxchg>()->expected);
        self->pushTask(SubType::scan, &curr->cast<AtomicCmpxchg>()->ptr);
        break;
      }
      case Expression::Id::AtomicWaitId: {
        self->pushTask(SubType::doVisitAtomicWait, currp);
        self->pushTask(SubType::scan, &curr->cast<AtomicWait>()->timeout);
        self->pushTask(SubType::scan, &curr->cast<AtomicWait>()->expected);
        self->pushTask(SubType::scan, &curr->cast<AtomicWait>()->ptr);
        break;
      }
      case Expression::Id::AtomicNotifyId: {
        self->pushTask(SubType::doVisitAtomicNotify, currp);
        self->pushTask(SubType::scan, &curr->cast<AtomicNotify>()->notifyCount);
        self->pushTask(SubType::scan, &curr->cast<AtomicNotify>()->ptr);
        break;
      }
      case Expression::Id::AtomicFenceId: {
        self->pushTask(SubType::doVisitAtomicFence, currp);
        break;
      }
      case Expression::Id::SIMDExtractId: {
        self->pushTask(SubType::doVisitSIMDExtract, currp);
        self->pushTask(SubType::scan, &curr->cast<SIMDExtract>()->vec);
        break;
      }
      case Expression::Id::SIMDReplaceId: {
        self->pushTask(SubType::doVisitSIMDReplace, currp);
        self->pushTask(SubType::scan, &curr->cast<SIMDReplace>()->value);
        self->pushTask(SubType::scan, &curr->cast<SIMDReplace>()->vec);
        break;
      }
      case Expression::Id::SIMDShuffleId: {
        self->pushTask(SubType::doVisitSIMDShuffle, currp);
        self->pushTask(SubType::scan, &curr->cast<SIMDShuffle>()->right);
        self->pushTask(SubType::scan, &curr->cast<SIMDShuffle>()->left);
        break;
      }
      case Expression::Id::SIMDTernaryId: {
        self->pushTask(SubType::doVisitSIMDTernary, currp);
        self->pushTask(SubType::scan, &curr->cast<SIMDTernary>()->c);
        self->pushTask(SubType::scan, &curr->cast<SIMDTernary>()->b);
        self->pushTask(SubType::scan, &curr->cast<SIMDTernary>()->a);
        break;
      }
      case Expression::Id::SIMDShiftId: {
        self->pushTask(SubType::doVisitSIMDShift, currp);
        self->pushTask(SubType::scan, &curr->cast<SIMDShift>()->shift);
        self->pushTask(SubType::scan, &curr->cast<SIMDShift>()->vec);
        break;
      }
      case Expression::Id::SIMDLoadId: {
        self->pushTask(SubType::doVisitSIMDLoad, currp);
        self->pushTask(SubType::scan, &curr->cast<SIMDLoad>()->ptr);
        break;
      }
      case Expression::Id::MemoryInitId: {
        self->pushTask(SubType::doVisitMemoryInit, currp);
        self->pushTask(SubType::scan, &curr->cast<MemoryInit>()->size);
        self->pushTask(SubType::scan, &curr->cast<MemoryInit>()->offset);
        self->pushTask(SubType::scan, &curr->cast<MemoryInit>()->dest);
        break;
      }
      case Expression::Id::DataDropId: {
        self->pushTask(SubType::doVisitDataDrop, currp);
        break;
      }
      case Expression::Id::MemoryCopyId: {
        self->pushTask(SubType::doVisitMemoryCopy, currp);
        self->pushTask(SubType::scan, &curr->cast<MemoryCopy>()->size);
        self->pushTask(SubType::scan, &curr->cast<MemoryCopy>()->source);
        self->pushTask(SubType::scan, &curr->cast<MemoryCopy>()->dest);
        break;
      }
      case Expression::Id::MemoryFillId: {
        self->pushTask(SubType::doVisitMemoryFill, currp);
        self->pushTask(SubType::scan, &curr->cast<MemoryFill>()->size);
        self->pushTask(SubType::scan, &curr->cast<MemoryFill>()->value);
        self->pushTask(SubType::scan, &curr->cast<MemoryFill>()->dest);
        break;
      }
      case Expression::Id::ConstId: {
        self->pushTask(SubType::doVisitConst, currp);
        break;
      }
      case Expression::Id::UnaryId: {
        self->pushTask(SubType::doVisitUnary, currp);
        self->pushTask(SubType::scan, &curr->cast<Unary>()->value);
        break;
      }
      case Expression::Id::BinaryId: {
        self->pushTask(SubType::doVisitBinary, currp);
        self->pushTask(SubType::scan, &curr->cast<Binary>()->right);
        self->pushTask(SubType::scan, &curr->cast<Binary>()->left);
        break;
      }
      case Expression::Id::SelectId: {
        self->pushTask(SubType::doVisitSelect, currp);
        self->pushTask(SubType::scan, &curr->cast<Select>()->condition);
        self->pushTask(SubType::scan, &curr->cast<Select>()->ifFalse);
        self->pushTask(SubType::scan, &curr->cast<Select>()->ifTrue);
        break;
      }
      case Expression::Id::DropId: {
        self->pushTask(SubType::doVisitDrop, currp);
        self->pushTask(SubType::scan, &curr->cast<Drop>()->value);
        break;
      }
      case Expression::Id::ReturnId: {
        self->pushTask(SubType::doVisitReturn, currp);
        self->maybePushTask(SubType::scan, &curr->cast<Return>()->value);
        break;
      }
      case Expression::Id::MemorySizeId:
        self->pushTask(SubType::doVisitMemorySize, currp);
        break;
      case Expression::Id::MemoryGrowId:
        self->pushTask(SubType::doVisitMemoryGrow, currp);
        self->pushTask(SubType::scan, &curr->cast<MemoryGrow>()->delta);
        break;
      case Expression::Id::RefNullId: {
        self->pushTask(SubType::doVisitRefNull, currp);
        break;
      }
      case Expression::Id::RefIsNullId: {
        self->pushTask(SubType::doVisitRefIsNull, currp);
        self->pushTask(SubType::scan, &curr->cast<RefIsNull>()->value);
        break;
      }
      case Expression::Id::RefFuncId: {
        self->pushTask(SubType::doVisitRefFunc, currp);
        break;
      }
      case Expression::Id::RefEqId: {
        self->pushTask(SubType::doVisitRefEq, currp);
        self->pushTask(SubType::scan, &curr->cast<RefEq>()->right);
        self->pushTask(SubType::scan, &curr->cast<RefEq>()->left);
        break;
      }
      case Expression::Id::TryId: {
        self->pushTask(SubType::doVisitTry, currp);
        self->pushTask(SubType::scan, &curr->cast<Try>()->catchBody);
        self->pushTask(SubType::scan, &curr->cast<Try>()->body);
        break;
      }
      case Expression::Id::ThrowId: {
        self->pushTask(SubType::doVisitThrow, currp);
        auto& list = curr->cast<Throw>()->operands;
        for (int i = int(list.size()) - 1; i >= 0; i--) {
          self->pushTask(SubType::scan, &list[i]);
        }
        break;
      }
      case Expression::Id::RethrowId: {
        self->pushTask(SubType::doVisitRethrow, currp);
        self->pushTask(SubType::scan, &curr->cast<Rethrow>()->exnref);
        break;
      }
      case Expression::Id::BrOnExnId: {
        self->pushTask(SubType::doVisitBrOnExn, currp);
        self->pushTask(SubType::scan, &curr->cast<BrOnExn>()->exnref);
        break;
      }
      case Expression::Id::NopId: {
        self->pushTask(SubType::doVisitNop, currp);
        break;
      }
      case Expression::Id::UnreachableId: {
        self->pushTask(SubType::doVisitUnreachable, currp);
        break;
      }
      case Expression::Id::PopId: {
        self->pushTask(SubType::doVisitPop, currp);
        break;
      }
      case Expression::Id::TupleMakeId: {
        self->pushTask(SubType::doVisitTupleMake, currp);
        auto& operands = curr->cast<TupleMake>()->operands;
        for (int i = int(operands.size()) - 1; i >= 0; --i) {
          self->pushTask(SubType::scan, &operands[i]);
        }
        break;
      }
      case Expression::Id::TupleExtractId: {
        self->pushTask(SubType::doVisitTupleExtract, currp);
        self->pushTask(SubType::scan, &curr->cast<TupleExtract>()->tuple);
        break;
      }
      case Expression::Id::I31NewId: {
        self->pushTask(SubType::doVisitI31New, currp);
        self->pushTask(SubType::scan, &curr->cast<I31New>()->value);
        break;
      }
      case Expression::Id::I31GetId: {
        self->pushTask(SubType::doVisitI31Get, currp);
        self->pushTask(SubType::scan, &curr->cast<I31Get>()->i31);
        break;
      }
      case Expression::Id::RefTestId:
        self->pushTask(SubType::doVisitRefTest, currp);
        WASM_UNREACHABLE("TODO (gc): ref.test");
        break;
      case Expression::Id::RefCastId:
        self->pushTask(SubType::doVisitRefCast, currp);
        WASM_UNREACHABLE("TODO (gc): ref.cast");
        break;
      case Expression::Id::BrOnCastId:
        self->pushTask(SubType::doVisitBrOnCast, currp);
        WASM_UNREACHABLE("TODO (gc): br_on_cast");
        break;
      case Expression::Id::RttCanonId:
        self->pushTask(SubType::doVisitRttCanon, currp);
        WASM_UNREACHABLE("TODO (gc): rtt.canon");
        break;
      case Expression::Id::RttSubId:
        self->pushTask(SubType::doVisitRttSub, currp);
        WASM_UNREACHABLE("TODO (gc): rtt.sub");
        break;
      case Expression::Id::StructNewId:
        self->pushTask(SubType::doVisitStructNew, currp);
        WASM_UNREACHABLE("TODO (gc): struct.new");
        break;
      case Expression::Id::StructGetId:
        self->pushTask(SubType::doVisitStructGet, currp);
        WASM_UNREACHABLE("TODO (gc): struct.get");
        break;
      case Expression::Id::StructSetId:
        self->pushTask(SubType::doVisitStructSet, currp);
        WASM_UNREACHABLE("TODO (gc): struct.set");
        break;
      case Expression::Id::ArrayNewId:
        self->pushTask(SubType::doVisitArrayNew, currp);
        WASM_UNREACHABLE("TODO (gc): array.new");
        break;
      case Expression::Id::ArrayGetId:
        self->pushTask(SubType::doVisitArrayGet, currp);
        WASM_UNREACHABLE("TODO (gc): array.get");
        break;
      case Expression::Id::ArraySetId:
        self->pushTask(SubType::doVisitArraySet, currp);
        WASM_UNREACHABLE("TODO (gc): array.set");
        break;
      case Expression::Id::ArrayLenId:
        self->pushTask(SubType::doVisitArrayLen, currp);
        WASM_UNREACHABLE("TODO (gc): array.len");
        break;
      case Expression::Id::NumExpressionIds:
        WASM_UNREACHABLE("unexpected expression type");
    }
  }
};

// Stacks of expressions tend to be limited in size (although, sometimes
// super-nested blocks exist for br_table).
typedef SmallVector<Expression*, 10> ExpressionStack;

// Traversal with a control-flow stack.

template<typename SubType, typename VisitorType = Visitor<SubType>>
struct ControlFlowWalker : public PostWalker<SubType, VisitorType> {
  ControlFlowWalker() = default;

  ExpressionStack controlFlowStack; // contains blocks, loops, and ifs

  // Uses the control flow stack to find the target of a break to a name
  Expression* findBreakTarget(Name name) {
    assert(!controlFlowStack.empty());
    Index i = controlFlowStack.size() - 1;
    while (true) {
      auto* curr = controlFlowStack[i];
      if (Block* block = curr->template dynCast<Block>()) {
        if (name == block->name) {
          return curr;
        }
      } else if (Loop* loop = curr->template dynCast<Loop>()) {
        if (name == loop->name) {
          return curr;
        }
      } else {
        // an if or try, ignorable
        assert(curr->template is<If>() || curr->template is<Try>());
      }
      if (i == 0) {
        return nullptr;
      }
      i--;
    }
  }

  static void doPreVisitControlFlow(SubType* self, Expression** currp) {
    self->controlFlowStack.push_back(*currp);
  }

  static void doPostVisitControlFlow(SubType* self, Expression** currp) {
    // note that we might be popping something else, as we may have been
    // replaced
    self->controlFlowStack.pop_back();
  }

  static void scan(SubType* self, Expression** currp) {
    auto* curr = *currp;

    switch (curr->_id) {
      case Expression::Id::BlockId:
      case Expression::Id::IfId:
      case Expression::Id::LoopId:
      case Expression::Id::TryId: {
        self->pushTask(SubType::doPostVisitControlFlow, currp);
        break;
      }
      default: {
      }
    }

    PostWalker<SubType, VisitorType>::scan(self, currp);

    switch (curr->_id) {
      case Expression::Id::BlockId:
      case Expression::Id::IfId:
      case Expression::Id::LoopId:
      case Expression::Id::TryId: {
        self->pushTask(SubType::doPreVisitControlFlow, currp);
        break;
      }
      default: {
      }
    }
  }
};

// Traversal with an expression stack.

template<typename SubType, typename VisitorType = Visitor<SubType>>
struct ExpressionStackWalker : public PostWalker<SubType, VisitorType> {
  ExpressionStackWalker() = default;

  ExpressionStack expressionStack;

  // Uses the control flow stack to find the target of a break to a name
  Expression* findBreakTarget(Name name) {
    assert(!expressionStack.empty());
    Index i = expressionStack.size() - 1;
    while (true) {
      auto* curr = expressionStack[i];
      if (Block* block = curr->template dynCast<Block>()) {
        if (name == block->name) {
          return curr;
        }
      } else if (Loop* loop = curr->template dynCast<Loop>()) {
        if (name == loop->name) {
          return curr;
        }
      }
      if (i == 0) {
        return nullptr;
      }
      i--;
    }
  }

  Expression* getParent() {
    if (expressionStack.size() == 1) {
      return nullptr;
    }
    assert(expressionStack.size() >= 2);
    return expressionStack[expressionStack.size() - 2];
  }

  static void doPreVisit(SubType* self, Expression** currp) {
    self->expressionStack.push_back(*currp);
  }

  static void doPostVisit(SubType* self, Expression** currp) {
    self->expressionStack.pop_back();
  }

  static void scan(SubType* self, Expression** currp) {
    self->pushTask(SubType::doPostVisit, currp);

    PostWalker<SubType, VisitorType>::scan(self, currp);

    self->pushTask(SubType::doPreVisit, currp);
  }

  Expression* replaceCurrent(Expression* expression) {
    PostWalker<SubType, VisitorType>::replaceCurrent(expression);
    // also update the stack
    expressionStack.back() = expression;
    return expression;
  }
};

// Traversal in the order of execution. This is quick and simple, but
// does not provide the same comprehensive information that a full
// conversion to basic blocks would. What it does give is a quick
// way to view straightline execution traces, i.e., that have no
// branching. This can let optimizations get most of what they
// want without the cost of creating another AST.
//
// When execution is no longer linear, this notifies via a call
// to noteNonLinear().

template<typename SubType, typename VisitorType = Visitor<SubType>>
struct LinearExecutionWalker : public PostWalker<SubType, VisitorType> {
  LinearExecutionWalker() = default;

  // subclasses should implement this
  void noteNonLinear(Expression* curr) { abort(); }

  static void doNoteNonLinear(SubType* self, Expression** currp) {
    self->noteNonLinear(*currp);
  }

  static void scan(SubType* self, Expression** currp) {

    Expression* curr = *currp;

    switch (curr->_id) {
      case Expression::Id::InvalidId:
        abort();
      case Expression::Id::BlockId: {
        self->pushTask(SubType::doVisitBlock, currp);
        if (curr->cast<Block>()->name.is()) {
          self->pushTask(SubType::doNoteNonLinear, currp);
        }
        auto& list = curr->cast<Block>()->list;
        for (int i = int(list.size()) - 1; i >= 0; i--) {
          self->pushTask(SubType::scan, &list[i]);
        }
        break;
      }
      case Expression::Id::IfId: {
        self->pushTask(SubType::doVisitIf, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->maybePushTask(SubType::scan, &curr->cast<If>()->ifFalse);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->pushTask(SubType::scan, &curr->cast<If>()->ifTrue);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->pushTask(SubType::scan, &curr->cast<If>()->condition);
        break;
      }
      case Expression::Id::LoopId: {
        self->pushTask(SubType::doVisitLoop, currp);
        self->pushTask(SubType::scan, &curr->cast<Loop>()->body);
        self->pushTask(SubType::doNoteNonLinear, currp);
        break;
      }
      case Expression::Id::BreakId: {
        self->pushTask(SubType::doVisitBreak, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->maybePushTask(SubType::scan, &curr->cast<Break>()->condition);
        self->maybePushTask(SubType::scan, &curr->cast<Break>()->value);
        break;
      }
      case Expression::Id::SwitchId: {
        self->pushTask(SubType::doVisitSwitch, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->maybePushTask(SubType::scan, &curr->cast<Switch>()->value);
        self->pushTask(SubType::scan, &curr->cast<Switch>()->condition);
        break;
      }
      case Expression::Id::ReturnId: {
        self->pushTask(SubType::doVisitReturn, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->maybePushTask(SubType::scan, &curr->cast<Return>()->value);
        break;
      }
      case Expression::Id::TryId: {
        self->pushTask(SubType::doVisitTry, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->pushTask(SubType::scan, &curr->cast<Try>()->catchBody);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->pushTask(SubType::scan, &curr->cast<Try>()->body);
        break;
      }
      case Expression::Id::ThrowId: {
        self->pushTask(SubType::doVisitThrow, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        auto& list = curr->cast<Throw>()->operands;
        for (int i = int(list.size()) - 1; i >= 0; i--) {
          self->pushTask(SubType::scan, &list[i]);
        }
        break;
      }
      case Expression::Id::RethrowId: {
        self->pushTask(SubType::doVisitRethrow, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->pushTask(SubType::scan, &curr->cast<Rethrow>()->exnref);
        break;
      }
      case Expression::Id::BrOnExnId: {
        self->pushTask(SubType::doVisitBrOnExn, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        self->pushTask(SubType::scan, &curr->cast<BrOnExn>()->exnref);
        break;
      }
      case Expression::Id::UnreachableId: {
        self->pushTask(SubType::doVisitUnreachable, currp);
        self->pushTask(SubType::doNoteNonLinear, currp);
        break;
      }
      default: {
        // other node types do not have control flow, use regular post-order
        PostWalker<SubType, VisitorType>::scan(self, currp);
      }
    }
  }
};

} // namespace wasm

#endif // wasm_wasm_traversal_h
