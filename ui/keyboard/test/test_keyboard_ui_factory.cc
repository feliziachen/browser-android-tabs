// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/keyboard/test/test_keyboard_ui_factory.h"

#include "base/threading/sequenced_task_runner_handle.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/keyboard/test/keyboard_test_util.h"

#include <utility>

namespace keyboard {

TestKeyboardUIFactory::TestKeyboardUIFactory(ui::InputMethod* input_method)
    : input_method_(input_method) {}

TestKeyboardUIFactory::~TestKeyboardUIFactory() = default;

std::unique_ptr<KeyboardUI> TestKeyboardUIFactory::CreateKeyboardUI() {
  return std::make_unique<TestKeyboardUI>(input_method_);
}

// TestKeyboardUIFactory::TestKeyboardUI:

TestKeyboardUIFactory::TestKeyboardUI::TestKeyboardUI(
    ui::InputMethod* input_method)
    : input_method_(input_method) {}

TestKeyboardUIFactory::TestKeyboardUI::~TestKeyboardUI() {
  // Destroy the window before the delegate.
  window_.reset();
}

aura::Window* TestKeyboardUIFactory::TestKeyboardUI::LoadKeyboardWindow(
    LoadCallback callback) {
  DCHECK(!window_);
  window_ = std::make_unique<aura::Window>(&delegate_);
  window_->Init(ui::LAYER_NOT_DRAWN);
  window_->set_owned_by_parent(false);

  // Set a default size for the keyboard.
  display::Screen* screen = display::Screen::GetScreen();
  window_->SetBounds(
      KeyboardBoundsFromRootBounds(screen->GetPrimaryDisplay().bounds()));

  // Simulate an asynchronous load.
  base::SequencedTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                   std::move(callback));

  return window_.get();
}

aura::Window* TestKeyboardUIFactory::TestKeyboardUI::GetKeyboardWindow() const {
  return window_.get();
}

ui::InputMethod* TestKeyboardUIFactory::TestKeyboardUI::GetInputMethod() {
  return input_method_;
}

}  // namespace keyboard
