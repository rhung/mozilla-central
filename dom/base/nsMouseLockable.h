#ifndef nsDOMMouseLockable_h___
#define nsDOMMouseLockable_h___

#include "nsIDOMMouseLockable.h"

class nsDOMMouseLockable : public nsIDOMMouseLockable
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMMOUSELOCKABLE

  nsDOMMouseLockable();

private:
  ~nsDOMMouseLockable();
  bool mIsLocked;

protected:
  /* additional members */
};

#endif /* nsDOMMouseLockable_h___ */
