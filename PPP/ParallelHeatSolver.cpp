/**
 * @file    ParallelHeatSolver.cpp
 *
 * @author  Name Surname <xmimoc01@fit.vutbr.cz>
 *
 * @brief   Course: PPP 2023/2024 - Project 1
 *          This file contains implementation of parallel heat equation solver
 *          using MPI/OpenMP hybrid approach.
 *
 * @date    2024-02-23
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <ios>
#include <string_view>

#include "ParallelHeatSolver.hpp"

ParallelHeatSolver::ParallelHeatSolver(const SimulationProperties &simulationProps,
                                       const MaterialProperties &materialProps)
    : HeatSolverBase(simulationProps, materialProps)
{
  MPI_Comm_size(MPI_COMM_WORLD, &mWorldSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &mWorldRank);

  mSimulationProps.getDecompGrid(tilesInCol, tilesInRow);
  domainWidth = mMaterialProps.getEdgeSize();
  tileWidth = domainWidth / tilesInRow;
  tileHeight = domainWidth / tilesInCol;
  tileOffsets = 4;
  tileOffset = 2;
  haloOffset = 2;

  temperatures.resize(domainWidth * domainWidth);
  domainMap.resize(domainWidth * domainWidth);
  domainParameters.resize(domainWidth * domainWidth);
  if (mWorldRank == 0)
  {
    std::copy(mMaterialProps.getInitialTemperature().begin(), mMaterialProps.getInitialTemperature().end(), temperatures.begin());
    std::copy(mMaterialProps.getDomainMap().begin(), mMaterialProps.getDomainMap().end(), domainMap.begin());
    std::copy(mMaterialProps.getDomainParameters().begin(), mMaterialProps.getDomainParameters().end(), domainParameters.begin());
  }

  

  /**********************************************************************************************************************/
  /*                                  Call init* and alloc* methods in correct order                                    */
  /**********************************************************************************************************************/
  initGridTopology();
  allocLocalTiles();
  initDataDistribution();
  initHaloExchange();

  if (mSimulationProps.isRunParallelRMA()) {
    MPI_Win_create(localTemperatures.data(), (tileWidth + tileOffsets) * (tileHeight + tileOffsets) * sizeof(float), sizeof(float), MPI_INFO_NULL, MPI_COMM_TOPOLOGY, &tileWindow);
    MPI_Win_create(localTemperatures2.data(), (tileWidth + tileOffsets) * (tileHeight + tileOffsets) * sizeof(float), sizeof(float), MPI_INFO_NULL, MPI_COMM_TOPOLOGY, &nextTileWindow);
  }

  if (!mSimulationProps.getOutputFileName().empty())
  {
    /**********************************************************************************************************************/
    /*                               Open output file if output file name was specified.                                  */
    /*  If mSimulationProps.useParallelIO() flag is set to true, open output file for parallel access, otherwise open it  */
    /*                         only on MASTER rank using sequetial IO. Use openOutputFile* methods.                       */
    /**********************************************************************************************************************/
    if (mSimulationProps.useParallelIO()) {
      openOutputFileParallel();
    } else {
      if (mWorldRank == 0) {
        openOutputFileSequential();
      }
    }
  }
}

ParallelHeatSolver::~ParallelHeatSolver()
{
  /**********************************************************************************************************************/
  /*                                  Call deinit* and dealloc* methods in correct order                                */
  /*                                             (should be in reverse order)                                           */
  /**********************************************************************************************************************/

  if (mSimulationProps.isRunParallelRMA()) {
    MPI_Win_free(&tileWindow);
    MPI_Win_free(&nextTileWindow);
  }

  deinitGridTopology();
  deallocLocalTiles();
  deinitDataDistribution();
  deinitHaloExchange();
}

std::string_view ParallelHeatSolver::getCodeType() const
{
  return codeType;
}

