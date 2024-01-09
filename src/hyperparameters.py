import sys
import os
import numpy as np
import pandas as pd
import optuna
import matplotlib.pyplot as plt

from Autoencoder import Encoder, Decoder, Autoencoder

import tensorflow as tf
from tensorflow.keras import Model
from tensorflow.keras import backend as K
from tensorflow.keras.utils import set_random_seed
from tensorflow.keras.datasets import mnist
from tensorflow.keras.models import save_model, load_model
from keras.metrics import Precision, Recall
from tensorflow.keras.layers import Input


def load_mnist():
    # Load MNIST dataset
    (x_train, _), (x_test, _) = tf.keras.datasets.mnist.load_data()
    x_train = x_train.astype("float32") / 255.0
    x_test = x_test.astype("float32") / 255.0
    x_train = np.reshape(x_train, (len(x_train), 28, 28, 1))
    x_test = np.reshape(x_test, (len(x_test), 28, 28, 1))
    return x_train, x_test


def f1_score(precision, recall):
    return 2 * (precision * recall) / (precision + recall)


def objective(trial):
    X_train, X_val = load_mnist()
    # Hyperparameter search space
    num_conv_layers = trial.suggest_int("num_conv_layers", 1, 3)
    conv_filters = [trial.suggest_categorical(f"conv_filter_{i}", [8, 16, 32, 64, 128]) for i in range(num_conv_layers)]
    conv_kernels = [trial.suggest_categorical(f"conv_kernel_{i}", [1, 2, 3]) for i in range(num_conv_layers)]
    pool_sizes = [trial.suggest_categorical(f"pool_size_{i}", [1, 2, 3, 4]) for i in range(num_conv_layers)]
    epochs = trial.suggest_int("epochs", 1, 2)
    batch_size = trial.suggest_categorical("batch_size", [16, 32, 64, 128, 256])
    optimizer = trial.suggest_categorical("optimizer", ["adam", "rmsprop", "sgd", "adagrad", "adadelta", "adamax", "nadam"])
    latent_space = trial.suggest_categorical("latent_space", [10, 20, 30])

    K.clear_session()
    set_random_seed(279)

    # Build the dynamic autoencoder model
    autoencoder = Autoencoder((28, 28, 1), num_conv_layers, conv_filters, conv_kernels, pool_sizes, latent_space)

    try:
        # Train the autoencoder
        autoencoder.compile(optimizer=optimizer, loss="mse")  # Adjust loss function as needed
        autoencoder.fit(X_train, X_train, epochs=epochs, batch_size=batch_size, shuffle=True)
        return autoencoder.evaluate(X_val, X_val)

    except:
        return float("inf")  # Return a large value to indicate failure


if __name__ == "__main__":
    study = optuna.create_study(direction="minimize", study_name="Autoencoder Tuning")
    study.optimize(objective, n_trials=10)

    # Get optuna plots
    fig = optuna.visualization.plot_optimization_history(study)
    fig.show()
    fig = optuna.visualization.plot_param_importances(study)
    fig.show()

    # Convert the trials to a DataFrame
    trials_df = study.trials_dataframe()

    # Save the DataFrame to a CSV file
    if not os.path.exists("AUTOENCODER/optuna"):
        os.makedirs("AUTOENCODER/optuna")
    trials_df.to_csv(f"AUTOENCODER/optuna/optuna_trials.csv", index=False)

    idx_min_values = trials_df.groupby("params_latent_space")["value"].idxmin()
    best_trials_df = trials_df.loc[idx_min_values].reset_index(drop=True)

    for index, row in best_trials_df.iterrows():
        print(row)
        curr_num_conv_layer = int(row["params_num_conv_layers"])
        curr_conv_filters = [int(row[f"params_conv_filter_{i}"]) for i in range(row["params_num_conv_layers"])]
        curr_conv_kernels = [int(row[f"params_conv_kernel_{i}"]) for i in range(row["params_num_conv_layers"])]
        curr_pool_sizes = [int(row[f"params_pool_size_{i}"]) for i in range(row["params_num_conv_layers"])]
        curr_epochs = int(row["params_epochs"])
        curr_batch_size = int(row["params_batch_size"])
        curr_optimizer = row["params_optimizer"]
        curr_latent_space = int(row["params_latent_space"])

        K.clear_session()
        set_random_seed(279)
        X_train, X_val = load_mnist()

        # Build the best autoencoder model
        encoder_input = Input(shape=(28, 28, 1))

        encoder_output, shape_before = Encoder(encoder_input, curr_num_conv_layer, curr_conv_filters, curr_conv_kernels, curr_pool_sizes, curr_latent_space)

        decoder_output = Decoder(encoder_output, shape_before, curr_num_conv_layer, curr_conv_filters, curr_conv_kernels, curr_pool_sizes)

        best_autoencoder = Model(encoder_input, decoder_output)
        best_autoencoder.compile(curr_optimizer, loss="mse", metrics=[Precision(), Recall()])

        # Train the best autoencoder
        best_history = best_autoencoder.fit(X_train, X_train, epochs=curr_epochs, batch_size=curr_batch_size, shuffle=True, validation_data=(X_val, X_val))
        best_history.history["f1"] = [f1_score(p, r) for p, r in zip(best_history.history["precision"], best_history.history["recall"])]
        best_history.history["val_f1"] = [f1_score(p, r) for p, r in zip(best_history.history["val_precision"], best_history.history["val_recall"])]

        # Save the Encoder
        EncoderModel = Model(encoder_input, encoder_output)
        if not os.path.exists(f"AUTOENCODER/latent_{curr_latent_space}"):
            os.makedirs(f"AUTOENCODER/latent_{curr_latent_space}")
        EncoderModel.save(f"AUTOENCODER/latent_{curr_latent_space}/encoder.keras")

        # Plot training and validation loss curves
        plt.figure()
        plt.plot(best_history.history["loss"], marker="o", color="red", label="Train Loss")
        plt.plot(best_history.history["val_loss"], marker="o", color="green", label="Validation Loss")
        plt.xlabel("Epochs")
        plt.ylabel("Mean Squared Error")
        plt.title(f"Loss Curve for Latent Space {curr_latent_space}")
        plt.legend()
        if not os.path.exists(f"AUTOENCODER/latent_{curr_latent_space}"):
            os.makedirs(f"AUTOENCODER/latent_{curr_latent_space}")
        plt.savefig(f"AUTOENCODER/latent_{curr_latent_space}/loss.png")

        # Plot training and validation learning curves
        plt.figure()
        plt.plot(best_history.history["f1"], marker="o", color="red", label="Train F1 Score")
        plt.plot(best_history.history["val_f1"], marker="o", color="green", label="Validation F1 Score")
        plt.xlabel("Epochs")
        plt.ylabel("F1 Score")
        plt.title(f"Learning Curve for Latent Space {curr_latent_space}")
        plt.legend()
        if not os.path.exists(f"AUTOENCODER/latent_{curr_latent_space}"):
            os.makedirs(f"AUTOENCODER/latent_{curr_latent_space}")
        plt.savefig(f"AUTOENCODER/latent_{curr_latent_space}/f1.png")
