import os
import sys
import subprocess
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from tabulate import tabulate


def execute_make(command):
    # Save the current working directory
    current_directory = os.getcwd()

    # Change to the directory where your Makefile is located
    os.chdir("..")

    # Run the 'make tests' command
    result = subprocess.run(f"make -s {command}", shell=True)

    # Restore the original working directory
    os.chdir(current_directory)


def execute_test(test_name, args):
    # Get the current working directory
    current_directory = os.getcwd()

    # Construct the absolute path to the "bin" directory and change to the "bin" directory
    os.chdir(os.path.join(current_directory, os.pardir, "bin"))

    # Specify the command to run, including the C program and any command-line arguments
    command = f"./{test_name}_test {args}"

    # Use subprocess to run the command and capture the output
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    # Change back to the original directory
    os.chdir(current_directory)

    # Return the output
    return result.stdout.decode("utf-8")


def save_plots(results, test_name, graph_type):
    latent = results["Latent Space"]
    time_ = results["Time"]
    aaf = results["AAF"]

    fig, ax1 = plt.subplots(figsize=(7.5, 5.63))
    time_bars = ax1.barh(np.arange(len(latent)) - 0.35 / 2, time_, 0.35, color="r", label="Time")
    ax1.set_yticks(np.arange(len(latent)))
    ax1.set_yticklabels(latent)
    ax1.set_ylabel("Names")
    ax1.set_xlabel("Time", color="r")
    ax1.tick_params("x", colors="r")

    ax2 = ax1.twiny()
    aaf_bars = ax2.barh(np.arange(len(latent)) + 0.35 / 2, aaf, 0.35, color="b", label="AAF")
    ax2.set_yticks(np.arange(len(latent)))
    ax2.set_yticklabels(latent)
    ax2.set_xlabel("AAF", color="b")
    ax2.tick_params("x", colors="b")

    fig.legend([time_bars, aaf_bars], ["Time", "AAF"], loc="lower right", bbox_to_anchor=(0.9, 0.12))

    plt.title(f"{test_name.upper()} Latent Space Comparison Results")

    if graph_type is None:
        if not os.path.exists(f"{test_name.upper()}"):
            os.makedirs(f"{test_name.upper()}")
        plt.savefig(f"{test_name.upper()}/results.png")
    else:
        if not os.path.exists(f"{graph_type.upper()}"):
            os.makedirs(f"{graph_type.upper()}")
        plt.savefig(f"{graph_type.upper()}/results.png")

    print("The bar plots were saved")


def save_csv(results, test_name, graph_type):
    # Convert the dictionary to a DataFrame
    df = pd.DataFrame(results)

    # Save the DataFrame to a CSV file
    if graph_type is None:
        if not os.path.exists(f"{test_name.upper()}"):
            os.makedirs(f"{test_name.upper()}")
        df.to_csv(f"{test_name.upper()}/{test_name}.csv", index=False)
    else:
        if not os.path.exists(f"{graph_type.upper()}"):
            os.makedirs(f"{graph_type.upper()}")
        df.to_csv(f"{graph_type.upper()}/{test_name}_{graph_type}.csv", index=False)

    print("CSV file was saved")


if __name__ == "__main__":
    execute_make(f"lsh-test")

    results = {
        "Latent Space": ["Original", 10, 15, 20, 25],
        "Time": [10, 15, 20, 25, 30],
        "AAF": [1, 2, 3, 4, 5],
    }

    # output = execute_test(f"lsh", f"1 2 3")
    # output = output.split("\n")
    # print(output)

    save_plots(results, "lsh", None)

    save_csv(results, "lsh", None)

    execute_make("clean")