void ParallelHeatSolver::initGridTopology()
{
  /**********************************************************************************************************************/
  /*                          Initialize 2D grid topology using non-periodic MPI Cartesian topology.                    */
  /*                       Also create a communicator for middle column average temperature computation.                */
  /**********************************************************************************************************************/

  std::vector<int> dims{tilesInCol, tilesInRow};
  std::vector<int> periods{false, false};

  MPI_Cart_create(MPI_COMM_WORLD, 2, dims.data(), periods.data(), true, &MPI_COMM_TOPOLOGY);

  MPI_Comm_rank(MPI_COMM_TOPOLOGY, &topologyRank);

  MPI_Cart_coords(MPI_COMM_TOPOLOGY, topologyRank, 2, coords);

  MPI_Cart_shift(MPI_COMM_TOPOLOGY, 1, 1, &topologyLeftRank, &topologyRightRank);
  MPI_Cart_shift(MPI_COMM_TOPOLOGY, 0, 1, &topologyUpRank, &topologyDownRank);

  MPI_Comm_split(MPI_COMM_TOPOLOGY,
                 coords[1],
                 coords[0],
                 &MPI_COMM_MIDDLE_COLUMN);

  MPI_Comm_rank(MPI_COMM_TOPOLOGY, &mTopologyRank);
  MPI_Comm_size(MPI_COMM_TOPOLOGY, &mTopologySize);

  MPI_Comm_rank(MPI_COMM_MIDDLE_COLUMN, &mMiddleColumnRank);
  MPI_Comm_size(MPI_COMM_MIDDLE_COLUMN, &mMiddleColumnSize);
}

void ParallelHeatSolver::deinitGridTopology()
{
  /**********************************************************************************************************************/
  /*      Deinitialize 2D grid topology and the middle column average temperature computation communicator              */
  /**********************************************************************************************************************/

  MPI_Comm_free(&MPI_COMM_MIDDLE_COLUMN);
  MPI_Comm_free(&MPI_COMM_TOPOLOGY);
}

void ParallelHeatSolver::initDataDistribution()
{
  /**********************************************************************************************************************/
  /*                 Initialize variables and MPI datatypes for data distribution (float and int).                      */
  /**********************************************************************************************************************/

  std::vector<int> domainSizeParams{domainWidth, domainWidth};
  std::vector<int> tileSizeParams{tileHeight, tileWidth};
  std::vector<int> startIdxParams{0, 0};

  std::vector<int> tileSizeWithOffsetParams{tileHeight + tileOffsets, tileWidth + tileOffsets};
  std::vector<int> startIdxWithOffsetParams{2, 2};

  // FLOAT

  MPI_Type_create_subarray(2, domainSizeParams.data(), tileSizeParams.data(), startIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &sendTileType);
  MPI_Type_commit(&sendTileType);

  MPI_Type_create_resized(sendTileType, 0, 1 * sizeof(float), &sendTileTypeResized);
  MPI_Type_commit(&sendTileTypeResized);

  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), tileSizeParams.data(), startIdxWithOffsetParams.data(), MPI_ORDER_C, MPI_FLOAT, &recvTileType);
  MPI_Type_commit(&recvTileType);

  // INT

  MPI_Type_create_subarray(2, domainSizeParams.data(), tileSizeParams.data(), startIdxParams.data(), MPI_ORDER_C, MPI_INT, &sendTileTypeInt);
  MPI_Type_commit(&sendTileTypeInt);

  MPI_Type_create_resized(sendTileTypeInt, 0, 1 * sizeof(int), &sendTileTypeResizedInt);
  MPI_Type_commit(&sendTileTypeResizedInt);

  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), tileSizeParams.data(), startIdxWithOffsetParams.data(), MPI_ORDER_C, MPI_INT, &recvTileTypeInt);
  MPI_Type_commit(&recvTileTypeInt);

}

void ParallelHeatSolver::deinitDataDistribution()
{
  /**********************************************************************************************************************/
  /*                       Deinitialize variables and MPI datatypes for data distribution.                              */
  /**********************************************************************************************************************/

  MPI_Type_free(&sendTileType);
  MPI_Type_free(&sendTileTypeResized);
  MPI_Type_free(&recvTileType);
  MPI_Type_free(&sendTileTypeInt);
  MPI_Type_free(&sendTileTypeResizedInt);
  MPI_Type_free(&recvTileTypeInt);
}

void ParallelHeatSolver::allocLocalTiles()
{
  /**********************************************************************************************************************/
  /*            Allocate local tiles for domain map (1x), domain parameters (1x) and domain temperature (2x).           */
  /*                                               Use AlignedAllocator.                                                */
  /**********************************************************************************************************************/

  localTemperatures.resize((tileWidth + tileOffsets) * (tileHeight + tileOffsets));
  localTemperatures2.resize((tileWidth + tileOffsets) * (tileHeight + tileOffsets));
  localDomainMap.resize((tileWidth + tileOffsets) * (tileHeight + tileOffsets));
  localDomainParameters.resize((tileWidth + tileOffsets) * (tileHeight + tileOffsets));


  temperaturesResult.resize(domainWidth * domainWidth); // TODO: TOTO ASI NEKDE JINDE MOVE
  domainMapResult.resize(domainWidth * domainWidth);
}

