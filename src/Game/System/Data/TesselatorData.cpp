#include "System/Data/TesselatorData.hpp"

#include <cmath>
#include <cstring>
#include <math/seadMathCalcCommon.h>
#include <detail/aglGPUMemBlockMgr.h>
#include <nvn/nvn_FuncPtrInline.h>

/**
 * @brief Clear the border strips and the line counter without changing the X bounds.
 */
TesselatorData::TesselatorData() {
    mLeft.clear();
    mRight.clear();
    mTop.clear();
    mBottom.clear();
    mLine = 0;
}

/**
 * @brief Clear the border strips and the line counter without changing the X bounds.
 */
void TesselatorData::reset() {
    mLeft.clear();
    mRight.clear();
    mTop.clear();
    mBottom.clear();
    mLine = 0;
}

/**
 * @brief Advance to the next tessellation line.
 */
void TesselatorData::incrementLine() {
    ++mLine;
}

/**
 * @brief Set the current tessellation X position.
 * @param x Current X coordinate.
 */
void TesselatorData::setX(float x) {
    mX = x;
}

/**
 * @brief Set the tessellation X interval.
 * @param minX Lower X bound.
 * @param maxX Upper X bound.
 * @param step X sampling step.
 */
void TesselatorData::setXBounds(float minX, float maxX, float step) {
    mMinX = minX;
    mMaxX = maxX;
    mStep = step;
}

/**
 * @brief Read the current tessellation X coordinate.
 * @return The stored X coordinate.
 */
float TesselatorData::getX() const {
    return mX;
}

/**
 * @brief Read the lower X bound.
 * @return The lower X bound.
 */
float TesselatorData::getMinX() const {
    return mMinX;
}

/**
 * @brief Read the upper X bound.
 * @return The upper X bound.
 */
float TesselatorData::getMaxX() const {
    return mMaxX;
}

namespace {
/**
 * @brief Get the CPU mapping of a GPU memory block.
 * @param pBlock Block to map.
 * @return The first element of the block.
 */
template <typename T>
T* getBufferPtr(const agl::GPUMemBlock<T>* pBlock) {
    return reinterpret_cast<T*>(
        static_cast<u8*>(nvnMemoryPoolMap(pBlock->getMemoryPool()->getDriverPool())) +
        pBlock->getByteOffset());
}
}  // namespace

/**
 * @brief Construct the tesselator with empty border data.
 */
Tesselator::Tesselator() = default;

/**
 * @brief Attach the output buffers and restart the mesh.
 * @param pVertexBlock Vertex buffer to fill.
 * @param pIndexBlock Index buffer to fill.
 */
void Tesselator::prepare(agl::GPUMemBlock<OceanVertex>* pVertexBlock,
                         agl::GPUMemBlock<u32>* pIndexBlock) {
    mVertexBlock = pVertexBlock;
    mIndexBlock = pIndexBlock;
    mVertexNum = 0;
    mIndexNum = 0;
    _24 = 0;
    _28 = 0;
    _2c = 1;
    mData.reset();
}

/**
 * @brief Tessellate one cell, reusing the left border of the previous cell.
 * @param rData Cell corners, subdivision level and X position.
 */
void Tesselator::subdivide(const SubdivideData& rData) {
    TesselatorData in;
    in.copyLineState(mData);
    in.setX(rData.mX);
    in.setDepth(0);
    in.mLeft.copy(mData.mLeft);

    TesselatorData out;
    subdivideImpl(rData.mCorner0, rData.mCorner1, rData.mCorner2, rData.mCorner3,
                  rData.mLevel + 1, out, in, rData);
    mData.mLeft.copy(out.mRight);
}

/**
 * @brief Recursively split a quad into four quads and emit the leaves.
 * @param rCorner0 Corner at the minimum X and Z.
 * @param rCorner1 Corner at the minimum X and maximum Z.
 * @param rCorner2 Corner at the maximum X and minimum Z.
 * @param rCorner3 Corner at the maximum X and Z.
 * @param depth Remaining subdivision depth.
 * @param rOut Receives the right border of the quad.
 * @param rIn Left border and line state of the quad.
 * @param rData The original subdivision request.
 */
