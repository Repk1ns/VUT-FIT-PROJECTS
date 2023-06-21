/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Vojtech Mimochodek <xmimoc01@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    13.12.2021
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}


unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    unsigned totalTriangles = 0;

    #pragma omp parallel
    {
        #pragma omp single
        {
            totalTriangles = marchCubesGridCheck(mGridSize, field, Vec3_t<float>());
        }
    }

    return totalTriangles;    
}


unsigned TreeMeshBuilder::marchCubesGridCheck(size_t gridSize, const ParametricScalarField &field, const Vec3_t<float> &cubeOffset)
{
    if (gridSize < 2) {
        return buildCube(cubeOffset, field);
    }

    float condition = field.getIsoLevel() + (sqrt(3)/2) * (gridSize * mGridResolution);

    Vec3_t<float> middle(mGridResolution * (gridSize/2 + cubeOffset.x), mGridResolution * (gridSize/2 + cubeOffset.y), mGridResolution * (gridSize/2 + cubeOffset.z));

    float middleFieldValue = evaluateFieldAt(middle, field);

    if (middleFieldValue > condition) {
        return 0;
    }

    unsigned totalTriangles = 0;

    /*
     * 
     * RECURSIVE CHECK 8 CHILD CUBES
     * 
     */

    // X - 0 ; Y - 0 ; Z - 0
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 0 * (gridSize / 2), cubeOffset.y + 0 * (gridSize / 2), cubeOffset.z + 0 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize/2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 1 ; Y - 0 ; Z - 0
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 1 * (gridSize / 2), cubeOffset.y + 0 * (gridSize / 2), cubeOffset.z + 0 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 0 ; Y - 1 ; Z - 0
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 0 * (gridSize / 2), cubeOffset.y + 1 * (gridSize / 2), cubeOffset.z + 0 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 0 ; Y - 0 ; Z - 1
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 0 * (gridSize / 2), cubeOffset.y + 0 * (gridSize / 2), cubeOffset.z + 1 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 1 ; Y - 1 ; Z - 0
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 1 * (gridSize / 2), cubeOffset.y + 1 * (gridSize / 2), cubeOffset.z + 0 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 0 ; Y - 1 ; Z - 1
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 0 * (gridSize / 2), cubeOffset.y + 1 * (gridSize / 2), cubeOffset.z + 1 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 1 ; Y - 0 ; Z - 1
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 1 * (gridSize / 2), cubeOffset.y + 0 * (gridSize / 2), cubeOffset.z + 1 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    // X - 1 ; Y - 1 ; Z - 1
    #pragma omp task shared(totalTriangles)
    {
        Vec3_t<float> actualCubeOffset(cubeOffset.x + 1 * (gridSize / 2), cubeOffset.y + 1 * (gridSize / 2), cubeOffset.z + 1 * (gridSize / 2));

        unsigned childResult = marchCubesGridCheck(gridSize / 2, field, actualCubeOffset);

        #pragma omp atomic
        totalTriangles = totalTriangles + childResult;
    }

    #pragma omp taskwait
    {
        return totalTriangles;
    }
}


float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        value = std::min(value, distanceSquared);
    }

    return sqrt(value);
}


void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    #pragma omp critical
    {
        mTriangles.push_back(triangle);
    }
}
