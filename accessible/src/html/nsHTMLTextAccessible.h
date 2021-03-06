/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Aaron Leventhal (aaronl@netscape.com)
 *   Kyle Yuan (kyle.yuan@sun.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef _nsHTMLTextAccessible_H_
#define _nsHTMLTextAccessible_H_

#include "nsTextAccessibleWrap.h"
#include "nsAutoPtr.h"
#include "nsBaseWidgetAccessible.h"

/**
 * Used for text nodes within HTML document.
 */
class nsHTMLTextAccessible : public nsTextAccessibleWrap
{
public:
  nsHTMLTextAccessible(nsIContent *aContent, nsIWeakReference *aShell);

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIAccessible
  NS_IMETHOD GetName(nsAString& aName);

  // nsAccessible
  virtual nsresult GetAttributesInternal(nsIPersistentProperties *aAttributes);
  virtual mozilla::a11y::role NativeRole();
  virtual PRUint64 NativeState();
};

/**
 * Used for HTML hr element.
 */
class nsHTMLHRAccessible : public nsLeafAccessible
{
public:
  nsHTMLHRAccessible(nsIContent *aContent, nsIWeakReference *aShell);

  // nsAccessible
  virtual mozilla::a11y::role NativeRole();
};

/**
 * Used for HTML br element.
 */
class nsHTMLBRAccessible : public nsLeafAccessible
{
public:
  nsHTMLBRAccessible(nsIContent *aContent, nsIWeakReference *aShell);

  // nsAccessible
  virtual nsresult GetNameInternal(nsAString& aName);
  virtual mozilla::a11y::role NativeRole();
  virtual PRUint64 NativeState();
};

/**
 * Used for HTML label element.
 */
class nsHTMLLabelAccessible : public nsHyperTextAccessibleWrap
{
public:
  nsHTMLLabelAccessible(nsIContent *aContent, nsIWeakReference *aShell);

  NS_DECL_ISUPPORTS_INHERITED

  // nsAccessible
  virtual nsresult GetNameInternal(nsAString& aName);
  virtual mozilla::a11y::role NativeRole();
};

/**
 * Used for HTML output element.
 */
class nsHTMLOutputAccessible : public nsHyperTextAccessibleWrap
{
public:
  nsHTMLOutputAccessible(nsIContent* aContent, nsIWeakReference* aShell);

  NS_DECL_ISUPPORTS_INHERITED

  // nsAccessible
  virtual mozilla::a11y::role NativeRole();
  virtual nsresult GetAttributesInternal(nsIPersistentProperties* aAttributes);
  virtual Relation RelationByType(PRUint32 aType);
};

/**
 * Used for bullet of HTML list item element (for example, HTML li).
 */
class nsHTMLListBulletAccessible : public nsLeafAccessible
{
public:
  nsHTMLListBulletAccessible(nsIContent* aContent, nsIWeakReference* aShell);

  // nsIAccessible
  NS_IMETHOD GetName(nsAString& aName);

  // nsAccessNode
  virtual bool IsPrimaryForNode() const;

  // nsAccessible
  virtual mozilla::a11y::role NativeRole();
  virtual PRUint64 NativeState();
  virtual void AppendTextTo(nsAString& aText, PRUint32 aStartOffset = 0,
                            PRUint32 aLength = PR_UINT32_MAX);
};

/**
 * Used for HTML list (like HTML ul).
 */
class nsHTMLListAccessible : public nsHyperTextAccessibleWrap
{
public:
  nsHTMLListAccessible(nsIContent *aContent, nsIWeakReference *aShell);

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsAccessible
  virtual mozilla::a11y::role NativeRole();
  virtual PRUint64 NativeState();
};

/**
 * Used for HTML list item (e.g. HTML li).
 */
class nsHTMLLIAccessible : public nsHyperTextAccessibleWrap
{
public:
  nsHTMLLIAccessible(nsIContent* aContent, nsIWeakReference* aShell);

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsAccessNode
  virtual void Shutdown();

  // nsIAccessible
  NS_IMETHOD GetBounds(PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height);

  // nsAccessible
  virtual mozilla::a11y::role NativeRole();
  virtual PRUint64 NativeState();

  // nsHTMLLIAccessible
  void UpdateBullet(bool aHasBullet);

protected:
  // nsAccessible
  virtual void CacheChildren();

private:
  nsRefPtr<nsHTMLListBulletAccessible> mBullet;
};

inline nsHTMLLIAccessible*
nsAccessible::AsHTMLListItem()
{
  return mFlags & eHTMLListItemAccessible ?
    static_cast<nsHTMLLIAccessible*>(this) : nsnull;
}

#endif