void Tesselator::subdivideImpl(const sead::Vector3f& rCorner0, const sead::Vector3f& rCorner1,
                               const sead::Vector3f& rCorner2, const sead::Vector3f& rCorner3,
                               u32 depth, TesselatorData& rOut, const TesselatorData& rIn,
                               const SubdivideData& rData) {
    if (depth == 0) {
        createQuad(rCorner0, rCorner1, rCorner2, rCorner3, rOut, rIn);
        return;
    }

    u32 level = rIn.getDepth();
    u32 nextLevel = level + 1;
    f32 maxX = rIn.getMaxX();
    s32 line = rIn.getLine();
    f32 step = rIn.getStep();
    f32 minX = rIn.getMinX();
    f32 x = rIn.getX();

    if (level == 0) {
        TesselatorData child;
        TesselatorData next;
        next.setXBounds(minX, maxX, step);
        next.setLine(line);
        next.setX(x);
        next.setDepth(nextLevel);
        next.mLeft.copy(rIn.mLeft);
        subdivideImpl(rCorner0, rCorner1, rCorner2, rCorner3, depth - 1, child, next, rData);
        rOut.mRight.copy(child.mRight);
        return;
    }

    f32 halfWidth = (rCorner2.x - rCorner0.x) * 0.5f;
    f32 halfDepth = (rCorner1.z - rCorner0.z) * 0.5f;

    for (u32 row = 0; row < 2; row++) {
        TesselatorData child;
        child.mRight.setRun(rIn.mLeft, row);

        for (u32 column = 0; column < 2; column++) {
            sead::Vector3f corner0(rCorner0.x + halfWidth * column, rCorner0.y,
                                   rCorner0.z + halfDepth * row);
            sead::Vector3f corner1(corner0.x, rCorner0.y, corner0.z + halfDepth);
            sead::Vector3f corner2(corner0.x + halfWidth, rCorner0.y, corner0.z);
            sead::Vector3f corner3(corner0.x + halfWidth, rCorner0.y, corner0.z + halfDepth);

            TesselatorData next;
            next.mLeft.copy(child.mRight);
            child.mRight.clear();
            next.setDepth(nextLevel);
            subdivideImpl(corner0, corner1, corner2, corner3, depth - 1, child, next, rData);
        }

        rOut.mRight.append(child.mRight);
    }
}

/**
 * @brief Tessellate the area between two points (unused).
 * @param rMin Minimum corner.
 * @param rMax Maximum corner.
 * @param depth Subdivision depth.
 */
void Tesselator::subdivide(const sead::Vector3f& rMin, const sead::Vector3f& rMax, u32 depth) {}

/**
 * @brief Move to the next line of cells.
 * @param minX Lower X bound of the new line.
 * @param maxX Upper X bound of the new line.
 */
void Tesselator::moveUp(float minX, float maxX) {
    mData.setXBounds(minX, maxX, 1.0f);
    mData.incrementLine();
    mData.mLeft.clear();
}

/**
 * @brief Level-limited subdivision (unused).
 * @param rCorner0 Corner at the minimum X and Z.
 * @param rCorner1 Corner at the minimum X and maximum Z.
 * @param rCorner2 Corner at the maximum X and minimum Z.
 * @param rCorner3 Corner at the maximum X and Z.
 * @param depth Remaining subdivision depth.
 * @param level Current subdivision level.
 * @param rOut Receives the right border of the quad.
 * @param rIn Left border and line state of the quad.
 */
void Tesselator::subdivideImpl(const sead::Vector3f& rCorner0, const sead::Vector3f& rCorner1,
                               const sead::Vector3f& rCorner2, const sead::Vector3f& rCorner3,
                               u32 depth, u32 level, TesselatorData& rOut,
                               const TesselatorData& rIn) {}

/**
 * @brief Test a sphere against a quad (always hits).
 * @param rPos Sphere center.
 * @param radius Sphere radius.
 * @param rCorner0 First quad corner.
 * @param rCorner1 Second quad corner.
 * @param rCorner2 Third quad corner.
 * @param rCorner3 Fourth quad corner.
 * @return Always true.
 */
bool isCollision(const sead::Vector3f& rPos, float radius, const sead::Vector3f& rCorner0,
                 const sead::Vector3f& rCorner1, const sead::Vector3f& rCorner2,
                 const sead::Vector3f& rCorner3) {
    return true;
}

/**
 * @brief Emit the two triangles of a leaf quad, sharing vertices with its neighbours.
 * @param rCorner0 Corner at the minimum X and Z.
 * @param rCorner1 Corner at the minimum X and maximum Z.
 * @param rCorner2 Corner at the maximum X and minimum Z.
 * @param rCorner3 Corner at the maximum X and Z.
 * @param rOut Receives the right and top edges of the quad.
 * @param rIn Left and bottom edges shared with the neighbouring quads.
 */
