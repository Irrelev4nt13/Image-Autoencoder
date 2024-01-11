import os
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


def save_plots(df, name, cluster):
    first_column = df.columns[0]
    sns.set_theme(style="whitegrid", rc={"axes.grid": False})
    # Iterate through the remaining numeric columns and create bar plots
    for column in df.columns[1:]:
        if pd.api.types.is_numeric_dtype(df[column]):
            plt.figure()
            sns.barplot(x=first_column, y=column, data=df)
            plt.title(f"{column} in {name}")
            plt.xlabel(first_column)
            plt.ylabel(f"{column} values")
            if cluster is None:
                if not os.path.exists(f"NEAREST NEIGHBORS/{name.upper()}"):
                    os.makedirs(f"NEAREST NEIGHBORS/{name.upper()}")
                plt.savefig(f"NEAREST NEIGHBORS/{name.upper()}/{column}.png")
            else:
                if not os.path.exists(f"{cluster.upper()}"):
                    os.makedirs(f"{cluster.upper()}")
                plt.savefig(f"{cluster.upper()}/{column}.png")


if __name__ == "__main__":
    Original_Dimension = {
        "Algorithms": ["BRUTEFORCE", "LSH", "CUBE", "GNNS", "MRNG"],
        "Time Taken": [0.036007000, 0.004449070, 0.000615765, 0.000496924, 0.000326701],
        "AAF": [1.00000, 1.04879, 1.20910, 1.19411, 1.00002],
        "MAF": [1.00000, 1.54656, 2.40257, 3.44551, 1.04169],
    }

    Latent_Space_10 = {
        "Algorithms": ["BRUTEFORCE", "GNNS LSH", "GNNS BRUTEFORCE", "MRNG"],
        "Time Taken": [0.001343520, 0.000171030, 0.000133263, 0.000192239],
        "AAF": [1.12587, 1.23841, 1.21754, 1.13878],
        "MAF": [1.85081, 2.79808, 2.53537, 1.69740],
    }

    Latent_Space_20 = {
        "Algorithms": ["BRUTEFORCE", "GNNS LSH", "GNNS BRUTEFORCE", "MRNG"],
        "Time Taken": [0.001746020, 0.000142500, 0.000138986, 0.000179752],
        "AAF": [1.08886, 1.29320, 1.32244, 1.09369],
        "MAF": [2.01859, 4.23699, 4.40786, 1.88183],
    }

    Latent_Space_30 = {
        "Algorithms": ["BRUTEFORCE", "GNNS LSH", "GNNS BRUTEFORCE", "MRNG"],
        "Time Taken": [0.002126670, 0.000145430, 0.000145230, 0.000166726],
        "AAF": [1.04199, 1.29336, 1.25933, 1.04451],
        "MAF": [1.61952, 4.62074, 2.85942, 1.61952],
    }

    Clustering = {
        "Latent Space": ["Original", "10", "20", "30"],
        "Clustering Time": [0.811292, 0.205660, 0.446624, 0.892129],
        "Average Silhouette": [0.0745650, 0.0610482, 0.0549118, 0.0677806],
        "Objective Function": [2.91229e10, 5.15093e10, 4.97412e10, 4.73113e10],
    }

    for dict_name, name, cluster in zip(
        [Original_Dimension, Latent_Space_10, Latent_Space_20, Latent_Space_30, Clustering],
        ["Original Dimension", "Latent Space 10", "Latent Space 20", "Latent Space 30", "Clustering"],
        [None, None, None, None, "Clustering"],
    ):
        df = pd.DataFrame(dict_name)
        save_plots(df, name, cluster)
