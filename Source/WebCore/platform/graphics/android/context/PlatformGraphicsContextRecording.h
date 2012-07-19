/*
 * Copyright 2012, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef platform_graphics_context_recording_h
#define platform_graphics_context_recording_h

#include "PlatformGraphicsContext.h"

#include "GraphicsOperationCollection.h"
#include "SkRefCnt.h"

class SkCanvas;

namespace WebCore {
namespace GraphicsOperation {
class Operation;
class Save;
}

class RecordingImpl;
class Recording : public SkRefCnt {
public:
    Recording()
        : m_recording(0)
    {}
    ~Recording();

    void draw(SkCanvas* canvas);
    void setRecording(RecordingImpl* impl);
    RecordingImpl* recording() { return m_recording; }

private:
    RecordingImpl* m_recording;
};

class PlatformGraphicsContextRecording : public PlatformGraphicsContext {
public:
    PlatformGraphicsContextRecording(Recording* picture);
    virtual ~PlatformGraphicsContextRecording() {}
    virtual bool isPaintingDisabled();
    virtual SkCanvas* getCanvas() { return 0; }

    virtual SkCanvas* recordingCanvas();
    virtual void endRecording(const SkRect& bounds);

    virtual ContextType type() { return RecordingContext; }

    // State management
    virtual void beginTransparencyLayer(float opacity);
    virtual void endTransparencyLayer();
    virtual void save();
    virtual void restore();

    // State values
    virtual void setAlpha(float alpha);
    virtual void setCompositeOperation(CompositeOperator op);
    virtual bool setFillColor(const Color& c);
    virtual bool setFillShader(SkShader* fillShader);
    virtual void setLineCap(LineCap cap);
    virtual void setLineDash(const DashArray& dashes, float dashOffset);
    virtual void setLineJoin(LineJoin join);
    virtual void setMiterLimit(float limit);
    virtual void setShadow(int radius, int dx, int dy, SkColor c);
    virtual void setShouldAntialias(bool useAA);
    virtual bool setStrokeColor(const Color& c);
    virtual bool setStrokeShader(SkShader* strokeShader);
    virtual void setStrokeStyle(StrokeStyle style);
    virtual void setStrokeThickness(float f);

    // Matrix operations
    virtual void concatCTM(const AffineTransform& affine);
    virtual void rotate(float angleInRadians);
    virtual void scale(const FloatSize& size);
    virtual void translate(float x, float y);
    virtual const SkMatrix& getTotalMatrix();

    // Clipping
    virtual void addInnerRoundedRectClip(const IntRect& rect, int thickness);
    virtual void canvasClip(const Path& path);
    virtual bool clip(const FloatRect& rect);
    virtual bool clip(const Path& path);
    virtual bool clipConvexPolygon(size_t numPoints, const FloatPoint*, bool antialias);
    virtual bool clipOut(const IntRect& r);
    virtual bool clipOut(const Path& p);
    virtual bool clipPath(const Path& pathToClip, WindRule clipRule);

    // Drawing
    virtual void clearRect(const FloatRect& rect);
    virtual void drawBitmapPattern(const SkBitmap& bitmap, const SkMatrix& matrix,
                           CompositeOperator compositeOp, const FloatRect& destRect);
    virtual void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                        const SkRect& dst, CompositeOperator op);
    virtual void drawConvexPolygon(size_t numPoints, const FloatPoint* points,
                           bool shouldAntialias);
    virtual void drawEllipse(const IntRect& rect);
    virtual void drawFocusRing(const Vector<IntRect>& rects, int /* width */,
                       int /* offset */, const Color& color);
    virtual void drawHighlightForText(const Font& font, const TextRun& run,
                              const FloatPoint& point, int h,
                              const Color& backgroundColor, ColorSpace colorSpace,
                              int from, int to, bool isActive);
    virtual void drawLine(const IntPoint& point1, const IntPoint& point2);
    virtual void drawLineForText(const FloatPoint& pt, float width);
    virtual void drawLineForTextChecking(const FloatPoint& pt, float width,
                                         GraphicsContext::TextCheckingLineStyle);
    virtual void drawRect(const IntRect& rect);
    virtual void fillPath(const Path& pathToFill, WindRule fillRule);
    virtual void fillRect(const FloatRect& rect);
    virtual void fillRect(const FloatRect& rect, const Color& color);
    virtual void fillRoundedRect(const IntRect& rect, const IntSize& topLeft,
                         const IntSize& topRight, const IntSize& bottomLeft,
                         const IntSize& bottomRight, const Color& color);
    virtual void strokeArc(const IntRect& r, int startAngle, int angleSpan);
    virtual void strokePath(const Path& pathToStroke);
    virtual void strokeRect(const FloatRect& rect, float lineWidth);

private:

    virtual bool shadowsIgnoreTransforms() const {
        return false;
    }

    void clipState(const FloatRect& clip);
    void appendDrawingOperation(GraphicsOperation::Operation* operation, const FloatRect& bounds);
    void appendStateOperation(GraphicsOperation::Operation* operation);
    void onCurrentMatrixChanged();
    void pushSaveOperation(GraphicsOperation::Save* saveOp);
    void popSaveOperation();
    void pushMatrix();
    void popMatrix();

    SkPicture* mPicture;
    SkMatrix* mCurrentMatrix;
    // Used for getTotalMatrix, is not valid elsewhere
    SkMatrix mTotalMatrix;

    Recording* mRecording;
    class RecordingState {
    public:
        RecordingState(GraphicsOperation::Save* saveOp)
            : mSaveOperation(saveOp)
            , mHasDrawing(false)
            , mHasClip(false)
        {}

        RecordingState(const RecordingState& other)
            : mSaveOperation(other.mSaveOperation)
            , mHasDrawing(other.mHasDrawing)
            , mHasClip(other.mHasClip)
            , mBounds(other.mBounds)
        {}

        void addBounds(const FloatRect& bounds)
        {
            if (mHasClip)
                return;
            mBounds.unite(bounds);
        }

        void clip(const FloatRect& rect)
        {
            addBounds(rect);
            mHasClip = true;
        }

        GraphicsOperation::Save* mSaveOperation;
        bool mHasDrawing;
        bool mHasClip;
        FloatRect mBounds;
    };
    Vector<RecordingState> mRecordingStateStack;
    Vector<SkMatrix> mMatrixStack;
    State* mOperationState;
    SkMatrix* mOperationMatrix;
};

}
#endif
