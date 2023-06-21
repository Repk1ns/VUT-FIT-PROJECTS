--Author: Vojtech Mimochodek
--Login: xmimoc01
--Date: 28.2.2023
module Types where

data Item = Item {
    weight :: Int,
    cost :: Int
} deriving (Show)

data Knapsack = Knapsack {
    maxWeight :: Int,
    minCost :: Int,
    items :: [Item]
} deriving (Show)

item1 = Item { weight = 20, cost = 13 }
item2 = Item { weight = 5, cost = 10 }
item3 = Item { weight = 15, cost = 20 }
item4 = Item { weight = 20, cost = 15 }
item5 = Item { weight = 18, cost = 24 }
item6 = Item { weight = 21, cost = 9 }

item7 = Item { weight = 35, cost = 750 }
item8 = Item { weight = 76, cost = 7 }
item9 = Item { weight = 95, cost = 78 }
item10 = Item { weight = 76, cost = 1 }
item11 = Item { weight = 22, cost = 52 }
item12 = Item { weight = 72, cost = 68 }
item13 = Item { weight = 62, cost = 83 }
item14 = Item { weight = 43, cost = 39 }
item15 = Item { weight = 52, cost = 41 }
item16 = Item { weight = 4, cost = 15 }
item17 = Item { weight = 34, cost = 19 }
item18 = Item { weight = 18, cost = 21 }
item19 = Item { weight = 69, cost = 62 }
item20 = Item { weight = 20, cost = 82 }
item21 = Item { weight = 17, cost = 100 }
item22 = Item { weight = 10, cost = 52 }
item23 = Item { weight = 8, cost = 3 }
item24 = Item { weight = 90, cost = 39 }
item25 = Item { weight = 81, cost = 2 }
item26 = Item { weight = 15, cost = 56 }
item27 = Item { weight = 81, cost = 11 }
item28 = Item { weight = 51, cost = 88 }
item29 = Item { weight = 97, cost = 73 }

item17_1 = Item { weight = 12, cost = 16} 
item17_2 = Item { weight = 3,  cost = 30 }
item17_3 = Item { weight = 86, cost = 63 }
item17_4 = Item { weight = 55, cost = 71 }
item17_5 = Item { weight = 53, cost = 28 }
item17_6 = Item { weight = 95, cost = 10 }

batoh = Knapsack { maxWeight = 70, minCost = 10, items = [item1, item2, item3, item4, item5, item6] }
batoh49 = Knapsack { maxWeight = 102, minCost = 380, items = [item7, item8, item9, item10, item11, item12, item13, item14, item15, item16, item17, item18, item19, item20, item21, item22, item23, item24, item25, item26, item27, item28, item29]}
batoh17 = Knapsack { maxWeight = 135, minCost = 145, items = [item17_1, item17_2, item17_3, item17_4, item17_5, item17_6] }
