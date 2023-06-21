import argparse
import metropolis


parser = argparse.ArgumentParser(description='VUT FIT EVO: Project: Metropolis-Hastings - Sudoku')

parser.add_argument('-s', '--size', type=int,
                    help='Insert integer number for size of sub-grid (e.g 3 for 3x3)')
parser.add_argument('-p', '--probability', type=float,
                    help='The probability of accepting a worse state')
parser.add_argument('-i', '--iteration', type=int,
                    help='Iteration count')
parser.add_argument('-r', '--runs', type=int,
                    help='Number of program runs')
parser.add_argument('-d', '--difficulty', type=float,
                    help='Difficulty of Sudoku')

args = parser.parse_args()

metropolis.run_algorithm(args.size, args.runs, args.iteration, args.probability, args.difficulty)