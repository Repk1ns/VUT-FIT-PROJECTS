/**
 * @file    tree_mesh_builder.h
 *
 * @author  Vojtech Mimochodek <xmimoc01@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    13.12.2021
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include "base_mesh_builder.h"

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

protected:
    std::vector<Triangle_t> mTriangles;

    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }

    unsigned marchCubesGridCheck(size_t gridSize, const ParametricScalarField &field, const Vec3_t<float> &cubeOffset);
};

#endif // TREE_MESH_BUILDER_H
