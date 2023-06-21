--Author: Vojtech Mimochodek
--Login: xmimoc01
--Date: 8.3.2023

module Bruteforce where

import Types
import Helper

bruteforce :: Knapsack -> [Bool]
bruteforce knapsack = createArrayWithUpdatedValues (length (items knapsack)) (knapsackMaxCost knapsack (getValidItems knapsack))

knapsackMaxCost :: Knapsack -> [[Int]] -> [Int]
knapsackMaxCost knapsack x = foldl (\acc y -> if sum (indexesToCosts knapsack y) > sum (indexesToCosts knapsack acc) then y else acc) [] x

getValidItems :: Knapsack -> [[Int]]
getValidItems knapsack = controlWeightsAndCosts knapsack (makeCombinations (itemsIndexesFrom1 knapsack))

controlWeightsAndCosts :: Knapsack -> [[Int]] -> [[Int]]
controlWeightsAndCosts _ [] = []
controlWeightsAndCosts knapsack (x:xs)
    | checkWeightAndCost (maxWeight knapsack) (minCost knapsack) (indexesToWeights knapsack x) (indexesToCosts knapsack x) = x : controlWeightsAndCosts knapsack xs
    | otherwise = controlWeightsAndCosts knapsack xs

checkWeightAndCost :: Int -> Int -> [Int] -> [Int] -> Bool
checkWeightAndCost w c iw ic
    | (w < (sum iw)) || (c > (sum ic)) = False
    | otherwise = True