void Tesselator::createQuad(const sead::Vector3f& rCorner0, const sead::Vector3f& rCorner1,
                            const sead::Vector3f& rCorner2, const sead::Vector3f& rCorner3,
                            TesselatorData& rOut, const TesselatorData& rIn) {
    const TesselatorEdgeStrip& rLeft = rIn.mLeft;
    const TesselatorLineStrip& rBottom = rIn.mBottom;
    u32 index0;
    u32 index1;
    u32 index2;
    u32 index3;

    if (rLeft.mNum != 0 && rBottom.mNum == 0) {
        index0 = rLeft.mIndices[0];

        if (rLeft.mReuseNum != 0) {
            rLeft.mReuseNum--;
            index1 = createVertex(rCorner1);
        } else {
            index1 = rLeft.mIndices[rLeft.mNum - 1];
        }

        index2 = createVertex(rCorner2);
        index3 = createVertex(rCorner3);
    } else if (rLeft.mNum == 0 && rBottom.mNum == 0) {
        index0 = createVertex(rCorner0);
        index1 = createVertex(rCorner1);
        index2 = createVertex(rCorner2);
        index3 = createVertex(rCorner3);
    } else {
        if (rIn.getLine() == 0) {
            return;
        }

        f32 x = rIn.getX();
        f32 minX = rIn.getMinX();
        f32 maxX = rIn.getMaxX();
        f32 step = rIn.getStep();

        if (x + step < minX || x - step > maxX) {
            if (rLeft.mNum != 0) {
                index0 = rLeft.mIndices[0];
                index1 = rLeft.mIndices[1];
            } else {
                index0 = createVertex(rCorner0);
                index1 = createVertex(rCorner1);
            }

            index2 = createVertex(rCorner2);
            index3 = createVertex(rCorner3);
        } else if (x + step == minX) {
            if (rLeft.mNum != 0) {
                index0 = rLeft.mIndices[0];
                index1 = rLeft.mIndices[1];
            } else {
                index0 = createVertex(rCorner0);
                index1 = createVertex(rCorner1);
            }

            index2 = rBottom.mIndices[0];
            index3 = createVertex(rCorner3);
        } else if (x >= minX && x <= maxX) {
            index0 = rBottom.mIndices[0];

            if (rLeft.mNum != 0) {
                index1 = rLeft.mIndices[1];
            } else {
                index1 = createVertex(rCorner1);
            }

            index2 = rBottom.mIndices[1];
            index3 = createVertex(rCorner3);
        } else if (x - step == maxX) {
            index0 = rBottom.mIndices[1];

            if (rLeft.mNum != 0) {
                index1 = rLeft.mIndices[1];
            } else {
                index1 = createVertex(rCorner1);
            }

            index2 = createVertex(rCorner2);
            index3 = createVertex(rCorner3);
        } else {
            return;
        }
    }

    createTriangle(index0, index1, index3);
    createTriangle(index0, index3, index2);
    rOut.mRight.push(index2);
    rOut.mRight.push(index3);
    rOut.mTop.push(index1);
    rOut.mTop.push(index3);
}

/**
 * @brief Append a vertex to the vertex buffer.
 * @param rPos Vertex position.
 * @return Index of the new vertex.
 */
u32 Tesselator::createVertex(const sead::Vector3f& rPos) {
    s32 index = mVertexNum++;
    OceanVertex& rVertex = getBufferPtr(mVertexBlock)[index];
    rVertex.mPos = rPos;
    rVertex.mColor = sead::Color4f::cBlack;
    rVertex.mNormal.set(0.0f, 1.0f, 0.0f);
    return index;
}

/**
 * @brief Append one entry to the index buffer.
 * @param index Vertex index to write.
 */
inline void Tesselator::addIndex(u32 index) {
    agl::GPUMemBlock<u32>* pBlock = mIndexBlock;
    s32 position = mIndexNum++;
    getBufferPtr(pBlock)[position] = index;
}

/**
 * @brief Append a triangle to the index buffer and brighten its vertices.
 * @param index0 First vertex index.
 * @param index1 Second vertex index.
 * @param index2 Third vertex index.
 */
void Tesselator::createTriangle(u32 index0, u32 index1, u32 index2) {
    addIndex(index0);
    addIndex(index1);
    addIndex(index2);
    getBufferPtr(mVertexBlock)[static_cast<s32>(index0)].mColor += 0.2f;
    getBufferPtr(mVertexBlock)[static_cast<s32>(index1)].mColor += 0.2f;
    getBufferPtr(mVertexBlock)[static_cast<s32>(index2)].mColor += 0.2f;
}

/**
 * @brief Copy the bottom-edge run of the neighbouring cell that lines up with X.
 * @param rOut Receives the bottom edge.
 * @param rIn Data holding the previous line's top edges.
 * @param x Current X coordinate.
 * @param minX Lower X bound.
 * @param maxX Upper X bound.
 * @param step X sampling step.
 */
void Tesselator::selectiveCopy(TesselatorData& rOut, const TesselatorData& rIn, float x,
                               float minX, float maxX, float step) {
    if (x + step < minX) {
        return;
    }

    if (x + step == minX) {
        rOut.mBottom.setRun(rIn.mBottom, 0);
    } else if (x >= minX && x <= maxX) {
        rOut.mBottom.setRun(rIn.mBottom, std::fabs(x - minX));
    } else if (x - step == maxX) {
        rOut.mBottom.setRun(rIn.mBottom, std::fabs(maxX - minX));
    }
}
