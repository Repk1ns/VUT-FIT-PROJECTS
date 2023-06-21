--Author: Vojtech Mimochodek
--Login: xmimoc01
--Date: 13.3.2023

module Genetic where

import Types
import Helper
import System.Random
import Data.List
import Data.Ord

genetic :: Knapsack -> Int -> Int -> [[Int]] -> [[Int]]
genetic _ 0 _ result = result
genetic knapsack iteration seed actualPopulation = genetic knapsack (iteration - 1) (seed + 2) (createNewPopulation knapsack (mkStdGen seed) actualPopulation [])

createNewPopulation :: Knapsack -> StdGen -> [[Int]] -> [[Int]] -> [[Int]]
createNewPopulation knapsack gen oldPop newPop
  | (length newPop) == 100 = newPop
  | otherwise = createNewPopulation knapsack nextGen oldPop (newPop ++ (mutation randomSeed (crossover(getParents (evaluatePopulation knapsack oldPop) randomSeed)) [] ))
    where
      (randomSeed, nextGen) = randomR (0, 90484096) gen

mutation :: Int -> [[Int]] -> [[Int]] -> [[Int]]
mutation _ [] mutatedPopulation = mutatedPopulation
mutation seed (x:xs) mutatedPopulation = mutation (seed+1) xs (mutatedPopulation ++ [(mutate seed x)])

mutate :: Int -> [Int] -> [Int]
mutate seed element = mutatedElement
  where
    (idx, _) = randomR (0, length element - 1) (mkStdGen seed)
    mutatedElement = take idx element ++ [1 - element !! idx] ++ drop (idx + 1) element

crossover :: [[Int]] -> [[Int]]
crossover [] = [[]]
crossover [[]] = [[]]
crossover (x:y:_) = [fst sx ++ snd sy, fst sy ++ snd sx]
  where
    sx = splitAt (length x `div` 2) x
    sy = splitAt (length y `div` 2) y

getParents :: [(Int, [Int])] -> Int -> [[Int]]
getParents population seed = [parent1, parent2]
  where
    parent1 = tournament (randomSix (mkStdGen seed) population)
    parent2 = tournament (randomSix (mkStdGen (seed + 992)) population)

tournament :: [(Int, [Int])] -> [Int]
tournament candidates = snd $ maximumBy (comparing fst) candidates

-- TODO : nevybere 2 stejne
randomSix :: RandomGen g => g -> [(Int, [Int])] -> [(Int, [Int])]
randomSix gen xs
  | isUniqueItems [idx1, idx2, idx3, idx4, idx5, idx6] = [xs !! idx1, xs !! idx2, xs !! idx3, xs !! idx4, xs !! idx5, xs !! idx6]
  | otherwise = randomSix gen6 xs
  where
    (idx1, gen1) = randomR (0, length xs - 1) gen
    (idx2, gen2) = randomR (0, length xs - 1) gen1
    (idx3, gen3) = randomR (0, length xs - 1) gen2
    (idx4, gen4) = randomR (0, length xs - 1) gen3
    (idx5, gen5) = randomR (0, length xs - 1) gen4
    (idx6, gen6) = randomR (0, length xs - 1) gen5

evaluatePopulation :: Knapsack -> [[Int]] -> [(Int, [Int])]
evaluatePopulation knapsack it = map (evaluateOneElem knapsack) it

evaluateOneElem :: Knapsack -> [Int] -> (Int, [Int])
evaluateOneElem knapsack population = (fitness, population)
  where fitness = getFitness knapsack population

createRandomPopulation :: Knapsack -> Int -> Int -> [[Int]]
createRandomPopulation knapsack populationCount seed = randomLists (mkStdGen seed) populationCount (countItems knapsack) 

randomLists :: StdGen -> Int -> Int -> [[Int]]
randomLists gen n len
  | n <= 0 = []
  | otherwise =
    let (list, nextGen) = randomList len gen
    in list : randomLists nextGen (n-1) len

randomList :: Int -> StdGen -> ([Int], StdGen)
randomList n gen = (take n $ randomRs (0,1) gen, snd $ split gen)

getFitness :: Knapsack -> [Int] -> Int
getFitness knapsack it
  | (countedWeight > (maxWeight knapsack)) || (countedCost < (minCost knapsack)) = 0
  | otherwise = countedCost
  where
    countedWeight = sum (indexesToWeights knapsack (convertToIndices it))
    countedCost = sum (indexesToCosts knapsack (convertToIndices it))