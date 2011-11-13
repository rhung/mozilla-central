#include "nsMouseLockable.h"
#include "nsContentUtils.h"

DOMCI_DATA(MouseLockable, nsDOMMouseLockable)

NS_INTERFACE_MAP_BEGIN(nsDOMMouseLockable)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMouseLockable)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(MouseLockable)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsDOMMouseLockable)
NS_IMPL_RELEASE(nsDOMMouseLockable)

nsDOMMouseLockable::nsDOMMouseLockable() :
  mIsLocked(PR_FALSE)
{
}

nsDOMMouseLockable::~nsDOMMouseLockable()
{
}

/* void unlock (); */
NS_IMETHODIMP nsDOMMouseLockable::Unlock()
{
  mIsLocked = PR_TRUE;
  return NS_OK;
}

/* bool islocked (); */
NS_IMETHODIMP nsDOMMouseLockable::Islocked(bool *_retval NS_OUTPARAM)
{
  *_retval = mIsLocked;
  return NS_OK;
}
