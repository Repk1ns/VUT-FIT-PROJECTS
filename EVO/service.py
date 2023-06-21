import random
from sudoku import Sudoku


def generate_board(size, diffic):
    puzzle = Sudoku(size).difficulty(diffic)

    return puzzle.board


def prepare_board(board):
    for i in range(len(board)):
        for j in range(len(board[0])):
            if board[i][j] == None:
                board[i][j] = (0, False)
            else:
                board[i][j] = (board[i][j], True)
    return board


def fill_with_random(board):
    for i in range(len(board)):
        for j in range(len(board[0])):
            if board[i][j][1] == False:
                board[i][j] = (random.randint(1, len(board[0])), False)
    return board


def fill_board(board, size):
    for i in range(len(board)):
        line = list(range(size**2 + 1))

        for j in range(len(board[0])):
            if board[i][j][1] == True:
                line.remove(board[i][j][0])
        
        for k in range(len(board[0])):
            if board[i][k][1] == False:
                line_length = len(line) - 1
                random_number = random.randint(0, line_length)
                board[i][k] = (line[random_number], False)
                line.remove(line[random_number])
    
    return board


def change_two_positions(board):
    unfixed_positions = []

    for i in range(len(board)):
        for j in range(len(board[0])):
            if board[i][j][1] == False:
                unfixed_positions.append((i, j))

    unfixed_positions_count = len(unfixed_positions) - 1
    first_to_change = random.randint(0, unfixed_positions_count)
    second_to_change = random.randint(0, unfixed_positions_count)

    val1 = board[unfixed_positions[first_to_change][0]][unfixed_positions[first_to_change][1]][0]
    val2 = board[unfixed_positions[second_to_change][0]][unfixed_positions[second_to_change][1]][0]

    board[unfixed_positions[first_to_change][0]][unfixed_positions[first_to_change][1]] = (val2, False)
    board[unfixed_positions[second_to_change][0]][unfixed_positions[second_to_change][1]] = (val1, False)
    
    return board


def change_two_positions_in_row(board):
    unfixed_positions = []

    for i in range(len(board)):
        for j in range(len(board[0])):
            if board[i][j][1] == False:
                unfixed_positions.append((i, j))

        unfixed_positions_count = len(unfixed_positions) - 1
        first_to_change = random.randint(0, unfixed_positions_count)
        second_to_change = random.randint(0, unfixed_positions_count)

        val1 = board[unfixed_positions[first_to_change][0]][unfixed_positions[first_to_change][1]][0]
        val2 = board[unfixed_positions[second_to_change][0]][unfixed_positions[second_to_change][1]][0]

        board[unfixed_positions[first_to_change][0]][unfixed_positions[first_to_change][1]] = (val2, False)
        board[unfixed_positions[second_to_change][0]][unfixed_positions[second_to_change][1]] = (val1, False)

        unfixed_positions = []
    
    return board

def count_conflicts(board, size):
    for i in range(len(board)):
        for j in range(len(board[0])):
            board[i][j] = board[i][j][0]
            
    conflicts = 0
    for row in range(len(board)):
        for col in range(len(board[0])):
            value = board[row][col]
            if value != (0, False):
                if board[row].count(value) > 1:
                    conflicts += 1
                
                if [board[i][col] for i in range(len(board[0]))].count(value) > 1:
                    conflicts += 1
                
                block_row = (row // size) * size
                block_col = (col // size) * size
                block = [board[block_row+i][block_col+j] for i in range(size) for j in range(size)]
                if block.count(value) > 1:
                    conflicts += 1
    return conflicts


def print_board(board, size):
    print("\n=======================SUDOKU BOARD=======================\n")
    for i in range(len(board)):
        if i % size == 0 and i != 0:
            print("----------------------------------------------------------")
        for j in range(len(board[0])):
            if j % size == 0 and j != 0:
                print(" |  ", end="")
            if j == size ** 2 - 1:
                print(board[i][j][0])
            else:
                print(str(board[i][j][0]) + " ", end="")
    print("\n==========================================================\n")