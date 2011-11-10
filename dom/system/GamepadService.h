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

#ifndef mozilla_dom_GamepadService_h_
#define mozilla_dom_GamepadService_h_

#include "nsAutoPtr.h"
#include "nsCOMArray.h"
#include "nsDataHashtable.h"

class nsIDOMWindow;
class nsIDOMDocument;
class nsIDOMEventTarget;
class nsIFocusManager;
class nsDOMGamepad;
class nsITimer;

namespace mozilla {
namespace dom {

class GamepadService {
 public:
  // Get the singleton service
  static GamepadService* GetService();

  // Indicate that |aWindow| wants to receive gamepad events.
  void AddListener(nsIDOMWindow* aWindow);
  // Indicate that |aWindow| should no longer receive gamepad events.
  void RemoveListener(nsIDOMWindow* aWindow);

  // Add a gamepad to the list of known gamepads, and return its index.
  PRUint32 AddGamepad(const char* id, PRUint32 numButtons, PRUint32 numAxes);
  // Remove the gamepad at |index| from the list of known gamepads.
  void RemoveGamepad(PRUint32 index);

  void NewButtonEvent(PRUint32 index, PRUint32 button, bool pressed);
  void FireButtonEvent(nsIDOMDocument* domdoc,
                       nsIDOMEventTarget* target,
                       PRUint32 index,
                       PRUint32 button,
                       bool pressed);
  void NewAxisMoveEvent(PRUint32  index, PRUint32 axis, float value);
  void FireAxisMoveEvent(nsIDOMDocument* domdoc,
                         nsIDOMEventTarget* target,
                         PRUint32 index,
                         PRUint32 axis,
                         float value);

  void NewConnectionEvent(PRUint32 index, bool connected);
  void FireConnectionEvent(nsIDOMDocument* domdoc,
                           nsIDOMEventTarget* target,
                           PRUint32 index,
                           bool connected);

 protected:
  GamepadService();
  virtual ~GamepadService();
  virtual void Startup() = 0;
  virtual void Shutdown() = 0;
  void StartCleanupTimer();

  bool mStarted;

 private:
  // Returns true if we have already sent data from this gamepad
  // to this window. This should only return true if the user
  // explicitly interacted with a gamepad while this window
  // was focused, by pressing buttons or similar actions.
  bool WindowHasSeenGamepad(nsIDOMWindow* window, PRUint32 index);
  // Indicate that a window has recieved data from a gamepad.
  void SetWindowHasSeenGamepad(nsIDOMWindow* window, PRUint32 index,
                               bool hasSeen = true);

  bool GetFocusedWindow(nsIDOMWindow** window);
  static PLDHashOperator EnumerateForDisconnect(nsISupports* aKey,
                                                PRUint32& aData,
                                                void* userArg);

  static void TimeoutHandler(nsITimer* aTimer, void* aClosure);
  static GamepadService* sSingleton;

  // Gamepads connected to the system.
  nsTArray<nsRefPtr<nsDOMGamepad> > mGamepads;
  // This hashtable is keyed by nsIDOMWindows that are listening
  // for gamepad events. The data is a bitmap of gamepad numbers.
  // A bit in the bitmap is set if this window has received data
  // for that gamepad.
  nsDataHashtable<nsISupportsHashKey, PRUint32> mListeners;
  nsCOMPtr<nsITimer> mTimer;
  nsCOMPtr<nsIFocusManager> mFocusManager;
  // Used for convenience when enumerating mListeners entries.
  int mDisconnectingGamepad;
};

}
}

#endif // mozilla_dom_GamepadService_h_
