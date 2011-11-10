/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Gamepad API.
 *
 * The Initial Developer of the Original Code is
 * The Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Ted Mielczarek <ted.mielczarek@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "GamepadService.h"

#include "nsAutoPtr.h"
#include "nsFocusManager.h"
#include "nsIDOMEvent.h"
#include "nsIDOMDocument.h"
#include "nsIDOMEventTarget.h"
#include "nsDOMGamepad.h"
#include "nsIDOMGamepadButtonEvent.h"
#include "nsIDOMGamepadAxisMoveEvent.h"
#include "nsIDOMGamepadConnectionEvent.h"
#include "nsIDOMWindow.h"
#include "nsIPrivateDOMEvent.h"
#include "nsIServiceManager.h"
#include "nsITimer.h"

#include <cstddef>

namespace mozilla {
namespace dom {

// This should be implemented per-platform and return an instance
// of the GamepadService subclass.
extern GamepadService* CreateGamepadService();

// Amount of time to wait before cleaning up gamepad resources
// when no pages are listening for events.
static const int kCleanupDelayMS = 2000;

GamepadService* GamepadService::sSingleton = NULL;

GamepadService::GamepadService()
  : mStarted(false),
    mFocusManager(do_GetService(FOCUSMANAGER_CONTRACTID))
{
  mListeners.Init();
}

GamepadService::~GamepadService()
{
}

//TODO: add an xpcom shutdown listener to force shutdown
void
GamepadService::AddListener(nsIDOMWindow *aWindow)
{
  PRUint32 data;
  if (mListeners.Get(aWindow, &data)) {
    return; // already exists
  }

  if (!mStarted) {
    Startup();
  }

  mListeners.Put(aWindow, 0);
}

void
GamepadService::RemoveListener(nsIDOMWindow *aWindow)
{
  PRUint32 data;
  if (!mListeners.Get(aWindow, &data)) {
    return; // doesn't exist
  }

  mListeners.Remove(aWindow);

  if (mListeners.Count() == 0) {
    StartCleanupTimer();
  }
}

PRUint32
GamepadService::AddGamepad(const char* id,
                           PRUint32 numButtons,
                           PRUint32 numAxes) {
  //TODO: get initial button/axis state
  nsDOMGamepad* gamepad =
    new nsDOMGamepad(NS_ConvertUTF8toUTF16(nsDependentCString(id)),
                     0,
                     numButtons,
                     numAxes);
  int index = -1;
  for (PRUint32 i = 0; i < mGamepads.Length(); i++) {
    if (!mGamepads[i]) {
      mGamepads[i] = gamepad;
      index = i;
      break;
    }
  }
  if (index == -1) {
    mGamepads.AppendElement(gamepad);
    index = mGamepads.Length() - 1;
  }

  gamepad->SetIndex(index);
  NewConnectionEvent(index, true);

  return index;
}

void
GamepadService::RemoveGamepad(PRUint32 index) {
  if (index < mGamepads.Length()) {
    mGamepads[index]->SetConnected(false);
    NewConnectionEvent(index, false);
    // If this is the last entry in the list, just remove it.
    if (index == mGamepads.Length() - 1) {
      mGamepads.RemoveElementAt(index);
    } else {
      // Otherwise just null it out and leave it, so the
      // indices of the following entries remain valid.
      mGamepads[index] = NULL;
    }
  }
}

void
GamepadService::NewButtonEvent(PRUint32 index, PRUint32 button, bool pressed) {
  if (index >= mGamepads.Length()) {
    return;
  }

  mGamepads[index]->SetButton(button, pressed ? 1 : 0);

  nsCOMPtr<nsIDOMWindow> window;
  if (GetFocusedWindow(getter_AddRefs(window))) {
    if (!WindowHasSeenGamepad(window, index)) {
      SetWindowHasSeenGamepad(window, index);
      // This window hasn't seen this gamepad before, so
      // send a connection event first.
      NewConnectionEvent(index, true);
    }

    nsCOMPtr<nsIDOMDocument> domdoc;
    window->GetDocument(getter_AddRefs(domdoc));

    if (domdoc) {
      nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface(window);
      //Fire event
      FireButtonEvent(domdoc, target, index, button, pressed);
    }
  }
}

void
GamepadService::FireButtonEvent(nsIDOMDocument *domdoc,
                                nsIDOMEventTarget *target,
                                PRUint32 index,
                                PRUint32 button,
                                bool pressed)
{
  nsCOMPtr<nsIDOMEvent> event;
  bool defaultActionEnabled = true;
  domdoc->CreateEvent(NS_LITERAL_STRING("MozGamepadButtonEvent"),
                      getter_AddRefs(event));
  if (!event) {
    return;
  }

  nsCOMPtr<nsIDOMGamepadButtonEvent> je = do_QueryInterface(event);

  if (!je) {
    return;
  }

  nsString name = pressed ? NS_LITERAL_STRING("MozGamepadButtonDown") :
                            NS_LITERAL_STRING("MozGamepadButtonUp");
  je->InitGamepadButtonEvent(name, false, false, mGamepads[index], button);

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent = do_QueryInterface(event);
  if (privateEvent) {
    privateEvent->SetTrusted(PR_TRUE);
  }

  target->DispatchEvent(event, &defaultActionEnabled);
}

void
GamepadService::NewAxisMoveEvent(PRUint32 index, PRUint32 axis, float value) {
  if (index >= mGamepads.Length()) {
    return;
  }
  mGamepads[index]->SetAxis(axis, value);

  nsCOMPtr<nsIDOMWindow> window;
  if (GetFocusedWindow(getter_AddRefs(window))) {
    if (!WindowHasSeenGamepad(window, index)) {
      SetWindowHasSeenGamepad(window, index);
      // This window hasn't seen this gamepad before, so
      // send a connection event first.
      NewConnectionEvent(index, true);
    }

    nsCOMPtr<nsIDOMDocument> domdoc;
    window->GetDocument(getter_AddRefs(domdoc));

    if (domdoc) {
      nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface(window);
      //Fire event
      FireAxisMoveEvent(domdoc, target, index, axis, value);
    }
  }
}

void
GamepadService::FireAxisMoveEvent(nsIDOMDocument* domdoc,
                                  nsIDOMEventTarget* target,
                                  PRUint32 index,
                                  PRUint32 axis,
                                  float value)
{
  nsCOMPtr<nsIDOMEvent> event;
  bool defaultActionEnabled = true;
  domdoc->CreateEvent(NS_LITERAL_STRING("MozGamepadAxisMoveEvent"),
                      getter_AddRefs(event));
  if (!event) {
    return;
  }

  nsCOMPtr<nsIDOMGamepadAxisMoveEvent> je = do_QueryInterface(event);

  if (!je) {
    return;
  }

  je->InitGamepadAxisMoveEvent(NS_LITERAL_STRING("MozGamepadAxisMove"),
                                false, false, mGamepads[index], axis, value);

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent = do_QueryInterface(event);
  if (privateEvent)
    privateEvent->SetTrusted(PR_TRUE);

  target->DispatchEvent(event, &defaultActionEnabled);
}

void
GamepadService::NewConnectionEvent(PRUint32 index, bool connected)
{
  if (index >= mGamepads.Length()) {
    return;
  }

  if (connected) {
    nsCOMPtr<nsIDOMWindow> window;
    if (GetFocusedWindow(getter_AddRefs(window))) {
      // We don't fire a connected event here unless the window
      // has seen input from at least one device.
      PRUint32 data;
      bool hasSeenData = mListeners.Get(window, &data) && data != 0;
      if (connected && !hasSeenData) {
        return;
      }

      SetWindowHasSeenGamepad(window, index);

      nsCOMPtr<nsIDOMDocument> domdoc;
      window->GetDocument(getter_AddRefs(domdoc));

      if (domdoc) {
        nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface(window);
        // Fire event
        FireConnectionEvent(domdoc, target, index, connected);
      }
    }
  } else {
    // For disconnection events, fire one at every window that has received
    // data from this gamepad.
    mDisconnectingGamepad = index;
    mListeners.Enumerate(EnumerateForDisconnect, (void*)this);
  }
}

// static
PLDHashOperator
GamepadService::EnumerateForDisconnect(nsISupports* aKey,
                                       PRUint32& aData,
                                       void *userArg)
{
  GamepadService* self = reinterpret_cast<GamepadService*>(userArg);

  nsCOMPtr<nsIDOMWindow> window = do_QueryInterface(aKey);
  if (window &&
      self->WindowHasSeenGamepad(window, self->mDisconnectingGamepad)) {
    self->SetWindowHasSeenGamepad(window, self->mDisconnectingGamepad, false);

    nsCOMPtr<nsIDOMDocument> domdoc;
    window->GetDocument(getter_AddRefs(domdoc));

    if (domdoc) {
      nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface(window);
      // Fire event
      self->FireConnectionEvent(domdoc,
                                target,
                                self->mDisconnectingGamepad,
                                false);
    }
  }
  return PL_DHASH_NEXT;
}

void
GamepadService::FireConnectionEvent(nsIDOMDocument *domdoc,
                                    nsIDOMEventTarget *target,
                                    PRUint32 index,
                                    bool connected)
{
  nsCOMPtr<nsIDOMEvent> event;
  bool defaultActionEnabled = true;
  domdoc->CreateEvent(NS_LITERAL_STRING("MozGamepadConnectionEvent"),
                      getter_AddRefs(event));
  if (!event) {
    return;
  }

  nsCOMPtr<nsIDOMGamepadConnectionEvent> je = do_QueryInterface(event);

  if (!je) {
    return;
  }

  nsString name = connected ? NS_LITERAL_STRING("MozGamepadConnected") :
                              NS_LITERAL_STRING("MozGamepadDisconnected");
  je->InitGamepadConnectionEvent(name, false, false, mGamepads[index]);

  nsCOMPtr<nsIPrivateDOMEvent> privateEvent = do_QueryInterface(event);
  if (privateEvent) {
    privateEvent->SetTrusted(PR_TRUE);
  }

  target->DispatchEvent(event, &defaultActionEnabled);
}


// static
GamepadService* GamepadService::GetService() {
  if (!sSingleton) {
    sSingleton = CreateGamepadService();
  }
  return sSingleton;
}

bool
GamepadService::WindowHasSeenGamepad(nsIDOMWindow* aWindow, PRUint32 index)
{
  PRUint32 data;
  if (!mListeners.Get(aWindow, &data)) {
    // This window isn't even listening for gamepad events.
    return false;
  }

  return (data & (1 << index)) != 0;
}

void
GamepadService::SetWindowHasSeenGamepad(nsIDOMWindow* aWindow,
                                        PRUint32 index,
                                        bool hasSeen)
{
  PRUint32 data;
  if (!mListeners.Get(aWindow, &data))
    // This window isn't even listening for gamepad events.
    return;

  if (hasSeen) {
    data |= 1 << index;
  } else {
    data &= ~(1 << index);
  }
  mListeners.Put(aWindow, data);
}

bool
GamepadService::GetFocusedWindow(nsIDOMWindow** aWindow)
{
  nsCOMPtr<nsIDOMWindow> focusedWindow;
  if (NS_FAILED(mFocusManager->GetFocusedWindow(getter_AddRefs(focusedWindow)))) {
    return false;
  }

  nsCOMPtr<nsPIDOMWindow> outerWindow = do_QueryInterface(focusedWindow);
  if (!outerWindow) {
    return false;
  }
  nsCOMPtr<nsIDOMWindow> innerWindow = outerWindow->GetCurrentInnerWindow();
  innerWindow.forget(aWindow);
  return *aWindow;
}

// static
void
GamepadService::TimeoutHandler(nsITimer *aTimer, void *aClosure)
{
  // the reason that we use self, instead of just using nsITimerCallback or nsIObserver
  // is so that subclasses are free to use timers without worry about the base classes's
  // usage.
  GamepadService* self = reinterpret_cast<GamepadService*>(aClosure);
  if (!self) {
    NS_ERROR("no self");
    return;
  }

  if (self->mListeners.Count() == 0) {
    self->Shutdown();
    if (!self->mGamepads.IsEmpty()) {
      self->mGamepads.Clear();
    }
  }
}

void
GamepadService::StartCleanupTimer()
{
  if (mTimer) {
    mTimer->Cancel();
  }

  mTimer = do_CreateInstance("@mozilla.org/timer;1");
  if (mTimer) {
    mTimer->InitWithFuncCallback(TimeoutHandler,
                                 this,
                                 kCleanupDelayMS,
                                 nsITimer::TYPE_ONE_SHOT);
  }
}

} // namespace dom
} // namespace mozilla
