import numpy as np
import tensorflow as tf
from keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Reshape, Conv2DTranspose
from sklearn.model_selection import train_test_split

MAX_TRAINING_IMAGES = 3000 # Number of images to use for training

# Read IDX3-ubyte file
def read_idx(filename):
    with open(filename, 'rb') as file:
        data = np.fromfile(file, dtype=np.uint8, count=-1)
    return data

if __name__ == "__main__":
    # Load the binary training dataset
    file_path = 'datasets/train-images.idx3-ubyte'
    data = read_idx(file_path)

    # Extract header information. Read in Big Endian format
    magic_number = data[:4].view('>u4')[0]
    num_items = data[4:8].view('>u4')[0]
    num_rows = data[8:12].view('>u4')[0]
    num_cols = data[12:16].view('>u4')[0]

    num_to_load = min(MAX_TRAINING_IMAGES, num_items)

    # Extract image data
    images = data[16:].reshape(num_items, num_rows * num_cols)[:num_to_load]

    # Reshape images to 4D tensor (batch_size, height, width, channels)
    images = images.reshape(-1, num_rows, num_cols, 1)

    train_images, val_images = train_test_split(images, test_size=0.2)

    # Define the encoder part of the model
    encoder = tf.keras.Sequential([
        Conv2D(32, (3, 3), activation='relu', padding='same', input_shape=(num_rows, num_cols, 1)),
        MaxPooling2D((2, 2), padding='same'),
        Conv2D(64, (3, 3), activation='relu', padding='same'),
        MaxPooling2D((2, 2), padding='same'),
        Flatten(),
        Dense(64, activation='relu'),
        Dense(10, activation='relu')
    ])

    # Define the decoder part of the model
    decoder = tf.keras.Sequential([
        Dense(64, activation='relu', input_shape=(10,)),
        Dense(7*7*64, activation='relu'),
        Reshape((7, 7, 64)),
        Conv2DTranspose(64, (3, 3), strides=2, activation='relu', padding='same'),
        Conv2DTranspose(32, (3, 3), strides=2, activation='relu', padding='same'),
        Conv2D(1, (3, 3), activation='relu', padding='same')
    ])

    # Combine the encoder and decoder into an autoencoder model
    autoencoder = tf.keras.Model(inputs=encoder.input, outputs=decoder(encoder.output))

    autoencoder.summary()

    optimizer = tf.keras.optimizers.SGD(learning_rate=0.01, momentum=0.9)

    autoencoder.compile(optimizer=optimizer, loss='mae', metrics=['accuracy'])

    # Train the model
    autoencoder.fit(train_images, train_images, epochs=4, batch_size=32, validation_data=(val_images, val_images))

    # Save the model
    autoencoder.save('autoencoder_model.keras')