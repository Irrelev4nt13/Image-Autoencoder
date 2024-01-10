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


def save_plots(results, test_name, graph_type, cluster_type, bruteforce_type):
    # Extract names
    dict_keys_list = list(results.keys())
    zero = dict_keys_list[0]
    first = dict_keys_list[1]
    second = dict_keys_list[2]
    third = dict_keys_list[3]
    fourth = dict_keys_list[4]

    color_map = {"Original": "blue", "10": "red", "20": "green", "30": "purple"}

    plt.figure()
    plt.bar(results[zero], results[first], label=first, color=[color_map[name] for name in ["Original", "10", "20", "30"]])
    plt.xlabel(f"{zero} values")
    plt.ylabel(f"{first} values")
    plt.title(f"f({zero}) = {first}")
    if graph_type is None:
        plt.title(f"{test_name.upper()} Latent Space Comparison Results {first}")
        if not os.path.exists(f"{test_name.upper()}"):
            os.makedirs(f"{test_name.upper()}")
        plt.savefig(f"{test_name.upper()}/{first}.png")
    else:
        plt.title(f"{graph_type.upper()} Latent Space Comparison Results {first}")
        if not os.path.exists(f"{graph_type.upper()}"):
            os.makedirs(f"{graph_type.upper()}")
        plt.savefig(f"{graph_type.upper()}/{first}.png")

    plt.figure()
    plt.bar(results[zero], results[second], label=second, color=[color_map[name] for name in ["Original", "10", "20", "30"]])
    plt.xlabel(f"{zero} values")
    plt.ylabel(f"{second} values")
    plt.title(f"f({zero}) = {second}")
    if graph_type is None:
        plt.title(f"{test_name.upper()} Latent Space Comparison Results {second}")
        if not os.path.exists(f"{test_name.upper()}"):
            os.makedirs(f"{test_name.upper()}")
        plt.savefig(f"{test_name.upper()}/{second}.png")
    else:
        plt.title(f"{graph_type.upper()} Latent Space Comparison Results {second}")
        if not os.path.exists(f"{graph_type.upper()}"):
            os.makedirs(f"{graph_type.upper()}")
        plt.savefig(f"{graph_type.upper()}/{second}.png")

    if bruteforce_type is None:
        plt.figure()
        plt.bar(results[zero], results[third], label=third, color=[color_map[name] for name in ["Original", "10", "20", "30"]])
        plt.xlabel(f"{zero} values")
        plt.ylabel(f"{third} values")
        plt.title(f"f({zero}) = {third}")
        if graph_type is None:
            plt.title(f"{test_name.upper()} Latent Space Comparison Results {third}")
            if not os.path.exists(f"{test_name.upper()}"):
                os.makedirs(f"{test_name.upper()}")
            plt.savefig(f"{test_name.upper()}/{third}.png")
        else:
            plt.title(f"{graph_type.upper()} Latent Space Comparison Results {third}")
            if not os.path.exists(f"{graph_type.upper()}"):
                os.makedirs(f"{graph_type.upper()}")
            plt.savefig(f"{graph_type.upper()}/{third}.png")

    if cluster_type is None:
        plt.figure()
        plt.bar(results[zero], results[fourth], label=fourth, color=[color_map[name] for name in ["Original", "10", "20", "30"]])
        plt.xlabel(f"{zero} values")
        plt.ylabel(f"{fourth} values")
        plt.title(f"f({zero}) = {fourth}")
        if graph_type is None:
            plt.title(f"{test_name.upper()} Latent Space Comparison Results {fourth}")
            if not os.path.exists(f"{test_name.upper()}"):
                os.makedirs(f"{test_name.upper()}")
            plt.savefig(f"{test_name.upper()}/{fourth}.png")
        else:
            plt.title(f"{graph_type.upper()} Latent Space Comparison Results {fourth}")
            if not os.path.exists(f"{graph_type.upper()}"):
                os.makedirs(f"{graph_type.upper()}")
            plt.savefig(f"{graph_type.upper()}/{fourth}.png")

    print("Bar plots were saved")


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
    # execute_make(f"lsh-test")
    results = {
        "Latent Space": ["Original", "10", "20", "30"],
        "tAverageApproximate": [10, 15, 20, 25],
        "tAverageTrue": [100, 90, 80, 70],
        "AAF": [1, 2, 3, 4],
        "MAF": [1, 2, 3, 4],
    }
    test_name = "lsh"
    graph_type = None
    cluster_type = None
    bruteforce_type = None
    # for i, (test_name, graph_type) in enumerate(zip(["lsh", "cube", "graph", "graph"], [None, None, "gnns", "mrng"])):
    #     # results = {
    #     #     "Latent Space": ["Original", 10, 15, 20, 25],
    #     #     "Time": [10, 15, 20, 25, 30],
    #     #     "AAF": [1, 2, 3, 4, 5],
    #     # }
    #     for latent_space in ["Original", 10, 20, 30]:
    #         results = {
    #             "Latent Space": [],
    #             "tAverageApproximate": [],
    #             "tAverageTrue": [],
    #             "AAF": [],
    #             "MAF": [],
    #         }
    #         # print(test_name)
    #         # print(graph_type)
    #         output = execute_test(f"{test_name}", f"{i + 1} {i + 2}")
    #         output = output.split("\n")
    #         for curr_output in output:
    #             key, value = curr_output.split(":")
    #             results[key].append(float(value))
    #         results["Latent Space"].append(latent_space)
    #         # print(output)

    save_plots(results, test_name, graph_type, cluster_type, bruteforce_type)

    # save_csv(results, test_name, graph_type)

    execute_make("clean")
