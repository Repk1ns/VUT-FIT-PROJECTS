from math import exp
import random
import service
from copy import deepcopy
import matplotlib.pyplot as plt
import numpy as np


def run_algorithm(size, program_repeat, iteration_count, probability, difficulty):
    fitness_history = []

    board = service.generate_board(size, difficulty)
    board = service.prepare_board(board)

    solved_counter = 0

    for i in range(0, program_repeat):
        print("Program run: " + str(i))

        final_board, single_fitness_history = metropolis(deepcopy(board), size, iteration_count, probability)

        if single_fitness_history[len(single_fitness_history) - 1][1] == 0:
            solved_counter += 1
        
        fitness_history.append(single_fitness_history)
        print("Founded solution: ")
        print("Final Fitness: " + str(service.count_conflicts(deepcopy(final_board), size)))
    
    median_history = []
    min_history = []
    max_history = []

    for i in range(iteration_count):
        values_at_step = [run[i][1] for run in fitness_history]

        median_at_step = np.median(values_at_step)
        min_at_step = np.min(values_at_step)
        max_at_step = np.max(values_at_step)

        median_history.append((i, median_at_step))
        min_history.append((i, min_at_step))
        max_history.append((i, max_at_step))

    plt.xlabel("Počet iterací")
    plt.ylabel("Vývoj hodnoty fitness")
    plt.title("Metropolis-Hastings: Sudoku")
    plt.text(6000, 55, "Počet nalezených řešení: " + str(solved_counter) + "/" + str(program_repeat), fontsize=10)
    plt.plot(*zip(*median_history), label="medián")
    plt.plot(*zip(*min_history), label="minimální hodnoty")
    plt.plot(*zip(*max_history), label="maximální hodnoty")

    plt.legend()
    plt.show()

def metropolis(init_board, size, iteration_count, probability):
    fitness_history = []
    run_solved = False

    actual_state = deepcopy(init_board)
    # actual_state = service.fill_with_random(actual_state)
    actual_state = service.fill_board(actual_state, size)
    actual_fitness = service.count_conflicts(deepcopy(actual_state), size)
    print("Beginning fitness: " + str(actual_fitness))

    for i in range(0, iteration_count):
        if run_solved:
            fitness_history.append((i, 0))
            continue
        new_state = deepcopy(actual_state)
        new_state = service.change_two_positions(deepcopy(actual_state))
        new_fitness = service.count_conflicts(deepcopy(new_state), size)

        is_accepted = exp((actual_fitness - new_fitness) / probability) > random.random()

        if is_accepted:
            actual_state = []
            actual_state = deepcopy(new_state)
            actual_fitness = new_fitness
            fitness_history.append((i, new_fitness))
            if actual_fitness == 0:
                run_solved = True
        else:
            fitness_history.append((i, actual_fitness))

    return actual_state, fitness_history