void ParallelHeatSolver::deallocLocalTiles()
{
  /**********************************************************************************************************************/
  /*                                   Deallocate local tiles (may be empty).                                           */
  /**********************************************************************************************************************/

  localTemperatures.clear();
  localTemperatures2.clear();
  localDomainMap.clear();
  localDomainParameters.clear();
  temperaturesResult.clear();

}

void ParallelHeatSolver::initHaloExchange()
{
  /**********************************************************************************************************************/
  /*                            Initialize variables and MPI datatypes for halo exchange.                               */
  /*                    If mSimulationProps.isRunParallelRMA() flag is set to true, create RMA windows.                 */
  /**********************************************************************************************************************/
  // VERTICALS
  std::vector<int> verticalHaloSizeParams{tileHeight, haloOffset};
  // HORIZONTALS
  std::vector<int> horizontalHaloSizeParams{haloOffset, tileWidth};

  std::vector<int> tileSizeWithOffsetParams{tileHeight + tileOffsets, tileWidth + tileOffsets};


  // RIGHT IN
  std::vector<int> rightHaloInStartIdxParams{haloOffset, tileWidth};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), verticalHaloSizeParams.data(), rightHaloInStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneRightIn);
  MPI_Type_commit(&haloZoneRightIn);

  // LEFT IN
  std::vector<int> leftHaloInStartIdxParams{haloOffset, haloOffset};
  MPI_Type_create_subarray(2 , tileSizeWithOffsetParams.data(), verticalHaloSizeParams.data(), leftHaloInStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneLeftIn);
  MPI_Type_commit(&haloZoneLeftIn);

  // UP IN
  std::vector<int> upperHaloInStartIdxParams{haloOffset, haloOffset};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), horizontalHaloSizeParams.data(), upperHaloInStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneUpIn);
  MPI_Type_commit(&haloZoneUpIn);

  // DOWN IN
  std::vector<int> downHaloInStartIdxParams{tileHeight, haloOffset};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), horizontalHaloSizeParams.data(), downHaloInStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneDownIn);
  MPI_Type_commit(&haloZoneDownIn);

  // RIGHT OUT
  std::vector<int> rightHaloOutStartIdxParams{haloOffset, tileWidth + haloOffset};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), verticalHaloSizeParams.data(), rightHaloOutStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneRightOut);
  MPI_Type_commit(&haloZoneRightOut);

  // LEFT OUT
  std::vector<int> leftHaloOutStartIdxParams{haloOffset, 0};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), verticalHaloSizeParams.data(), leftHaloOutStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneLeftOut);
  MPI_Type_commit(&haloZoneLeftOut);

  // UP OUT
  std::vector<int> upperHaloOutStartIdxParams{0, haloOffset};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), horizontalHaloSizeParams.data(), upperHaloOutStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneUpOut);
  MPI_Type_commit(&haloZoneUpOut);

  // DOWN OUT
  std::vector<int> downHaloOutStartIdxParams{tileHeight + haloOffset, haloOffset};
  MPI_Type_create_subarray(2, tileSizeWithOffsetParams.data(), horizontalHaloSizeParams.data(), downHaloOutStartIdxParams.data(), MPI_ORDER_C, MPI_FLOAT, &haloZoneDownOut);
  MPI_Type_commit(&haloZoneDownOut);
}

void ParallelHeatSolver::deinitHaloExchange()
{
  /**********************************************************************************************************************/
  /*                            Deinitialize variables and MPI datatypes for halo exchange.                             */
  /**********************************************************************************************************************/

  MPI_Type_free(&haloZoneRightIn);
  MPI_Type_free(&haloZoneLeftIn);
  MPI_Type_free(&haloZoneUpIn);
  MPI_Type_free(&haloZoneDownIn);
  MPI_Type_free(&haloZoneRightOut);
  MPI_Type_free(&haloZoneLeftOut);
  MPI_Type_free(&haloZoneUpOut);
  MPI_Type_free(&haloZoneDownOut);

}

