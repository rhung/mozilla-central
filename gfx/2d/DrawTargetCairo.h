/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla Corporation code.
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef _MOZILLA_GFX_DRAWTARGET_CAIRO_H_
#define _MOZILLA_GFX_DRAWTARGET_CAIRO_H_

#include "2D.h"
#include "cairo.h"
#include "PathCairo.h"

#include <vector>

namespace mozilla {
namespace gfx {

class SourceSurfaceCairo;

class GradientStopsCairo : public GradientStops
{
  public:
    GradientStopsCairo(GradientStop* aStops, uint32_t aNumStops)
    {
      for (uint32_t i = 0; i < aNumStops; ++i) {
        mStops.push_back(aStops[i]);
      }
    }

    virtual ~GradientStopsCairo() {}

    const std::vector<GradientStop>& GetStops() const
    {
      return mStops;
    }

    virtual BackendType GetBackendType() const { return BACKEND_CAIRO; }

  private:
    std::vector<GradientStop> mStops;
};

class DrawTargetCairo : public DrawTarget
{
public:
  DrawTargetCairo();
  virtual ~DrawTargetCairo();

  virtual BackendType GetType() const { return BACKEND_CAIRO; }
  virtual TemporaryRef<SourceSurface> Snapshot();
  virtual IntSize GetSize();

  virtual void Flush();
  virtual void DrawSurface(SourceSurface *aSurface,
                           const Rect &aDest,
                           const Rect &aSource,
                           const DrawSurfaceOptions &aSurfOptions = DrawSurfaceOptions(),
                           const DrawOptions &aOptions = DrawOptions());
  virtual void DrawSurfaceWithShadow(SourceSurface *aSurface,
                                     const Point &aDest,
                                     const Color &aColor,
                                     const Point &aOffset,
                                     Float aSigma,
                                     CompositionOp aOperator);

  virtual void ClearRect(const Rect &aRect);

  virtual void CopySurface(SourceSurface *aSurface,
                           const IntRect &aSourceRect,
                           const IntPoint &aDestination);

  virtual void FillRect(const Rect &aRect,
                        const Pattern &aPattern,
                        const DrawOptions &aOptions = DrawOptions());
  virtual void StrokeRect(const Rect &aRect,
                          const Pattern &aPattern,
                          const StrokeOptions &aStrokeOptions = StrokeOptions(),
                          const DrawOptions &aOptions = DrawOptions());
  virtual void StrokeLine(const Point &aStart,
                          const Point &aEnd,
                          const Pattern &aPattern,
                          const StrokeOptions &aStrokeOptions = StrokeOptions(),
                          const DrawOptions &aOptions = DrawOptions());

  virtual void Stroke(const Path *aPath,
                      const Pattern &aPattern,
                      const StrokeOptions &aStrokeOptions = StrokeOptions(),
                      const DrawOptions &aOptions = DrawOptions());

  virtual void Fill(const Path *aPath,
                    const Pattern &aPattern,
                    const DrawOptions &aOptions = DrawOptions());

  virtual void FillGlyphs(ScaledFont *aFont,
                          const GlyphBuffer &aBuffer,
                          const Pattern &aPattern,
                          const DrawOptions &aOptions);
  virtual void Mask(const Pattern &aSource,
                    const Pattern &aMask,
                    const DrawOptions &aOptions = DrawOptions());

  virtual void PushClip(const Path *aPath);
  virtual void PushClipRect(const Rect &aRect);
  virtual void PopClip();

  virtual TemporaryRef<PathBuilder> CreatePathBuilder(FillRule aFillRule = FILL_WINDING) const;

  virtual TemporaryRef<SourceSurface> CreateSourceSurfaceFromData(unsigned char *aData,
                                                            const IntSize &aSize,
                                                            int32_t aStride,
                                                            SurfaceFormat aFormat) const;
  virtual TemporaryRef<SourceSurface> OptimizeSourceSurface(SourceSurface *aSurface) const;
  virtual TemporaryRef<SourceSurface>
    CreateSourceSurfaceFromNativeSurface(const NativeSurface &aSurface) const;
  virtual TemporaryRef<DrawTarget>
    CreateSimilarDrawTarget(const IntSize &aSize, SurfaceFormat aFormat) const;

  virtual TemporaryRef<GradientStops>
    CreateGradientStops(GradientStop *aStops,
                        uint32_t aNumStops,
                        ExtendMode aExtendMode = EXTEND_CLAMP) const;

  virtual void *GetNativeSurface(NativeSurfaceType aType);

  bool Init(cairo_surface_t* aSurface);

  void SetPathObserver(CairoPathContext* aPathObserver);

  virtual void SetTransform(const Matrix& aTransform);

  // Call to set up aContext for drawing (with the current transform, etc).
  // Pass the path you're going to be using if you have one.
  // Implicitly calls WillChange(aPath).
  void PrepareForDrawing(cairo_t* aContext, const Path* aPath = NULL);

private: // methods
  enum DrawPatternType { DRAW_FILL, DRAW_STROKE };
  void DrawPattern(const Pattern& aPattern,
                   const StrokeOptions& aStrokeOptions,
                   const DrawOptions& aOptions,
                   DrawPatternType aDrawType);

  // Copy-on-write support for snapshot surfaces.
  friend class SourceSurfaceCairo;
  void AppendSnapshot(SourceSurfaceCairo* aSnapshot);
  void RemoveSnapshot(SourceSurfaceCairo* aSnapshot);

  // Call before you make any changes to the backing surface with which this
  // context is associated. Pass the path you're going to be using if you have
  // one.
  void WillChange(const Path* aPath = NULL);

  // Call if there is any reason to disassociate all snapshots from this draw
  // target; for example, because we're going to be destroyed.
  void MarkSnapshotsIndependent();

private: // data
  cairo_t* mContext;
  std::vector<SourceSurfaceCairo*> mSnapshots;
  mutable RefPtr<CairoPathContext> mPathObserver;
};

}
}

#endif // _MOZILLA_GFX_DRAWTARGET_CAIRO_H_
