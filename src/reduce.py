import sys
import numpy as np

from Arguments import *

from tensorflow.keras.models import load_model


if __name__ == "__main__":
    print("Choose compress dimension from 10, 20, 30: ", end="")
    dim = input()
    if dim not in ["10", "20", "30"]:
        print(f"Sorry, the dimension {dim} is not supported")
    else:
        files = get_files(sys.argv)

        train_data = load_file(files["-d"])

        query_data = load_file(files["-q"])

        encoder = load_model(f"AUTOENCODER/latent_{dim}/encoder.keras")

        train_data["images"] = encoder.predict(train_data["images"])
        train_data["images"] = ((train_data["images"] - np.min(train_data["images"])) / (np.max(train_data["images"]) - np.min(train_data["images"]))) * 255
        train_data["number_of_rows"] = int(dim)
        train_data["number_of_columns"] = 1
        save_file(train_data, files["-od"])

        query_data["images"] = encoder.predict(query_data["images"])
        query_data["images"] = ((query_data["images"] - np.min(query_data["images"])) / (np.max(query_data["images"]) - np.min(query_data["images"]))) * 255
        query_data["number_of_rows"] = int(dim)
        query_data["number_of_columns"] = 1
        save_file(query_data, files["-oq"])