template <typename T>
void ParallelHeatSolver::scatterTiles(const T *globalData, T *localData)
{
  static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>, "Unsupported scatter datatype!");

  /**********************************************************************************************************************/
  /*                      Implement master's global tile scatter to each rank's local tile.                             */
  /*     The template T parameter is restricted to int or float type. You can choose the correct MPI datatype like:     */
  /*                                                                                                                    */
  /*  const MPI_Datatype globalTileType = std::is_same_v<T, int> ? globalFloatTileType : globalIntTileType;             */
  /*  const MPI_Datatype localTileType  = std::is_same_v<T, int> ? localIntTileType    : localfloatTileType;            */
  /**********************************************************************************************************************/

  const MPI_Datatype globalTileType = std::is_same_v<T, int> ? sendTileTypeResizedInt : sendTileTypeResized;
  const MPI_Datatype localTileType  = std::is_same_v<T, int> ? recvTileTypeInt    : recvTileType;

  std::vector<int> displacements;
  int tilesCount = tilesInRow * tilesInCol;
  for (int j = 0; j < tilesInCol; ++j)
  {
    for (int i = 0; i < tilesInRow; ++i)
    {
      displacements.push_back(j * tileWidth * tileHeight * tilesInRow + i * tileWidth);
    }
  }

  std::vector<int> sendCounts(tilesCount, 1);

  MPI_Scatterv(globalData, sendCounts.data(), displacements.data(), globalTileType, localData, 1, localTileType, 0, MPI_COMM_TOPOLOGY);
}

template <typename T>
void ParallelHeatSolver::gatherTiles(const T *localData, T *globalData)
{
  static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>, "Unsupported gather datatype!");

  /**********************************************************************************************************************/
  /*                      Implement each rank's local tile gather to master's rank global tile.                         */
  /*     The template T parameter is restricted to int or float type. You can choose the correct MPI datatype like:     */
  /*                                                                                                                    */
  /*  const MPI_Datatype localTileType  = std::is_same_v<T, int> ? localIntTileType    : localfloatTileType;            */
  /*  const MPI_Datatype globalTileType = std::is_same_v<T, int> ? globalFloatTileType : globalIntTileType;             */
  /**********************************************************************************************************************/

  const MPI_Datatype globalTileType = std::is_same_v<T, int> ? sendTileTypeResizedInt : sendTileTypeResized;
  const MPI_Datatype localTileType  = std::is_same_v<T, int> ? recvTileTypeInt    : recvTileType;

  std::vector<int> displacements;
  int tilesCount = tilesInRow * tilesInCol;
  for (int j = 0; j < tilesInCol; ++j)
  {
    for (int i = 0; i < tilesInRow; ++i)
    {
      displacements.push_back(j * tileWidth * tileHeight * tilesInRow + i * tileWidth);
    }
  }

  std::vector<int> sendCounts(tilesCount, 1);

  MPI_Gatherv( localData , 1 , localTileType , globalData , sendCounts.data() , displacements.data() , globalTileType , 0 , MPI_COMM_TOPOLOGY);
}

void ParallelHeatSolver::computeHaloZones(const float *oldTemp, float *newTemp)
{
  /**********************************************************************************************************************/
  /*  Compute new temperatures in halo zones, so that copy operations can be overlapped with inner region computation.  */
  /*                        Use updateTile method to compute new temperatures in halo zones.                            */
  /*                             TAKE CARE NOT TO COMPUTE THE SAME AREAS TWICE                                          */
  /**********************************************************************************************************************/
  
  if (topologyLeftRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileOffset, tileOffset + haloOffset, 2, tileHeight - 2 * haloOffset, tileWidth + tileOffsets);
  }
  if (topologyRightRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileWidth, tileOffset + haloOffset, haloOffset, tileHeight - 2 * haloOffset, tileWidth + tileOffsets);
  }
  if (topologyUpRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileOffset + haloOffset, tileOffset, tileWidth - 2 * haloOffset, haloOffset, tileWidth + tileOffsets);
  }
  if (topologyDownRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileOffset + haloOffset, tileHeight, tileWidth - 2 * haloOffset, haloOffset, tileWidth + tileOffsets);
  }
  if (topologyLeftRank != MPI_PROC_NULL && topologyUpRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileOffset, tileOffset, haloOffset, haloOffset, tileWidth + tileOffsets);
  }
  if (topologyRightRank != MPI_PROC_NULL && topologyUpRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileWidth, tileOffset, haloOffset, haloOffset, tileWidth + tileOffsets);
  }
  if (topologyLeftRank != MPI_PROC_NULL && topologyDownRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileOffset, tileHeight, haloOffset, haloOffset, tileWidth + tileOffsets);
  }
  if (topologyRightRank != MPI_PROC_NULL && topologyDownRank != MPI_PROC_NULL) {
    updateTile(oldTemp, newTemp, localDomainParameters.data(), localDomainMap.data(), tileWidth, tileHeight, haloOffset, haloOffset, tileWidth + tileOffsets);
  }
}

