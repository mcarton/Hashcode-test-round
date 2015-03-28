import System.Environment
import Control.Monad
import Data.List
import Data.Ord

import Debug.Trace

type Part = (Int, Int, Int, Int)

main :: IO ()
main = do
  (inputFile:_) <- getArgs
  (nbr:pizza) <- liftM lines $ readFile inputFile
  let [nb_rows, nb_cols, jambon, tailleMax] = map (read :: String -> Int) . words $ nbr
      parts = augment pizza 0 [[] | _ <- [0..11]]
  putStr $ printer parts
  traceShow (size parts) $ return ()

isValid :: Int -> [String] -> Part -> Bool
isValid t pizza (r1, c1, r2, c2) = nbH >= t
  where nbH = sum [if getStat pizza (r, c) == 'H' then 1 else 0 | r <- [r1..r2], c <- [c1..c2]]

getStat :: [String] -> (Int, Int) -> Char
getStat pizza (r, c) = (pizza !! r) !! c

printer :: [Part] -> String
printer parts = show (length parts)++"\n"++concat [show r1++" "++show c1++" "++show r2++" "++show c2++"\n" | (r1, c1, r2, c2) <- parts]

augment :: [String] -> Int -> [[Part]] -> [Part]
augment pizza ligne score
  | ligne >= length pizza = head score
  | otherwise = augment pizza (ligne+1) $ ((score !! i')++sol0):init score
    where pour0 = [(n, consomme pizza (ligne-n) (n+1) 0 [[] | _ <- [0..12`div`(n+1)-1]]) | n <- [0..11]]
          (i', sol0) = maximumBy (comparing $ \(i, parts) -> size (score !! i)+size parts) pour0

consomme :: [String] -> Int -> Int -> Int -> [[Part]] -> [Part]
consomme pizza r hauteur c score
  | r < 0 = []
  | c >= length (head pizza) = head score
  | otherwise = consomme pizza r hauteur (c+1) $ ((score !! i')++sol0):init score
      where pour0 = [(n, getPart pizza r (c-n) (r+hauteur-1) c) | n <- [0..length score-1]]
            (i', sol0) = maximumBy (comparing $ \(i, parts) -> size (score !! i)+size parts) pour0

getPart :: [String] -> Int -> Int -> Int -> Int -> [Part]
getPart pizza r1 c1 r2 c2
  | c1 < 0 = []
  | isValid 3 pizza part = [part]
  | otherwise = []
  where part = (r1, c1, r2, c2)

size :: [Part] -> Int
size = sum . map (\(r1, c1, r2, c2) -> (r2-r1+1)*(c2-c1+1))
