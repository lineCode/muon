// Copyright (c) 2017 The Brave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRAVE_COMMON_WORKERS_WORKER_BINDINGS_H_
#define BRAVE_COMMON_WORKERS_WORKER_BINDINGS_H_

#include <utility>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "extensions/renderer/object_backed_native_handler.h"
#include "v8/include/v8.h"

namespace brave {

class V8WorkerThread;

class WorkerBindings : public extensions::ObjectBackedNativeHandler {
 public:
  WorkerBindings(extensions::ScriptContext* context, V8WorkerThread* worker);
  ~WorkerBindings() override;
  static void OnMessage(v8::Isolate* isolate,
                        base::PlatformThreadId thread_id,
                        v8::Local<v8::Value> message);

 private:
  void PostMessage(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Emit(const std::pair<uint8_t*, size_t>& buffer);

  V8WorkerThread* worker_;
  v8::Local<v8::Function> on_message_;

  DISALLOW_COPY_AND_ASSIGN(WorkerBindings);
};

}  // namespace brave

#endif  // BRAVE_COMMON_WORKERS_WORKER_BINDINGS_H_