void ParallelHeatSolver::startHaloExchangeP2P(float *localData, std::array<MPI_Request, 8> &requests)
{
  /**********************************************************************************************************************/
  /*                       Start the non-blocking halo zones exchange using P2P communication.                          */
  /*                         Use the requests array to return the requests from the function.                           */
  /*                            Don't forget to set the empty requests to MPI_REQUEST_NULL.                             */
  /**********************************************************************************************************************/
  
  if (topologyLeftRank != MPI_PROC_NULL) {
    MPI_Isend(localData, 1, haloZoneLeftIn, topologyLeftRank, 0, MPI_COMM_TOPOLOGY, &requests[0]);
    MPI_Irecv(localData, 1, haloZoneLeftOut, topologyLeftRank, 0, MPI_COMM_TOPOLOGY, &requests[1]);
  } else {
    requests[0] = MPI_REQUEST_NULL;
    requests[1] = MPI_REQUEST_NULL;
  }
  if (topologyRightRank != MPI_PROC_NULL) {
    MPI_Isend(localData, 1, haloZoneRightIn, topologyRightRank, 0, MPI_COMM_TOPOLOGY, &requests[2]);
    MPI_Irecv(localData, 1, haloZoneRightOut, topologyRightRank, 0, MPI_COMM_TOPOLOGY, &requests[3]);
  } else {
    requests[2] = MPI_REQUEST_NULL;
    requests[3] = MPI_REQUEST_NULL;
  }
  if (topologyUpRank != MPI_PROC_NULL) {
    MPI_Isend(localData, 1, haloZoneUpIn, topologyUpRank, 0, MPI_COMM_TOPOLOGY, &requests[4]);
    MPI_Irecv(localData, 1, haloZoneUpOut, topologyUpRank, 0, MPI_COMM_TOPOLOGY, &requests[5]);
  } else {
    requests[4] = MPI_REQUEST_NULL;
    requests[5] = MPI_REQUEST_NULL;
  }
  if (topologyDownRank != MPI_PROC_NULL) {
    MPI_Isend(localData, 1, haloZoneDownIn, topologyDownRank, 0, MPI_COMM_TOPOLOGY, &requests[6]);
    MPI_Irecv(localData, 1, haloZoneDownOut, topologyDownRank, 0, MPI_COMM_TOPOLOGY, &requests[7]);
  } else {
    requests[6] = MPI_REQUEST_NULL;
    requests[7] = MPI_REQUEST_NULL;
  }
}

void ParallelHeatSolver::startHaloExchangeRMA(float *localData, MPI_Win window)
{
  /**********************************************************************************************************************/
  /*                       Start the non-blocking halo zones exchange using RMA communication.                          */
  /*                   Do not forget that you put/get the values to/from the target's opposite side                     */
  /**********************************************************************************************************************/

  if (topologyLeftRank != MPI_PROC_NULL) {
    MPI_Put(localData, 1, haloZoneLeftIn, topologyLeftRank, 0, 1, haloZoneRightOut, window);
  }
  if (topologyRightRank != MPI_PROC_NULL) {
    MPI_Put(localData, 1, haloZoneRightIn, topologyRightRank, 0, 1, haloZoneLeftOut, window);
  }
  if (topologyUpRank != MPI_PROC_NULL) {
    MPI_Put(localData, 1, haloZoneUpIn, topologyUpRank, 0, 1, haloZoneDownOut, window);
  }
  if (topologyDownRank != MPI_PROC_NULL) {
    MPI_Put(localData, 1, haloZoneDownIn, topologyDownRank, 0, 1, haloZoneUpOut, window);
  }
}

void ParallelHeatSolver::awaitHaloExchangeP2P(std::array<MPI_Request, 8> &requests)
{
  /**********************************************************************************************************************/
  /*                       Wait for all halo zone exchanges to finalize using P2P communication.                        */
  /**********************************************************************************************************************/
  MPI_Waitall(8, requests.data(), MPI_STATUSES_IGNORE);
}

void ParallelHeatSolver::awaitHaloExchangeRMA(MPI_Win window)
{
  /**********************************************************************************************************************/
  /*                       Wait for all halo zone exchanges to finalize using RMA communication.                        */
  /**********************************************************************************************************************/
  MPI_Win_fence(0, window);
}

