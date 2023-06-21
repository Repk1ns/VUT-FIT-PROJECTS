--Author: Vojtech Mimochodek
--Login: xmimoc01
--Date: 6.3.2023
--Description: Main module - parsing arguments, call actions

import System.IO
    ( hClose, IOMode(ReadMode), openFile, hGetContents, stdin )
import Control.Monad ()
import System.Directory.Internal.Prelude (getArgs)
import Types
import Bruteforce
import Text.ParserCombinators.Parsec
import Genetic
import System.Random

main :: IO ()
main = do
    args <- getArgs
    let (arg, file) = processArguments args
    handle <- if isFileExist file
           then openFile file ReadMode
           else return stdin
    contents <- hGetContents handle
    case parseInput contents of
      Left err -> print err
      Right knapsack -> resolve arg knapsack

    hClose handle


--printItems :: Knapsack -> IO()
--printItems knapsack = mapM_ print (items knapsack)

--Function that choose action depends on input arguments
resolve :: String -> Knapsack -> IO()
resolve x y
  | x == "-i" = print y
  | x == "-b" = print (bruteforce y)
  | x == "-o" = do
    seed <- randomIO :: IO Int
    let res = evaluatePopulation y ([tournament (evaluatePopulation y (genetic y 4000 (seed) (createRandomPopulation y 100 seed)))])
    print res
  | otherwise = error "Undefined action for input argument"

--Function that valid input arguments
processArguments :: [String] -> (String, String)
processArguments [] = error "Too less input arguments"
processArguments (_:xs)
  | length xs >= 2 = error "Too much input arguments"
processArguments [x, y]
  | x `notElem` ["-i", "-b", "-o"] = error "Wrong first argument"
  | otherwise = (x, y)
processArguments [x]
  | x `notElem` ["-i", "-b", "-o"] = error "Wrong first argument"
  | otherwise = (x, [])
processArguments _ = error "Another argument error" 

isFileExist :: String -> Bool
isFileExist [] = False
isFileExist _ = True

knapsackParser :: Parser Knapsack
knapsackParser = do
    _ <- string "Knapsack {"
    spaces
    maxWeight' <- string "maxWeight:" *> spaces *> many1 digit <* spaces
    minCost' <- string "minCost:" *> spaces *> many1 digit <* spaces
    items' <- itemsParser
    _ <- char '}'
    return $ Knapsack (read maxWeight') (read minCost') items'

itemsParser :: Parser [Item]
itemsParser = do
    _ <- string "items:"
    spaces
    _ <- char '['
    spaces
    items' <- many1 itemParser
    spaces
    _ <- char ']'
    spaces
    return items'

itemParser :: Parser Item
itemParser = do
    spaces
    _ <- string "Item {"
    spaces
    weight' <- string "weight:" *> spaces *> many1 digit <* spaces
    cost' <- string "cost:" *> spaces *> many1 digit <* spaces
    _ <- char '}'
    spaces
    return $ Item (read weight') (read cost')

parseInput :: String -> Either ParseError Knapsack
parseInput input = parse knapsackParser "(unknown)" input
