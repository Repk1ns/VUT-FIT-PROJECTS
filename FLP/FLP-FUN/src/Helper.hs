--Author: Vojtech Mimochodek
--Login: xmimoc01
--Date: 8.3.2023

module Helper where

import Types
import Data.List


createArrayWithUpdatedValues :: Int -> [Int] -> [Bool]
createArrayWithUpdatedValues x y = foldl (\acc idx -> take (idx-1) acc ++ [True] ++ drop idx acc) (replicate x False) y

convertToIndices :: [Int] -> [Int]
convertToIndices xs = map (+1) $ elemIndices 1 xs

makeCombinations :: [Int] -> [[Int]]
makeCombinations [] = [[]]
makeCombinations (x:xs) = makeCombinations xs ++ map (x:) (makeCombinations xs)

countItems :: Knapsack -> Int
countItems knapsack = length (items knapsack)

itemsIndexesFrom1 :: Knapsack -> [Int]
itemsIndexesFrom1 knapsack  = [1..(countItems knapsack)]

indexesToCosts :: Knapsack -> [Int] -> [Int]
indexesToCosts knapsack x = map (\i -> cost (items knapsack !! (i-1))) x

indexesToWeights :: Knapsack -> [Int] -> [Int]
indexesToWeights knapsack x = map (\i -> weight (items knapsack !! (i-1))) x

isUniqueItems :: [Int] -> Bool
isUniqueItems [] = True
isUniqueItems (x:xs) = x `notElem` xs && isUniqueItems xs