void ParallelHeatSolver::run(std::vector<float, AlignedAllocator<float>> &outResult)
{
  std::array<MPI_Request, 8> requestsP2P{};

  /**********************************************************************************************************************/
  /*                                         Scatter initial data.                                                      */
  /**********************************************************************************************************************/
  scatterTiles(temperatures.data(), localTemperatures.data());
  scatterTiles(domainMap.data(), localDomainMap.data());
  scatterTiles(domainParameters.data(), localDomainParameters.data());

  /**********************************************************************************************************************/
  /* Exchange halo zones of initial domain temperature and parameters using P2P communication. Wait for them to finish. */
  /**********************************************************************************************************************/
  std::array<MPI_Request, 8> requests;
  startHaloExchangeP2P(localTemperatures.data(), requests);
  awaitHaloExchangeP2P(requests);

  std::array<MPI_Request, 8> requestsDP;
  startHaloExchangeP2P(localDomainParameters.data(), requestsDP);
  awaitHaloExchangeP2P(requestsDP);

  /**********************************************************************************************************************/
  /*                            Copy initial temperature to the second buffer.                                          */
  /**********************************************************************************************************************/
  localTemperatures2 = localTemperatures;

  double startTime = MPI_Wtime();

  // 3. Start main iterative simulation loop.
  for (std::size_t iter = 0; iter < mSimulationProps.getNumIterations(); ++iter)
  {
    const std::size_t oldIdx = iter % 2;       // Index of the buffer with old temperatures
    const std::size_t newIdx = (iter + 1) % 2; // Index of the buffer with new temperatures

    /**********************************************************************************************************************/
    /*                            Compute and exchange halo zones using P2P or RMA.                                       */
    /**********************************************************************************************************************/
    std::array<MPI_Request, 8> requestsP2P;
    computeHaloZones(localTemperatures.data(), localTemperatures2.data());

    if (mSimulationProps.isRunParallelRMA()) {
      
      auto win = (iter % 2) == 0 ? nextTileWindow : tileWindow;

      awaitHaloExchangeRMA(tileWindow);
      awaitHaloExchangeRMA(nextTileWindow);

      startHaloExchangeRMA(localTemperatures2.data(), win);
    } else {
      startHaloExchangeP2P(localTemperatures2.data(), requestsP2P);
    }

    /**********************************************************************************************************************/
    /*                           Compute the rest of the tile. Use updateTile method.                                     */
    /**********************************************************************************************************************/
    updateTile(localTemperatures.data(), localTemperatures2.data(), localDomainParameters.data(), localDomainMap.data(), tileOffset + haloOffset, tileOffset + haloOffset, tileWidth - 2 * haloOffset, tileHeight - 2 * haloOffset, tileWidth + tileOffsets);

    /**********************************************************************************************************************/
    /*                            Wait for all halo zone exchanges to finalize.                                           */
    /**********************************************************************************************************************/
    if (mSimulationProps.isRunParallelRMA()) {
      awaitHaloExchangeRMA(tileWindow);
      awaitHaloExchangeRMA(nextTileWindow);
    } else {
      awaitHaloExchangeP2P(requestsP2P);
    }
    std::swap(localTemperatures, localTemperatures2);


    if (shouldStoreData(iter))
    {
      /**********************************************************************************************************************/
      /*                          Store the data into the output file using parallel or sequential IO.                      */
      /**********************************************************************************************************************/
      if (mSimulationProps.useParallelIO()) {
        storeDataIntoFileParallel(mFileHandle, iter, localTemperatures.data());
      } else {
        gatherTiles(localTemperatures.data(), temperaturesResult.data());
        if (mWorldRank == 0) {
          storeDataIntoFileSequential(mFileHandle, iter, temperaturesResult.data());
        }
      }
    }

    if (shouldPrintProgress(iter) && shouldComputeMiddleColumnAverageTemperature())
    {
      /**********************************************************************************************************************/
      /*                 Compute and print middle column average temperature and print progress report.                     */
      /**********************************************************************************************************************/

      float middleColAvgTemp = computeMiddleColumnAverageTemperatureParallel(localTemperatures.data());
      MPI_Reduce(&middleColAvgTemp, &middleColAvgTempReduceResult, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_MIDDLE_COLUMN);

      if (mMiddleColumnRank == 0) {
        printProgressReport(iter, middleColAvgTempReduceResult / static_cast<float>(mMiddleColumnSize));
      }
    }
  }

  const std::size_t resIdx = mSimulationProps.getNumIterations() % 2; // Index of the buffer with final temperatures

  double elapsedTime = MPI_Wtime() - startTime;

  /**********************************************************************************************************************/
  /*                                     Gather final domain temperature.                                               */
  /**********************************************************************************************************************/
  gatherTiles(localTemperatures.data(), outResult.data());

  /**********************************************************************************************************************/
  /*           Compute (sequentially) and report final middle column temperature average and print final report.        */
  /**********************************************************************************************************************/

  if (mWorldRank == 0)
  {
    float middleColAvgTemp = computeMiddleColumnAverageTemperatureSequential(outResult.data());
    printFinalReport(elapsedTime, middleColAvgTemp);
  }
}

