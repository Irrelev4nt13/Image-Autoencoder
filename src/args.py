import sys
import numpy as np


def get_files(arguments):
    files = {"-d": None, "-q": None, "-od": None, "-oq": None}

    for index, arg in enumerate(arguments):
        if arg in files.keys():
            files[arg] = arguments[index + 1]

    return files


def load_file(file_name):
    # Get data
    with open(file_name, "rb") as file:
        meta_data = np.fromfile(file, dtype=np.int32, count=4)
        pixels = np.fromfile(file, dtype=np.uint8, count=-1)

    # Intialize the dictionary we are going to use to save the file information
    data = {"magic_number": None, "number_of_images": None, "number_of_rows": None, "number_of_columns": None, "images": None}

    # Make the necessary convertion and save the data
    data["magic_number"] = meta_data.view(">i4")[0]
    data["number_of_images"] = meta_data.view(">i4")[1]
    data["number_of_rows"] = meta_data.view(">i4")[2]
    data["number_of_columns"] = meta_data.view(">i4")[3]
    data["images"] = pixels.reshape(data["number_of_images"], data["number_of_rows"] * data["number_of_columns"])

    # Return data
    return data


def save_file(data, file_name):
    # Save data
    with open(file_name, "wb") as file:
        # In order for the output folder to keep the same format we need to make the number little endian again
        # Because we read it as big endian
        file.write(data["magic_number"].byteswap().tobytes())
        file.write(data["number_of_images"].to_bytes(4, 'big'))
        file.write(data["number_of_rows"].to_bytes(4, 'big'))
        file.write(data['number_of_columns'].to_bytes(4, 'big'))

        for image in data["images"]:
            file.write(image)
