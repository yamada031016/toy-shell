module Main (main) where

import Lib
import System.Environment
import System.Console.Readline

main :: IO ()
main = do
    -- name <- getEnv "USER"
    -- putStrLn (name ++ "> ")
    line <- readline "% "
    case line of
        Nothing     ->  return ()
        Just "exit" ->  return ()
        Just line   ->  do  addHistory line
                            putStrLn $ "input: " ++ (show line)
                            main