bool ParallelHeatSolver::shouldComputeMiddleColumnAverageTemperature() const
{
  /**********************************************************************************************************************/
  /*                Return true if rank should compute middle column average temperature.                               */
  /**********************************************************************************************************************/
  if (coords[1] == tilesInRow / 2) {
    return true;
  }

  return false;
}

float ParallelHeatSolver::computeMiddleColumnAverageTemperatureParallel(const float *localData) const
{
  /**********************************************************************************************************************/
  /*                  Implement parallel middle column average temperature computation.                                 */
  /*                      Use OpenMP directives to accelerate the local computations.                                   */
  /**********************************************************************************************************************/
  float middleColAvgTemp{};

  for (std::size_t i = tileOffset; i < tileHeight + tileOffset; i++) {
    if (tilesInRow == 1) {
      middleColAvgTemp += localData[i * (tileWidth + tileOffsets) + (tileOffset + tileWidth / 2)];
    } else {
      middleColAvgTemp += localData[i * (tileWidth + tileOffsets) + tileOffset];
    }
  }

  return middleColAvgTemp / static_cast<float>(tileHeight);
}

float ParallelHeatSolver::computeMiddleColumnAverageTemperatureSequential(const float *globalData) const
{
  /**********************************************************************************************************************/
  /*                  Implement sequential middle column average temperature computation.                               */
  /*                      Use OpenMP directives to accelerate the local computations.                                   */
  /**********************************************************************************************************************/
  float middleColAvgTemp{};

  #pragma omp simd reduction(+:middleColAvgTemp) aligned(globalData)
  for(std::size_t i = 0; i < mMaterialProps.getEdgeSize(); i++)
  {
    middleColAvgTemp += globalData[i * mMaterialProps.getEdgeSize() + mMaterialProps.getEdgeSize() / 2];
  }

  return middleColAvgTemp / static_cast<float>(mMaterialProps.getEdgeSize());
}

void ParallelHeatSolver::openOutputFileSequential()
{
  // Create the output file for sequential access.
  mFileHandle = H5Fcreate(mSimulationProps.getOutputFileName(codeType).c_str(),
                          H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (!mFileHandle.valid())
  {
    throw std::ios::failure("Cannot create output file!");
  }
}

void ParallelHeatSolver::storeDataIntoFileSequential(hid_t fileHandle,
                                                     std::size_t iteration,
                                                     const float *globalData)
{
  storeDataIntoFile(fileHandle, iteration, globalData);
}

void ParallelHeatSolver::openOutputFileParallel()
{
#ifdef H5_HAVE_PARALLEL
  Hdf5PropertyListHandle faplHandle{};

  /**********************************************************************************************************************/
  /*                          Open output HDF5 file for parallel access with alignment.                                 */
  /*      Set up faplHandle to use MPI-IO and alignment. The handle will automatically release the resource.            */
  /**********************************************************************************************************************/

  faplHandle = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(faplHandle, MPI_COMM_TOPOLOGY, MPI_INFO_NULL);

  mFileHandle = H5Fcreate(mSimulationProps.getOutputFileName(codeType).c_str(),
                          H5F_ACC_TRUNC,
                          H5P_DEFAULT,
                          faplHandle);
  if (!mFileHandle.valid())
  {
    throw std::ios::failure("Cannot create output file!");
  }
#else
  throw std::runtime_error("Parallel HDF5 support is not available!");
#endif /* H5_HAVE_PARALLEL */


}

void ParallelHeatSolver::storeDataIntoFileParallel(hid_t fileHandle,
                                                   [[maybe_unused]] std::size_t iteration,
                                                   [[maybe_unused]] const float *localData)
{
  if (fileHandle == H5I_INVALID_HID)
  {
    return;
  }

#ifdef H5_HAVE_PARALLEL
  std::array gridSize{static_cast<hsize_t>(mMaterialProps.getEdgeSize()),
                      static_cast<hsize_t>(mMaterialProps.getEdgeSize())};

  // Create new HDF5 group in the output file
  std::string groupName = "Timestep_" + std::to_string(iteration / mSimulationProps.getWriteIntensity());

  Hdf5GroupHandle groupHandle(H5Gcreate(fileHandle, groupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));

  {
    /**********************************************************************************************************************/
    /*                                Compute the tile offsets and sizes.                                                 */
    /*               Note that the X and Y coordinates are swapped (but data not altered).                                */
    /**********************************************************************************************************************/

    hsize_t slabStartIdx[] = {hsize_t(coords[0] * tileHeight), hsize_t(coords[1] * tileWidth)};
    hsize_t slabSize[] = {hsize_t(tileHeight), hsize_t(tileWidth)};

    hsize_t slabMemStartIdx[] = {hsize_t(tileOffset), hsize_t(tileOffset)};
    hsize_t slabMemSize[] = {hsize_t(tileHeight), hsize_t(tileWidth)};

    // Create new dataspace and dataset using it.
    static constexpr std::string_view dataSetName{"Temperature"};

    Hdf5PropertyListHandle datasetPropListHandle{};

    /**********************************************************************************************************************/
    /*                            Create dataset property list to set up chunking.                                        */
    /*                Set up chunking for collective write operation in datasetPropListHandle variable.                   */
    /**********************************************************************************************************************/

    hsize_t chunk[] = {hsize_t(tileHeight), hsize_t(tileWidth)};
    
    datasetPropListHandle = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(datasetPropListHandle, 2, chunk);

    Hdf5DataspaceHandle dataSpaceHandle(H5Screate_simple(2, gridSize.data(), nullptr));
    Hdf5DatasetHandle dataSetHandle(H5Dcreate(groupHandle, dataSetName.data(),
                                              H5T_NATIVE_FLOAT, dataSpaceHandle,
                                              H5P_DEFAULT, datasetPropListHandle,
                                              H5P_DEFAULT));

    Hdf5DataspaceHandle memSpaceHandle{};

    /**********************************************************************************************************************/
    /*                Create memory dataspace representing tile in the memory (set up memSpaceHandle).                    */
    /**********************************************************************************************************************/

    hsize_t memSize[] = {hsize_t(tileHeight + tileOffsets), hsize_t(tileWidth + tileOffsets)};
    memSpaceHandle = H5Screate_simple(2, memSize, nullptr);

    /**********************************************************************************************************************/
    /*              Select inner part of the tile in memory and matching part of the dataset in the file                  */
    /*                           (given by position of the tile in global domain).                                        */
    /**********************************************************************************************************************/

    H5Sselect_hyperslab(dataSpaceHandle, H5S_SELECT_SET, slabStartIdx, nullptr, slabSize, nullptr);
    H5Sselect_hyperslab(memSpaceHandle, H5S_SELECT_SET, slabMemStartIdx, nullptr, slabMemSize, nullptr);

    Hdf5PropertyListHandle propListHandle{};

    /**********************************************************************************************************************/
    /*              Perform collective write operation, writting tiles from all processes at once.                        */
    /*                                   Set up the propListHandle variable.                                              */
    /**********************************************************************************************************************/

    propListHandle = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(propListHandle, H5FD_MPIO_COLLECTIVE);

    H5Dwrite(dataSetHandle, H5T_NATIVE_FLOAT, memSpaceHandle, dataSpaceHandle, propListHandle, localData);
  }

  {
    // 3. Store attribute with current iteration number in the group.
    static constexpr std::string_view attributeName{"Time"};
    Hdf5DataspaceHandle dataSpaceHandle(H5Screate(H5S_SCALAR));
    Hdf5AttributeHandle attributeHandle(H5Acreate2(groupHandle, attributeName.data(),
                                                   H5T_IEEE_F64LE, dataSpaceHandle,
                                                   H5P_DEFAULT, H5P_DEFAULT));
    const double snapshotTime = static_cast<double>(iteration);
    H5Awrite(attributeHandle, H5T_IEEE_F64LE, &snapshotTime);
  }
#else
  throw std::runtime_error("Parallel HDF5 support is not available!");
#endif /* H5_HAVE_PARALLEL */
}
